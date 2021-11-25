#library("future")
#library(rsoc)



#' Run a socsim-simulation in a seperate process
#'
#' @param rootfolder rootfolder...
#' @param folder folder name of the simulation
#' @param supfile the .sup file to start the simulation
#' @param seed RNG seed
#' @return The results will be written into the specified folder
#' @export
run1simulationwithfile <- function(rootfolder,folder, supfile,seed) {
  # use the "future" library to run a rcpp-socsim simulation
  # in a seperate process
  print("Start run1simulationwithfile.")
  future::plan(future::multisession)
  print("after future::plan(future::multisession)")
  setwd(paste0(rootfolder,"\\",folder))
  print(seed)
  
  f1 <- future::future({
    startSocsimWithFile(supfile,seed)
    v1 <- future::value(f1)
  })
}

#' Run a socsim-simulation in the r-process
#'
#' @param rootfolder rootfolder  name of the simulation
#' @param supfile the .sup file to start the simulation
#' @param seed RNG seed
#' @return The results will be written into the specified folder
#' @export
run1simulationwithfile_inprocess <- function(rootfolder, supfile,seed) {
  # use the "future" library to run a rcpp-socsim simulation
  # in a seperate process
  print("Start run1simulationwithfile. in process.")
  print(rootfolder)
  setwd(paste0(rootfolder))
  print(seed)
  startSocsimWithFile(supfile,seed)
}


#library(parallel)

#' @export
run1simulationwithfile_apply <- function(folder, supfile,seed="23") {
  # use the "future" library to run a rcpp-socsim simulation
  # in a seperate process
  folder = "D:\\dev\\r\\socsimprojects\\CousinDiversity"
  supfile = "CousinDiversity.sup"
  seed = "23"
  numCores=1
  parallel::mclapply(c(1),run1simulationwithfile_inprocess(folder, supfile,seed), mc.cores = numCores)
  # now there is a mysterious error: startSocsimWithFile is not available somehow????? 
}
