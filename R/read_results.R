
#' Title
#' 

#' | **position** | **name** | **description**                                                    |
#' | -----------: | :------- | :----------------------------------------------------------------- |
#' |            1 | mid      | Marriage id number (unique sequential integer)                     |
#' |            2 | wpid     | Wife’s person id                                                   |
#' |            3 | hpid     | Husband’s person id                                                |
#' |            4 | dstart   | Date marriage began                                                |
#' |            5 | dend     | Date marriage ended or zero if still in force at end of simulation |
#' |            6 | rend     | Reason marriage ended 2 = divorce; 3 = death of one partner        |
#' |            7 | wprior   | Marriage id of wife’s next most recent prior marriage              |
#' |            8 | hprior   | Marriage id of husband’s next most recent prior marriage           |
#' 
#' you can either provide the complete `path` to the file or the `folder`, supfilename, seed and suffix with which you
#' started the simulation
#' 
#' @param folder simulation base folder ("~/socsim/simulation_235/")
#' @param supfile name of supplement-file ("socsim.sup")
#' @param seed random number seed (42)
#' @param suffix optional suffix for the results-directory (default="")
#' @param fn complete path to the file. If not provided, it will be created from the other arguments
#'
#' @return Data frame with the information of the output population file
#' 
#' @export
read_omar <- function(folder=NULL, supfile="socsim.sup",seed=42,suffix="",fn=NULL){

  # create fn
  if (is.null(fn)){
    fn <- paste0(folder,"/sim_results_", supfile, "_",seed,"_",suffix, "/result.omar")
  } else {
    fn <- fn
  }
  print(paste0("read marriage file: ",fn))
  omar<-read.table(file = fn, header = F, as.is = T)
  names(omar)<-c("mid","wpid","hpid","dstart","dend", "rend","wprior","hprior")
  return(omar)
}


#' Read output population file into a data frame
#' 
#' after the end of the simulation, socsim writes every person of the simulation into
#' a file called result.opop. This file contains the following information about
#' each person:
#'
#' | **position** | **name** | **description**                                                                       |
#' | -----------: | :------- | :------------------------------------------------------------------------------------ |
#' |            1 | pid      | Person id unique identifier assigned as integer in birth order                        |
#' |            2 | fem      | 1 if female 0 if male                                                                 |
#' |            3 | group    | Group identifier 1..60 current group membership of individual                         |
#' |            4 | nev      | Next scheduled event                                                                  |
#' |            5 | dob      | Date of birth integer month number                                                    |
#' |            6 | mom      | Person id of mother                                                                   |
#' |            7 | pop      | Person id of father                                                                   |
#' |            8 | nesibm   | Person id of next eldest sibling through mother                                       |
#' |            9 | nesibp   | Person id of next eldest sibling through father                                       |
#' |           10 | lborn    | Person id of last born child                                                          |
#' |           11 | marid    | Id of marriage in .omar file                                                          |
#' |           12 | mstat    | Marital status at end of simulation integer 1=single;2=divorced; 3=widowed; 4=married |
#' |           13 | dod      | Date of death or 0 if alive at end of simulation                                      |
#' |           14 | fmult    | Fertility multiplier                                                                  |
#' 
#' you can either provide the complete `path` to the file or the `folder`, supfilename, seed and suffix with which you
#' started the simulation
#' 
#' @param folder simulation base folder ("~/socsim/simulation_235/")
#' @param supfile name of supplement-file ("socsim.sup")
#' @param seed random number seed (42)
#' @param suffix optional suffix for the results-directory (default="")
#' @param fn complete path to the file. If not provided, it will be created from the other arguments
#'
#' @return Data frame with the information of the output population file
#' 
#' @export
read_opop <- function(folder=NULL, supfile="socsim.sup",seed=42,suffix="",fn=NULL){

  # create fn
  if (is.null(fn)){
    fn <- paste0(folder,"/sim_results_", supfile, "_",seed,"_",suffix, "/result.opop")
  } else {
    fn <- fn
  }
  
  print(paste0("read population file: ",fn))
  opop <- read.table(file=fn,header=F,as.is=T)  
  ## assign names to columns
  names(opop)<-c("pid","fem","group",
                 "nev","dob","mom","pop","nesibm","nesibp",
                 "lborn","marid","mstat","dod","fmult")
  return(opop)
}
