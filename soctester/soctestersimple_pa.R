
library("tools")
  


print_string_to_start_socsim_on_cmd <- function(rootfolder,folder, supfile,seed) {
  print(paste0("cd ", rootfolder, "\\",folder))
  print(paste0("socsim.exe", " ", rootfolder, "\\", folder,"\\",supfile, " ", toString(seed)))
}



  rootfolder <-"D:/dev/r/socsimprojects";
  folderlist <- c("CousinDiversity", "MarriageSqueeze", "sweden_small", "sweden_small")
  supfilelist <- c("CousinDiversity.sup", "squeeze.sup", "simul_test.sup", "simul_Sweden.sup")
  outfilespyr <- c("SimResults/4groups.pyr","SimResults/squeeze.pyr","popfiles/output_pop.pyr","popfiles/output_pop.pyr")
  outfilespyrhashes <-c("","","","")
  outfilesopop <- c("SimResults/4groups.opop","SimResults/squeeze.opop","popfiles/output_pop.opop","popfiles/output_pop.opop")
  outfilesopophashes <-c("","","","")
  outfilesomar <- c("SimResults/4groups.omar","SimResults/squeeze.omar","popfiles/output_pop.omar","popfiles/output_pop.omar")
  outfilesomarhashes <-c("","","","")
  seedlist <- c(12340,1234.0,1234.0,1234)
  df <- data.frame(folderlist,supfilelist,seedlist,outfilespyr,outfilespyrhashes,outfilesopop,outfilesopophashes,outfilesomar,outfilesomarhashes)
  
  #run1simulation(rootfolder,folder[1],supfile[1],seed[1]);
  results.df <- data.frame(matrix(ncol = 6, nrow = 0))
  
  colnames(results.df)<-c("pyr","opop","omar","supfile","time","comment")

  results.df <- load("results.rda")
  
  
  library("future")
  f <- future({
       cat("Hello world!\n")
       3.14
       
       
       startSocsimWithFile(supfilelist[i],toString(seedlist[i]))
   })
  v <- value(f)  
  
  library(rsoc)
  for (i in 1:2) {

    comment <- "third run"
        print(i)
    print(folderlist[i])
    print(supfilelist[i])
    print_string_to_start_socsim_on_cmd(rootfolder,folderlist[i],supfilelist[i],toString(seedlist[i]))
  
    setwd(paste0(rootfolder,"\\",folderlist[i]))
    print(seedlist)
    startSocsimWithFile(supfilelist[i],toString(seedlist[i]))
    hash1 <- md5sum(outfilespyr[i])
    print(paste0("done with ",i))
    print(hash1)
    df[i,"outfilespyrhashes"] <-hash1
    
    hash2 <- md5sum(df[i,"outfilesomar"])
    print(hash2)
    df[i,"outfilesomarhashes"] <-hash2
    
    hash3 <- md5sum(df[i,"outfilesopop"])
    print(hash3)
    df[i,"outfilesopophashes"] <-hash3
    newrow <- data.frame(matrix(nrow=1,data= c(hash1,hash2,hash3,df[i,"supfilelist"],Sys.time(),comment)))
    print(newrow)
    results.df <- rbind(results.df, data.frame(newrow))
    save(results.df,file="results.rda")
  }
  typeof(seedlist[1])
  
  
  detach(package:rsoc, unload=TRUE)#
  
  reload(pkg_name("rsoc"))
library(devtools)  
  
  install.packages("devtools")  
  
  library.dynam.unload()#"rsoc")
  
  library.dynam.unload("src\rsoc.dll","rsoc")
?library.dynam.unload
?sub
  
  
i=1
socsimbin = "C:/dev/socsim-dev/socsim.exe"
print(socsimbin)

setwd(paste0(rootfolder,"\\",folderlist[i]))
socsimcommand <- paste0(socsimbin," ",rootfolder,"/",folderlist[i],"/",supfilelist[i], " ", toString(seedlist[i]),sep="")
print(socsimcommand)
system(socsimcommand)

system("dir")
system("cmd.exe /c dir", intern = T)  


md5sum(socsimbin)
