
#' Title
#'
#' @param path 
#'
#' @return
#' @export
#'
#' @examples
#'
#' @export
read_omar <- function(folder, supfile="socsim.sup",seed=42,suffix=""){
  # create fn
  
  
  omar<-read.table(file = path, header = F, as.is = T)
  names(omar)<-c("mid","wpid","hpid","dstart","dend", "rend","wprior","hprior")
  return(omar)
}


#' Title
#'
#' @param path 
#'
#' @return
#' @export
#'
#' @examples
#'
#' @export
read_opop <- function(path){
  opop <- read.table(file=path,header=F,as.is=T)  
  ## assign names to columns
  names(opop)<-c("pid","fem","group",
                 "nev","dob","mom","pop","nesibm","nesibp",
                 "lborn","marid","mstat","dod","fmult")
  return(opop)
}
