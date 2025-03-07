#' Identify members of a kin network for an individual or individuals of 
#' interest. 
#' @param opop An R object from SOCSIM microsimulation output (population file). Create this object with the function read_opop().
#' @param omar An R object from SOCSIM microsimulation output (marriage file). Create this object with the function read_omar().
#' @param pid A character vector of person IDs, indicating persons of interest
#' for whom these kin networks should be identified.
#' @param extra_kintypes A vector of character values indicating which
#' additional types of kin should be obtained. For reasons of computational
#' efficiency, the function will by default only identify an individual's
#'  great-grandparents ("ggparents" in function output), 
#'  grandparents ("gparents"), parents, 
#'  siblings, spouse, children, and grandchildren ("gchildren"). 
#'  However, by selecting one or more of the following 
#'  kin types, the kin network generated will also include these individuals:
#'  \itemize{
#'  \item "gunclesaunts": Great-uncles and great-aunts
#'  \item "unclesaunts": Uncles and aunts
#'  \item "firstcousins": First cousins (Children of uncles and aunts)
#'  \item "niblings": Nieces and nephews (Children of siblings)
#'  \item "inlaws": Parents-in-law (parents of spouse) and brothers and sisters
#'  in law (siblings of spouse and spouse of siblings)
#' }
#' @param kin_by_sex A logical value indicating whether output should include
#' kin relations additionally disaggregated by the sex of the relative. Setting
#' this value to TRUE will result in additional objects being generated
#' to identify individuals' relatives by sex. 
#' @return An R object containing a list of lists with person IDs
#' of kin, organized by relationship. These person ID values will be named 
#' based on the person of interest with whom they are associated.
#' For example, for a list named "parents", the values will be person IDs of
#' the parents of individuals of interest. These values will be named according
#' to their children's IDs (given that their children are, in this case, 
#' the persons of interest provided to the function input).
#' With kin_by_sex set to TRUE and extra_kintypes set to c(c("gunclesaunts",
#'  "unclesaunts", "firstcousins", "niblings", "inlaws")), 
#'  the full list of kin relations identified are:
#'  \itemize{
#'  \item "ggparents": great-grandparents
#'  \item "ggmothers": great-grandmothers
#'  \item "ggfathers": great-grandfathers
#'  \item "gparents": grandparents
#'  \item "gmothers": grandmothers
#'  \item "gfathers": grandfathers
#'  \item "gunclesaunts": great-uncles and great-aunts
#'  \item "guncles": great-uncles
#'  \item "gaunts": great-aunts
#'  \item "parents": parents
#'  \item "mother": mother
#'  \item "father": father
#'  \item "unclesaunts": uncles and aunts (siblings of parents)
#'  \item "uncles": uncles 
#'  \item "aunts": aunts
#'  \item "siblings": siblings
#'  \item "sisters": sisters
#'  \item "brothers": brothers
#'  \item "firstcousins": first cousins
#'  \item "firstcousinsfemale": female first cousins
#'  \item "firstcousinsmale": male first cousins
#'  \item "children": children
#'  \item "daughters": daughters
#'  \item "sons": sons
#'  \item "gchildren": grandchildren
#'  \item "gdaughters": granddaughters
#'  \item "gsons": grandsons
#'  \item "niblings": nephews and nieces
#'  \item "nieces": nieces
#'  \item "nephews": nephews
#'  \item "spouse": spouse (based on final marriage, in the case of 
#'  multiple marriages)
#'  \item "parentsinlaw": parents-in-law
#'  \item "motherinlaw": mother-in-law
#'  \item "fatherinlaw": father-in-law
#'  \item "siblingsinlaw": brothers and sisters in law
#'  \item "sistersinlaw": sisters-in-law
#'  \item "brothersinlaw": brothers-in-law
#'  }
#' 
#'@examples
#' \dontrun{
#' #Individuals of interest
#' pid <- c("10111", "10211", "10311")
#' #Obtain partial kinship network, with omar and opop already in R environment
#' kin_network <- getKin(opop = opop, omar = omar, pid = pid, 
#' extra_kintypes = c("unclesaunts", "niblings"), kin_by_sex = TRUE)
#' }
#' 
#' @export
retrieve_kin <- function(opop = opop, omar = omar, KidsOf = KidsOf, pid, 
                   extra_kintypes, 
                   kin_by_sex) {
  
  ###Generic Helper Functions
  #Negation
  '%ni%' <- Negate('%in%')
  
  #NA values
  zna <- function(x){return(ifelse(x==0,NA,x))}
  
  ###Ancestors and Spouses
  opop$spouse<-ifelse(opop$fem,
                      (omar[zna(opop$marid),"hpid"]),
                      (omar[zna(opop$marid),"wpid"]))
  
  #Ancestors
  opop$FM<-opop$mom[match(opop$pop, opop$pid)]
  opop$MM<-opop$mom[match(opop$mom, opop$pid)]
  opop$MF<-opop$pop[match(opop$mom, opop$pid)]
  opop$FF<-opop$pop[match(opop$pop, opop$pid)]
  
  opop$FFM<-opop$FM[match(opop$pop, opop$pid)]
  opop$FMM<-opop$MM[match(opop$pop, opop$pid)]
  opop$FMF<-opop$MF[match(opop$pop, opop$pid)]
  opop$FFF<-opop$FF[match(opop$pop, opop$pid)]
  
  opop$MFM<-opop$FM[match(opop$mom, opop$pid)]
  opop$MMM<-opop$MM[match(opop$mom, opop$pid)]
  opop$MMF<-opop$MF[match(opop$mom, opop$pid)]
  opop$MFF<-opop$FF[match(opop$mom, opop$pid)]
  
  ###Children
  #Preparing to find kin relationships
  # Function for finding children
  kidsOf<-with(opop,{c(
    tapply(pid,mom,c),
    tapply(pid,pop,c)
  )})
  #This finds us all the instances where a given PID is a mother or father
  #And packages all of these together in a list
  
  #Using indexes saves space
  #We have to run both of these since there are two 0s
  kidsOf["0"]<-NULL;  kidsOf["0"]<-NULL
  KidsOf<-list()
  KidsOf[as.numeric(names(kidsOf))]<-kidsOf
  
  
  ###Helper Functions
  #Helper function for subsetting kids
  ko <- function(KidsOf = KidsOf, p){
    lapply(p,function(x){unique(as.vector(unlist(KidsOf[x])))})
  }
  
  #Helper function for finding the spouse as previously identified
  so <- function(opop = opop, p){
    lapply(p,function(p){as.vector(unlist(opop[opop$pid %in% p,
                                               c("spouse")]))})
  }
  
  ###Identifying Kin
  #Creating a list to hold these various kin relationships
  res<-list()
  
  ##----The family tree above ego
  #Great grandparents
  res$ggparents <- lapply(pid,
                          function(pid){
                            as.vector(unlist(opop[opop$pid == pid, 
                                                  c("MMM","MMF","MFM","MFF",
                                                    "FMM","FMF","FFM","FFF")]))})
  
  #--Grandparents
  res$gparents <- lapply(pid,
                         function(pid){
                           as.vector(unlist(opop[opop$pid == pid,
                                                 c("MM","MF","FM","FF")]))})
  if(kin_by_sex){
    res$gmothers <- lapply(pid,
                           function(pid){
                             as.vector(unlist(opop[opop$pid == pid,
                                                   c("MM","FM")]))})
    
    res$gfathers <- lapply(pid,
                           function(pid){
                             as.vector(unlist(opop[opop$pid == pid, 
                                                   c("MF","FF")]))})
  }
  
  #--Great-uncles and aunts
  if("gunclesaunts" %in% extra_kintypes) {
    #Grandparents, great-uncles, great-aunts
    g1 <- ko(KidsOf = KidsOf, p = res$ggparents)
    #Great-uncles and aunts
    res$gunclesaunts <- lapply(seq_along(g1), 
                               function(i) g1[[i]][g1[[i]] %ni% res$gparents[[i]]])
    if(kin_by_sex){
      res$gaunts <- lapply(seq_along(res$gunclesaunts), function(i) res$gunclesaunts[[i]][which(opop$fem[match(res$gunclesaunts[[i]], opop$pid)] == 1)])
      res$guncles <- lapply(seq_along(res$gunclesaunts), function(i) res$gunclesaunts[[i]][which(opop$fem[match(res$gunclesaunts[[i]], opop$pid)] == 0)])
    }
  }
  
  #--Parents
  res$parents <- lapply(pid,
                        function(pid){
                          as.vector(unlist(opop[opop$pid == pid, c("mom","pop")]))})
  if(kin_by_sex){
    res$mother <- lapply(pid,
                         function(pid){
                           as.vector(unlist(opop[opop$pid == pid, "mom"]))})
    res$father <- lapply(pid,
                         function(pid){
                           as.vector(unlist(opop[opop$pid == pid, "pop"]))})
  }
  
  #--Uncles and Aunts
  if("unclesaunts" %in% extra_kintypes) {
    #Parents, uncles, and aunts
    g2 <- ko(KidsOf = KidsOf, p = res$gparents)
    #Uncles and Aunts
    res$unclesaunts <- lapply(seq_along(g2), function(i) g2[[i]][g2[[i]] %ni% res$parents[[i]]])
    if(kin_by_sex) {
      res$aunts <- lapply(seq_along(res$unclesaunts), function(i) res$unclesaunts[[i]][which(opop$fem[match(res$unclesaunts[[i]], opop$pid)] == 1)])
      res$uncles <- lapply(seq_along(res$unclesaunts), function(i) res$unclesaunts[[i]][which(opop$fem[match(res$unclesaunts[[i]], opop$pid)] == 0)])
    }
  }
  
  ##----The family tree at ego’s generation (g3)
  #--Siblings
  res$siblings <- ko(KidsOf = KidsOf, p = res$parents)
  #Making sure to remove ego
  res$siblings <- lapply(seq_along(res$siblings), function(i) res$siblings[[i]][res$siblings[[i]] %ni% pid[[i]]])
  if(kin_by_sex){
    res$sisters <- lapply(seq_along(res$siblings), function(i) res$siblings[[i]][which(opop$fem[match(res$siblings[[i]], opop$pid)] == 1)])
    res$brothers <- lapply(seq_along(res$siblings), function(i) res$siblings[[i]][which(opop$fem[match(res$siblings[[i]], opop$pid)] == 0)])
  }
  
  #--First cousins
  if("firstcousins" %in% extra_kintypes){
    res$firstcousins <- ko(KidsOf = KidsOf, p = res$unclesaunts)
    if(kin_by_sex){
      res$firstcousinsfemale <- lapply(seq_along(res$firstcousins), function(i) res$firstcousins[[i]][which(opop$fem[match(res$firstcousins[[i]], opop$pid)] == 1)])
      res$firstcousinsmale <- lapply(seq_along(res$firstcousins), function(i) res$firstcousins[[i]][which(opop$fem[match(res$firstcousins[[i]], opop$pid)] == 0)])
    }
  }
  
  ##----The family tree below ego’s generation (g4 etc)
  #--Children
  res$children <- ko(KidsOf = KidsOf, p = pid)
  if(kin_by_sex){
    res$daughters <- lapply(seq_along(res$children), function(i) res$children[[i]][which(opop$fem[match(res$children[[i]], opop$pid)] == 1)])
    res$sons <- lapply(seq_along(res$children), function(i) res$children[[i]][which(opop$fem[match(res$children[[i]], opop$pid)] == 0)])
  }
  
  #--Grandchildren
  res$gchildren <- ko(KidsOf = KidsOf, p = res$children)
  if(kin_by_sex){
    res$gdaughters <- lapply(seq_along(res$gchildren), function(i) res$gchildren[[i]][which(opop$fem[match(res$gchildren[[i]], opop$pid)] == 1)])
    res$gsons <- lapply(seq_along(res$gchildren), function(i) res$gchildren[[i]][which(opop$fem[match(res$gchildren[[i]], opop$pid)] == 0)])
  }
  
  #--Nephews and Nieces (Niblings)
  if("niblings" %in% extra_kintypes) {
    res$niblings <- ko(KidsOf = KidsOf, p = res$siblings)
    res$nieces <- lapply(seq_along(res$niblings), function(i) res$niblings[[i]][which(opop$fem[match(res$niblings[[i]], opop$pid)] == 1)])
    res$nephews <- lapply(seq_along(res$niblings), function(i) res$niblings[[i]][which(opop$fem[match(res$niblings[[i]], opop$pid)] == 0)])
  }
  
  #--Spouse
  res$spouse <- so(opop = opop, p = pid)
  
  #--In-laws
  if("inlaws" %in% extra_kintypes) {
    #Parents-in-law
    res$parentsinlaw <- lapply(res$spouse,
                               function(x){
                                 unlist(opop[opop$pid %in% x,
                                             c("mom","pop")])})
    if(kin_by_sex){
      res$motherinlaw <- lapply(res$spouse,
                                function(x){
                                  unlist(opop[opop$pid %in% x,
                                              "mom"])})
      res$fatherinlaw <- lapply(res$spouse,
                                function(x){
                                  unlist(opop[opop$pid %in% x,
                                              "pop"])})
    }
    
    #Brothers and sister in laws = siblings of spouse + spouse of siblings + spouse of spouse's siblings
    spouseofsiblings <- so(opop = opop, p = res$siblings)
    siblingsofspouse <- ko(KidsOf = KidsOf, p = res$parentsinlaw)
    siblingsofspouse <- lapply(seq_along(siblingsofspouse), #Remove spouse from children of parents-in-law
                               function(i) siblingsofspouse[[i]][siblingsofspouse[[i]] %ni% res$spouse[[i]]])
    spouseofsiblingsofspouse <- so(opop = opop, p = siblingsofspouse)
    res$siblingsinlaw <- mapply(c, spouseofsiblings, siblingsofspouse, spouseofsiblingsofspouse, SIMPLIFY=F)
    if(kin_by_sex){
      res$sistersinlaw <- lapply(seq_along(res$siblingsinlaw), function(i) res$siblingsinlaw[[i]][which(opop$fem[match(res$siblingsinlaw[[i]], opop$pid)] == 1)])
      res$brothersinlaw <- lapply(seq_along(res$siblingsinlaw), function(i) res$siblingsinlaw[[i]][which(opop$fem[match(res$siblingsinlaw[[i]], opop$pid)] == 0)])
    }
  }
  
  #--Data cleaning
  #Replacing NULLs and integer(0) with NA
  #It may be necessary later to remove NA values
  for (i in 1:length(names(res))) {
    res[[i]][sapply(res[[i]], function(x) length(x)==0)] <- NA #removing NULLs and integer(0) with NA
  }
  
    # "Additional lines of code from Mallika"
  res$ggparents <- lapply(pid, function(pid){
    as.vector(
      unlist(
        opop[opop$pid == pid, c("MMM","MMF","MFM","MFF","FMM","FMF","FFM","FFF")]
      )
    )
  })
  
  if(kin_by_sex){ 
    res$ggmothers <- lapply(pid,function(pid){
      as.vector(unlist(opop[opop$pid == pid, c("MMM","FMM", "FFM", "MFM")]))
    })
    res$ggfathers <- lapply(pid, function(pid){
      as.vector(unlist(opop[opop$pid == pid, c("FFF", "MMF", "MFF", "FMF")]))
    })
  }
  return(res)
}
