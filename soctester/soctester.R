
library("future")

test_all <- function() {
  
  rootfolder <-"D:\\dev\\r\\socsimprojects";
  folder <- c("CousinDiversity","CousinDiversity", "MarriageSqueeze", "sweden_small", "sweden_small")
  supfile <- c("CousinDiversity.sup","CousinDiversity.sup", "squeeze.sup", "simul_test.sup", "simul_Sweden.sup")
  seed <- c("12345","12345","1234","1234","1234")
  df <- data.frame(folder,supfile,seed)
  
  plan(multisession)
  f1 <- future({
    i<- 1
    run1simulation(rootfolder,folder[i],supfile[i],seed[i]);
  })

  
  f2 <- future({
    i<- 1
    run1simulation(rootfolder,folder[i],supfile[i],seed[i]);
  })
  
  f3 <- future({
    i<- 2
    run1simulation(rootfolder,folder[i],supfile[i],seed[i]);
  })
  v1 <- value(f1)
  v2 <- value(f2)
  v3 <- value(f3)
  
  
}

run1simulation <- function(rootfolder,folder, supfile,seed) {
  library(rsoc)
  
  setwd(paste0(rootfolder,"\\",folder[1]))
  print(seed)
  startSocsimWithFile(supfile,seed)
  
}

# i=1 runs correct, even many times
# i=2 runs correct.
# i=1 after i=2 -->fails!

test_all()




detach("package:rsoc", unload=TRUE)
