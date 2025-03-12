#' download and convert rate files from HMD, HFD, UN
#' @description Given a iso2code countrycode and a range of years (between 1950 
#'      and 2100) this function will download mortality and fertility rates and
#'      saves them in a
#' @param folder folder where the rates will be saved
#' @param countrycode 2-character country code https://en.wikipedia.org/wiki/ISO_3166-1_alpha-2
#' @param yearStart
#' @param yearEnd
#' @param source "UN", at the moment only UN is supported
#' @param apiUrl url of the API
#' @details 
#' See https://github.com/tomthe/retrieveVitalRates_for_rsocsim_API for the 
#' code that converts the source data into socsim format.
#' 
#' @return todo
#'@examples
#' \dontrun{
#' # todo
#' }
#' @export
download_rates <- function(folder, countrycode, yearStart = 1950, yearEnd = 2100, 
                         source = "UN", 
                         apiUrl = "https://user.demogr.mpg.de/theile/socsimratesAPI/v1/") {
  
  # Validate input parameters
  if (!dir.exists(folder)) {
    stop("The specified folder does not exist.")
  }
  
  if (!is.character(countrycode) || nchar(countrycode) != 2) {
    stop("countrycode should be a 2-character string.")
  }
  
  if (!is.numeric(yearStart) || !is.numeric(yearEnd) || yearStart > yearEnd) {
    stop("yearStart and yearEnd should be numeric values with yearStart <= yearEnd")
  }
  
  # Construct API URLs for fertility and mortality data
  fertUrl <- paste0(apiUrl, "fert/", source, "/", countrycode)
  mortUrl <- paste0(apiUrl, "mort/", source, "/", countrycode)
  
  # Create a temporary directory for downloading zip files
  tempDir <- tempdir()
  
  # Download fertility and mortality zip files
  fertZipPath <- file.path(tempDir, "fertility.zip")
  mortZipPath <- file.path(tempDir, "mortality.zip")
  
  tryCatch({
    message("Downloading fertility data...")
    download.file(fertUrl, destfile = fertZipPath, mode = "wb", quiet = TRUE)
    message("Downloading mortality data...")
    download.file(mortUrl, destfile = mortZipPath, mode = "wb", quiet = TRUE)
  }, error = function(e) {
    stop(paste("Error downloading data from API:", e$message))
  })
  
  message("Processing files...")
  
  # Process zip file and extract relevant files based on year range
  processZip <- function(zipPath, type) {
    tryCatch({
      # Extract file list from zip
      zipContent <- unzip(zipPath, list = TRUE)
      files <- zipContent$Name
      
      # Initialize vector for extracted files
      extractedFiles <- character(0)
      
      for (file in files) {
        # Extract year numbers from filename - looking for 4-digit numbers
        yearMatches <- regmatches(file, gregexpr("\\d{4}", file))[[1]]
        
        # Skip if no year found in filename
        if (length(yearMatches) == 0) {
          next
        }
        
        # Convert matched strings to numbers
        years <- as.numeric(yearMatches)
        
        # Determine the file's year range 
        fileYearStart <- min(years)
        fileYearEnd <- max(years)
        
        # Check if file's year range overlaps with requested range
        if (fileYearEnd >= yearStart && fileYearStart <= yearEnd) {
          # Extract the file
          unzip(zipPath, files = file, exdir = folder, overwrite = TRUE)
          extractedFiles <- c(extractedFiles, file)
        }
      }
      
      return(extractedFiles)
    }, error = function(e) {
      message("Error processing ", type, " zip file: ", e$message)
      return(character(0))
    })
  }
  
  # Process both zip files
  extractedFertFiles <- processZip(fertZipPath, "fertility")
  extractedMortFiles <- processZip(mortZipPath, "mortality")
  
  # Create info file
  infoFilePath <- file.path(folder, paste0(countrycode, "_rates_info.sup"))
  
  # Create supervisory file content
  supContent <- paste0(
    "***************************************************************************\n",
    "* Supervisory file for SOCSIM microsimulation for ", countrycode, " ", yearStart, "-", yearEnd, "\n",
    "* Created on ", format(Sys.time(), "%d.%m.%Y"), "\n",
    "***************************************************************************\n",
    "* This simulation uses as input age-specific data retrieved from ", source, "\n",
    "* converted into SOCSIM format.\n",
    "***************************************************************************\n",
    "segments ", yearEnd - yearStart + 1, "\n",
    "input_file presim\n",
    "execute \"touch presim.omar\"\n",
    "\nbint 12\nhetfert 1\nmarriage_queues 1\nmarriage_eval distribution\nmarriage_after_childbirth 1\nrandom_father 1\n",
    "\n***********************************************************************\n",
    "* Pre-simulation (100 years) to get stable population at ", yearStart - 1, "\n",
    "* Using rates for ", yearStart, "\n",
    "\nduration 1200\ninclude rates/", countrycode, "fert", yearStart, "\ninclude rates/", countrycode, "mort", yearStart, "\nrun\n"
  )
  
  # Add simulation segments for each year
  for (year in yearStart:yearEnd) {
    supContent <- paste0(
      supContent,
      "\n* Simulation for year ", year, "\n",
      "duration 12\ninclude rates/", countrycode, "fert", year, "\ninclude rates/", countrycode, "mort", year, "\nrun\n"
    )
  }
  
  writeLines(supContent, infoFilePath)
  
  # Clean up temporary files
  unlink(fertZipPath)
  unlink(mortZipPath)
  
  message("Process complete. Files extracted to ", folder)
  
  return(list(
    fertility = extractedFertFiles, 
    mortality = extractedMortFiles,
    infoFile = infoFilePath
  ))
}