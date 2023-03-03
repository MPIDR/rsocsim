res$ggparents <- lapply(pid,
                        function(pid){
                          as.vector(unlist(opop[opop$pid == pid, 
                                                c("MMM","MMF","MFM","MFF",
                                                  "FMM","FMF","FFM","FFF")]))})

if(kin_by_sex){
  res$ggmothers <- lapply(pid,
                         function(pid){
                           as.vector(unlist(opop[opop$pid == pid,
                                                 c("MMM","FMM", "FFM", "MFM")]))})
  
  res$ggfathers <- lapply(pid,
                          function(pid){
                            as.vector(unlist(opop[opop$pid == pid, 
                                                  c("FFF", "MMF", "MFF", "FMF")]))})
}