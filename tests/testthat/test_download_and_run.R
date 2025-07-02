library(testthat)
library(rsocsim)

test_that("Download rates and run simulation", {
  dir.create("tmp/tmp", recursive = TRUE)
  tmpdir <- file.path(getwd(), "tmp/tmp")
  
  expect_true(dir.exists(tmpdir))

  countrycode <- "MM"
  final_sim_year <- 2010
  rsocsim::download_rates(tmpdir, countrycode, yearStart = 2003, yearEnd = final_sim_year, source = "UN")
  
  # Assuming the download_rates function creates a specific file, check if it exists
  expect_true(file.exists(file.path(tmpdir, "rates", paste0(countrycode, "_rates_info.sup"))))
  
  rsocsim::create_initial_population(tmpdir, size_opop = 1000, output_base = "presim")

  result <- rsocsim::socsim(tmpdir, paste0(countrycode, "_rates_info.sup"))
  print(paste("Result:", result))
  # Add assertions based on expected results from the simulation
  expect_is(result, 1)

  
  # read simulation outputs
  omar = rsocsim::read_omar(folder,supfile,seed,suffix=suffix)
  omar.head()

  opop = rsocsim::read_opop(folder,supfile,seed,suffix)
  opop.head()

  # Obtain partial kinship network, with omar and opop already in R environment
  pid <- c("111", "10211", "10311")
  kin_network <- getKin(opop = opop, omar = omar, pid = pid, 
                        extra_kintypes = c("unclesaunts", "niblings"), kin_by_sex = TRUE)

  kin_network$nieces[[3]][1]

  opop$pid[kin_network$nieces[[3]]]


  ####

  fert_rates <- estimate_fertility_rates(
    opop=opop,
    final_sim_year=final_sim_year,
    year_min=2004,
    year_max=2009,
    year_group = 5,
    age_min_fert = 15,
    age_max_fert = 50,
    age_group = 5
  )
  print(fert_rates.head())
})