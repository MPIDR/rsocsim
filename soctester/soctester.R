test_all <- function() {
  
  rootfolder <-"D:\\dev\\r\\socsimprojects";
  folder <- c("CousinDiversity", "MarriageSqueeze", "sweden_small", "sweden_small")
  supfile <- c("CousinDiversity.sup", "squeeze.sup", "simul_test.sup", "simul_Sweden.sup")
  seed <- c(12345,1234,1234,1234)
  df <- data.frame(folder,supfile,seed)
  
  run1simulation(rootfolder,folder[1],supfile[1],seed[1]);
  
}

run1simulation <- function(rootfolder,folder, supfile,seed) {
  library(rsoc)
  
  setwd(paste0(rootfolder,"\\",folder[1]))
  print(seed)
  startSocsimWithFile(supfile,seed)
  
}
test_all()




detach("package:rsoc", unload=TRUE)
