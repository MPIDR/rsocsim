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
  future::plan(future::multisession)
  setwd(paste0(rootfolder,"\\",folder[1]))
  print(seed)
  
  f1 <- future({
    startSocsimWithFile(supfile,seed)
    v1 <- future::value(f1)
  })
}
