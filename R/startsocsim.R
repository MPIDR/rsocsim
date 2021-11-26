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
  result = NULL
  tryCatch(expr = {
    setwd(paste0(folder))
    if ((process_method=="inprocess") | (process_method =="default")) {
      result = run1simulationwithfile_inprocess(supfile=supfile,seed=seed)
    } else if (process_method=="future") {
      result = run1simulationwithfile_future(supfile=supfile,seed=seed)
    } else if (process_method=="clustercall") {
      result = run1simulationwithfile_clustercall(supfile=supfile,seed=seed)
    }
  },
  error = function(w){
    print("Error during execution of simulation!")
  },
  finally = {
    print(previous_wd)
    setwd(previous_wd)
  }
  )
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
#run1simulationwithfile_apply <- function(folder, supfile,seed="23") {
#  # use the "future" library to run a rcpp-socsim simulation
#  # in a seperate process
#  folder = "D:\\dev\\r\\socsimprojects\\CousinDiversity"
#  supfile = "CousinDiversity.sup"
#  seed = "23"
#  numCores=2
#  cl <- parallel::makeCluster(numCores, type="PSOCK", outfile="")
#  parallel::clusterExport(cl, run1simulationwithfile_inprocess)
#  parallel::parLapply(cl,c(2),run1simulationwithfile_inprocess, folder=folder, supfile=supfile,seed=seed)
#  # now there is a mysterious error: startSocsimWithFile is not available somehow????? 
#  parallel::stopCluster(cl)
#  return(1)
#}

run1simulationwithfile_clustercall <- function(supfile,seed="23") {
  # use the "future" library to run a rcpp-socsim simulation
  # in a seperate process
  print("parallel::clusterCall")
  numCores=1
  cl <- parallel::makeCluster(numCores, type="PSOCK", outfile="socsim_clustercall.log")
  parallel::clusterExport(cl, "startSocsimWithFile")
  parallel::clusterCall(cl,startSocsimWithFile, supfile=supfile,seed=seed)
  parallel::stopCluster(cl)
  return(1)
}

#bla <- function(){
#  print("function bla")
#    folder = "D:\\dev\\r\\socsimprojects\\CousinDiversity"
#  supfile = "CousinDiversity.sup"
#  seed="33"
#  previous_wd = getwd()
#  setwd(paste0(folder))
#  print(seed)
#  startSocsimWithFile(supfile,seed)
#  setwd(previous_wd)
#  return(1) 
#}

#' Run a single socsim-simulation with a socsim binary.
#' the place
#' The results will be saved into that folder
#'
#' @param rootfolder rootfolder...
#' @param folder base-directory of the simulation. 
#' @param supfile the .sup file to start the simulation, relative to the
#' folder
#' @param seed RNG seed as string, Default="42"
#' @param socsim_path path+filename of a socsim-executable. Download one from https://github.com/tomthe/socsim/releases/download/0.3/socsim.exe
#' @return The results will be written into the specified folder
#' @export
run1simulationwithfile_from_binary <- function(folder, supfile,seed="42",socsim_path=NULL) {
  if (is.null(socsim_path)){
    print("No socsim_path specified. So I will download the Windows-binary from github to a temporary directory!")
    print("This will probably not work due to antivirus-software.")
    print("please download an executable socsim from https://github.com/tomthe/socsim/releases/download/0.3/socsim.exe")
    print("then save the whole path and specify it as socsim_path for this function!")
    url = "https://github.com/tomthe/socsim/releases/download/0.3/socsim.exe"
    socsim_path = paste0(tempdir(),"\\","socsim.exe")
    download.file(url,socsim_path,method="auto")
  }
  seed = toString(seed)
  print("Start run1simulationwithfile")
  print(folder)
  print(supfile)
  print(paste0("socsim_path: ",socsim_path))
  print(seed)
  previous_wd = getwd()
  setwd(paste0(folder))
  
  print(paste0("command:  ",socsim_path,args=paste0(" ",supfile," ", seed)))
  
  print(system2(socsim_path,args=c(supfile," ", seed)))
  print(system(paste(socsim_path,supfile, seed)))
  a = (system(paste(socsim_path,paste0(folder,"\\",supfile), seed)))
  print(paste(socsim_path,paste0(folder,"\\",supfile), seed))
  print(a)
  print(previous_wd)
  setwd(previous_wd)
  
  return(1)
}
