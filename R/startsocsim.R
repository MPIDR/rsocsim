#' Run a single Socsim simulation with a given supervisory file and directory
#'
#' @param folder A string. This is the base directory of the simulation. Every
#'   .sup and rate file should be named relative to this directory. 
#' @param supfile A string. The name of the .sup file to start the simulation,
#'   relative to the directory.
#' @param seed A string. The seed for the RNG, so expects an integer. Defaults
#'   to "42".
#' @param process_method A string. Whether and how SOCSIM should be started in
#'   its own process or in the running R process. Defaults to "incprocess". Use
#'   one of:
#'    * "future" - the safest option. A new process will be started via the
#'      "future" package
#'    * "inprocess" - SOCSIM runs in the R-process. Beware if you run several
#'      different simulations, they may affect later simulations.
#'    * "clustercall" - if the future package is not available, try this method
#'      instead.
#' @param compatibility_mode A string.
#' @param suffix A string.
#' @return Returns the name of the directory to which the results will be
#'   written.
#' @export
socsim <- function(folder, supfile, seed = "42", process_method = "inprocess",
                   compatibility_mode = "1", suffix = "") {
  seed = as.character(seed)
  compatibility_mode = as.character(compatibility_mode)
  print("Start running one simulation with a .sup file.")
  print(paste("Base directory of the simulation:", folder))
  print(paste("RNG seed:", seed))
  previous_wd = getwd()
  result = NULL
  tryCatch(expr = {
    setwd(folder)
    if (process_method == "inprocess") {
      result = run1simulationwithfile_inprocess(supfile = supfile,
                                                seed = seed,
                                                compatibility_mode = compatibility_mode,
                                                suffix = suffix)
    } else if (process_method == "future") {
      result = run1simulationwithfile_future(supfile = supfile,
                                             seed = seed,
                                             compatibility_mode = compatibility_mode,
                                             suffix = suffix)
    } else if (process_method == "clustercall") {
      result = run1simulationwithfile_clustercall(supfile = supfile, 
                                                  seed = seed, 
                                                  compatibility_mode = compatibility_mode,
                                                  suffix = suffix)
    }
  },
  error = function(w){
    warning("Error during execution of simulation!")
    warning(w)
  },
  finally = {
    print(paste("Restore previous working dir:", previous_wd))
    setwd(previous_wd)
  }
  )
  return(result)
  
}

print_last_line_of_logfile = function(logfilename, lastline = "") {
  tryCatch({
    con = file(logfilename, "r")
    while ( TRUE ) {
      line = readLines(con, n = 1)
      if ( length(line) == 0 ) {
        if(lastline != line2){
          print(line2);
        }
        break
      }
      line2 <- line
    }
    close(con)
    return(line2)
  }, error = function(e) {
    warning("Error while reading file")
    warning(logfilename)
    warning(e)
    return("err0")
  })
}

run1simulationwithfile_future <- function(supfile,seed="42",compatibility_mode="1",suffix="") {
  # use the "future" library to run a rcpp-socsim simulation
  # in a seperate process
  print("create future cluster")
  future::plan(future::multisession, workers=2)
  #print("after future::plan(future::multisession)")
  print("start socsim simulation now. no output will be shown!")
  
  f1 <- future::future({
    startSocsimWithFile(supfile,seed,compatibility_mode,result_suffix=suffix)
  },seed=TRUE)
  print("started!")
  # start a loop and check whether the simulation in the future is finished.
  # if it is not yet finished, read the output file and print the last line
  # to the console
  outfn = paste0("sim_results_",supfile,"_",seed,"_",suffix,"/logfile.log")
  print(paste0("wait for simulation to finish, log file: ",outfn))
  lastline = ""
  while (!future::resolved(f1)) {
    Sys.sleep(1)
    lastline = print_last_line_of_logfile(outfn, lastline)
    if (lastline=="err0"){
      break;
    }
  }
  print("simulation finished")
  
  v1 <- future::value(f1)
  return(1)
}

#' Run a socsim-simulation in the r-process
#'
#' @param rootfolder rootfolder  name of the simulation
#' @param supfile the .sup file to start the simulation
#' @param seed RNG seed
#' @return The results will be written into the specified folder
run1simulationwithfile_inprocess <- function(folder, supfile,seed,compatibility_mode="1",suffix="") {
  startSocsimWithFile(supfile,seed,compatibility_mode,result_suffix=suffix)
  return(1)
}

run1simulationwithfile_clustercall <- function(supfile,seed="23",compatibility_mode="1",suffix="") {
  # use the "future" library to run a rcpp-socsim simulation
  # in a seperate process
  print("parallel::clusterCall")
  numCores=1
  outfn = paste0("sim_results_",supfile,"_",seed,"_",suffix,"/logfile.log")
  cl <- parallel::makeCluster(numCores, type="PSOCK", outfile=outfn)
  parallel::clusterExport(cl, "startSocsimWithFile")
  parallel::clusterCall(cl,startSocsimWithFile, supfile=supfile,seed=seed,compatibility_mode=compatibility_mode,result_suffix=suffix)
  print("started!")
  print_last_line_of_logfile(outfn)
  parallel::stopCluster(cl)
  return(1)
}


#' Run a single socsim-simulation with a socsim binary
#'
#' @param rootfolder rootfolder...
#' @param folder base-directory of the simulation. 
#' @param supfile the .sup file to start the simulation, relative to the
#' folder
#' @param seed RNG seed as string, Default="42"
#' @param socsim_path path+filename of a socsim-executable. Download one from https://github.com/tomthe/socsim/releases/download/0.3/socsim.exe
#' @return The results will be written into the specified folder
#' @export
run1simulationwithfile_from_binary <- function(folder, supfile,seed="42",compatibility_mode="1",socsim_path=NULL) {
  if (is.null(socsim_path) || !dir.exists(normalizePath(socsim_path))) {
    print("No socsim_path specified. So I will download the Windows-binary from github to a temporary directory!")
    print("This will probably not work due to antivirus-software.")
    print("please download an executable socsim from https://github.com/tomthe/socsim/releases/download/0.3/socsim.exe")
    print("then save the whole path and specify it as socsim_path for this function!")
    url = "https://github.com/tomthe/socsim/releases/download/0.3/socsim.exe"
    socsim_path = file.path(tempdir(), "socsim.exe")
    download.file(url,socsim_path,method="auto")
  }
  seed = toString(seed)
  print("Start run1simulationwithfile")
  print(folder)
  print(supfile)
  print(paste0("socsim_path: ", socsim_path))
  print(seed)
  previous_wd = getwd()
  setwd(paste0(folder))
  
  print(paste0("command: ",socsim_path,args=paste0(" ",supfile," ", seed," ", compatibility_mode)))
  
  print(system2(socsim_path,args=c(supfile," ", seed, " ",compatibility_mode)))
  print(system(paste(socsim_path, supfile, seed,compatibility_mode)))
  a = (system(paste(socsim_path, paste0(dirname(folder), "\\", supfile), seed, compatibility_mode)))
  print(paste(socsim_path, paste0(dirname(folder), "\\", supfile), seed,compatibility_mode))
  print(a)
  print(previous_wd)
  setwd(previous_wd)
  return(1)
}

#' Create a directory structure for the simulation
#'
#' Create a two-level directory structure. If the first-level argument is NULL,
#' we look for and, if needed, created the directory 'socsim' in the user's
#' home directory. If the second-level argument is NULL, we create a directory
#' named 'socsim_sim_{some random component}' in the first-level directory.
#'
#' @param basedir A string. Optional. First-level directory where the
#'   simulation-specific directory will be created. Defaults to '$HOME/socsim'.
#' @param simdir A string. Optional. Simulation-specific directory which will
#'   be created within 'basedir'. Defaults to 'socsim_sim_' plus a random
#'   component created with [tempfile()].
#' @return A string. The full path to the simulation-specific directory.
#' @export
create_simulation_folder <- function(basedir = NULL, simdir = NULL) {
    # If no 'basedir' is given, we default to '$HOME/socsim'.
    if (is.null(basedir)) { basedir <- file.path(path.expand("~"), "socsim") }
    # If 'basedir' does not exist, create it.
    if (!dir.exists(basedir)) { dir.create(basedir) }
    if (is.null(simdir)) {
        # If no 'simdir' is given, create a random name that starts with
        # 'socsim_sim_'.
        subdir = tempfile(pattern = "socsim_sim_", tmpdir = basedir)
    } else {
        subdir = file.path(basedir, simdir)
    }
    if (!dir.exists(subdir)) { dir.create(subdir) }
    return(subdir)
}

#' Create a basic .sup file for a simulation
#' 
#' The simulation is only a simple one. The file will be saved into the
#' directory 'simdir'.
#' 
#' @param simdir A string. The directory where the .sup file will be saved.
#' @param simname A string. The name of the simulation.
#' @return A string. The filename of the supervisory file which is needed to
#'   start the simulation.
#' @export
create_sup_file <- function(simdir, simname) {
  sup_content <- "
*Supervisory file for a stable population
* 20220120
marriage_queues 1
bint 10
segments 1
marriage_eval distribution
marriage_after_childbirth 1
input_file init_new
*
duration 1200
include SWEfert2022
include SWEmort2022
run
"
  sup_fn <- "socsim.sup"
  cat(sup_content, file = file.path(simdir, sup_fn))
  fn_SWEfert2022_source <- system.file("extdata", "SWEfert2022", package = "rsocsim", mustWork = TRUE)
  fn_SWEfert2022_dest <- file.path(simdir, "SWEfert2022")
  fn_SWEmort2022_source <- system.file("extdata", "SWEmort2022", package = "rsocsim", mustWork = TRUE)
  fn_SWEmort2022_dest <- file.path(simdir, "SWEmort2022")
  fn_init_source <- system.file("extdata", "init_new.opop", package = "rsocsim", mustWork = TRUE)
  fn_init_dest <- file.path(simdir, "init_new.opop")
  file.copy(fn_SWEfert2022_source, fn_SWEfert2022_dest)
  file.copy(fn_SWEmort2022_source, fn_SWEmort2022_dest)
  file.copy(fn_init_source, fn_init_dest)
  return(sup_fn)
}

#' Read the content of the supervisory file 
#'
#' @param simdir A string. Base directory of the simulation.
#' @param simname A string. File name of the .sup file.
#' @return A list of strings. The content of the supervisory file.
#' @export
get_supervisory_content <- function(simdir, sup_fn) {
  if (is.null(sup_fn)) {
    sup_fn <- "socsim.sup"
  }
  sup_content <- readLines(file.path(simdir, sup_fn))
  return(sup_content)
}

#' Calculate for how many years the simulation ran
#'
#' @param simulation_time An integer. The number of periods (months) the
#'   simulation ran.
#' @param pre_simulation_time An integer. The number of periods (months) the
#'   simulation ran before getting to a stable population. This is subtracted
#'   from 'simulation_time' in order to arrive at the "real" simulation time
#' @param start_year An integer. The year the simulation started.
#' @return An number. The number of years for which the simulation ran. May
#'   have a fractional part.
#' @export
simulation_time_to_years <- function(simulation_time, pre_simulation_time, start_year) {
    stopifnot(all(is.integer(simulation_time), is.integer(pre_simulation_time), is.integer(start_year)))
    return(start_year + (simulation_time - pre_simulation_time)/12)
}
