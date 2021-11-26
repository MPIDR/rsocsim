#library("future")
#library(rsoc)


#' Run a single socsim-simulation with a given supplement-file and folder.
#' The results will be saved into that folder
#'
#' @param rootfolder rootfolder...
#' @param folder base-directory of the simulation. 
#' @param supfile the .sup file to start the simulation, relative to the
#' folder
#' @param seed RNG seed as string, Default="42"
#' @param process_method specify whether and how SOCSIM should be started in its
#' own process or in the running R process. Use one of
#'  "inprocess" - SOCSIM runs in the R-process. Beware if you run several different
#'  simulations - they may affect later simulations
#'  "future" - the safest option. A new process will be start via the "future"
#'  package
#'  "clustercall" - if the future package is not available, try this method instead
#' @return The results will be written into the specified folder
#' @export
run1simulationwithfile <- function(folder, supfile,seed="42",process_method="inprocess") {
  
  print("Start run1simulationwithfile")
  print(folder)
  print(seed)
  previous_wd = getwd()
  setwd(paste0(folder))
  
  result = NULL
  if ((process_method=="inprocess") | (process_method =="default")) {
    result = run1simulationwithfile_inprocess(supfile=supfile,seed=seed)
  } else if (process_method=="future") {
    result = run1simulationwithfile_future(supfile=supfile,seed=seed)
  } else if (process_method=="clustercall") {
    result = run1simulationwithfile_clustercall(supfile=supfile,seed=seed)
  }
  
  print(previous_wd)
  setwd(previous_wd)
  
  return(result)
  
}

run1simulationwithfile_future <- function(supfile,seed="42") {
  # use the "future" library to run a rcpp-socsim simulation
  # in a seperate process
  print("create future cluster")
  future::plan(future::multisession)
  print("after future::plan(future::multisession)")
  print("start socsim simulation now. no output will be shown!")
  
  f1 <- future::future({
    startSocsimWithFile(supfile,seed)
  },seed=TRUE)
  v1 <- future::value(f1)
  return(1)
}


#' Run a socsim-simulation in the r-process
#'
#' @param rootfolder rootfolder  name of the simulation
#' @param supfile the .sup file to start the simulation
#' @param seed RNG seed
#' @return The results will be written into the specified folder
run1simulationwithfile_inprocess <- function(folder, supfile,seed) {
  startSocsimWithFile(supfile,seed)
  return(1)
}



# Deprecated for now!
run1simulationwithfile_apply <- function(folder, supfile,seed="23") {
  # use the "future" library to run a rcpp-socsim simulation
  # in a seperate process
  folder = "D:\\dev\\r\\socsimprojects\\CousinDiversity"
  supfile = "CousinDiversity.sup"
  seed = "23"
  numCores=2
  cl <- parallel::makeCluster(numCores, type="PSOCK", outfile="")
  parallel::clusterExport(cl, run1simulationwithfile_inprocess)
  parallel::parLapply(cl,c(2),run1simulationwithfile_inprocess, folder=folder, supfile=supfile,seed=seed)
  # now there is a mysterious error: startSocsimWithFile is not available somehow????? 
  parallel::stopCluster(cl)
  return(1)
}

run1simulationwithfile_clustercall <- function(supfile,seed="23") {
  # use the "future" library to run a rcpp-socsim simulation
  # in a seperate process
  print("parallel::clusterCall")
  numCores=1
  cl <- parallel::makeCluster(numCores, type="PSOCK", outfile="socsim_clustercall.log")
  parallel::clusterExport(cl, "bla")
  parallel::clusterCall(cl,startSocsimWithFile, supfile=supfile,seed=seed)
  parallel::stopCluster(cl)
  return(1)
}

bla <- function(){
  print("function bla")
    folder = "D:\\dev\\r\\socsimprojects\\CousinDiversity"
  supfile = "CousinDiversity.sup"
  seed="33"
  previous_wd = getwd()
  setwd(paste0(folder))
  print(seed)
  startSocsimWithFile(supfile,seed)
  setwd(previous_wd)
  return(1) 
}
