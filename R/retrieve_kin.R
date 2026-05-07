#' Identify members of a kin network for an individual or individuals of
#' interest.
#' @param opop An R object from SOCSIM microsimulation output (population file). Create this object with the function read_opop().
#' @param omar An R object from SOCSIM microsimulation output (marriage file). Create this object with the function read_omar().
#' @param pid A vector of person IDs, indicating persons of interest
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
#' @param KidsOf An optional precomputed list object containing the children of
#' each person in the population. If `NULL`, it is built from `opop`.
#'
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
#' kin_network <- retrieve_kin(opop = opop, omar = omar, pid = pid,
#' extra_kintypes = c("unclesaunts", "niblings"), kin_by_sex = TRUE)
#' }
#'
#' @export
retrieve_kin <- function(opop, omar, pid,
                         extra_kintypes = character(),
                         kin_by_sex = FALSE,
                         KidsOf = NULL) {

  `%ni%` <- Negate(`%in%`)

  pid <- as.vector(pid)
  pid_keys <- as.character(pid)
  extra_kintypes <- unique(extra_kintypes)

  pid_index <- seq_len(nrow(opop))
  names(pid_index) <- as.character(opop$pid)

  lookup_column <- function(ids, column) {
    idx <- unname(pid_index[as.character(ids)])
    opop[[column]][idx]
  }

  subset_by_sex <- function(relatives, sex_value) {
    lapply(relatives, function(x) {
      x[sex_lookup[as.character(x)] == sex_value]
    })
  }

  list_for_pid <- function(row_index, columns) {
    lapply(row_index, function(idx) {
      if (is.na(idx)) {
        return(vector(mode = "numeric", length = 0L))
      }
      as.vector(unlist(opop[idx, columns, drop = FALSE], use.names = FALSE))
    })
  }

  if (is.null(KidsOf) || length(KidsOf) == 0L) {
    mother_kids <- split(opop$pid[opop$mom != 0], as.character(opop$mom[opop$mom != 0]))
    father_kids <- split(opop$pid[opop$pop != 0], as.character(opop$pop[opop$pop != 0]))
    parent_ids <- union(names(mother_kids), names(father_kids))
    KidsOf <- stats::setNames(vector("list", length(parent_ids)), parent_ids)
    for (parent_id in parent_ids) {
      KidsOf[[parent_id]] <- c(mother_kids[[parent_id]], father_kids[[parent_id]])
    }
  } else if (length(KidsOf) && is.null(names(KidsOf))) {
    names(KidsOf) <- as.character(seq_along(KidsOf))
  }

  ko <- function(p) {
    lapply(p, function(x) {
      ids <- as.character(x[!is.na(x)])
      unique(as.vector(unlist(KidsOf[ids], use.names = FALSE)))
    })
  }

  spouse_lookup <- rep(NA, nrow(opop))
  marriage_index <- match(opop$marid, omar$mid)
  valid_marriage <- !is.na(marriage_index)
  wife_rows <- valid_marriage & opop$fem == 1
  husband_rows <- valid_marriage & opop$fem == 0
  spouse_lookup[wife_rows] <- omar$hpid[marriage_index[wife_rows]]
  spouse_lookup[husband_rows] <- omar$wpid[marriage_index[husband_rows]]
  opop$spouse <- spouse_lookup
  sex_lookup <- opop$fem
  names(sex_lookup) <- as.character(opop$pid)
  spouse_lookup <- opop$spouse
  names(spouse_lookup) <- as.character(opop$pid)

  opop$FM <- lookup_column(opop$pop, "mom")
  opop$MM <- lookup_column(opop$mom, "mom")
  opop$MF <- lookup_column(opop$mom, "pop")
  opop$FF <- lookup_column(opop$pop, "pop")

  opop$FFM <- lookup_column(opop$pop, "FM")
  opop$FMM <- lookup_column(opop$pop, "MM")
  opop$FMF <- lookup_column(opop$pop, "MF")
  opop$FFF <- lookup_column(opop$pop, "FF")

  opop$MFM <- lookup_column(opop$mom, "FM")
  opop$MMM <- lookup_column(opop$mom, "MM")
  opop$MMF <- lookup_column(opop$mom, "MF")
  opop$MFF <- lookup_column(opop$mom, "FF")

  so <- function(p) {
    lapply(p, function(x) {
      as.vector(unname(spouse_lookup[as.character(x)]))
    })
  }

  ego_index <- unname(pid_index[pid_keys])
  res <- list()

  res$ggparents <- list_for_pid(ego_index, c("MMM", "MMF", "MFM", "MFF", "FMM", "FMF", "FFM", "FFF"))
  res$gparents <- list_for_pid(ego_index, c("MM", "MF", "FM", "FF"))

  if (kin_by_sex) {
    res$gmothers <- list_for_pid(ego_index, c("MM", "FM"))
    res$gfathers <- list_for_pid(ego_index, c("MF", "FF"))
    res$ggmothers <- list_for_pid(ego_index, c("MMM", "FMM", "FFM", "MFM"))
    res$ggfathers <- list_for_pid(ego_index, c("FFF", "MMF", "MFF", "FMF"))
  }

  if ("gunclesaunts" %in% extra_kintypes) {
    g1 <- ko(res$ggparents)
    res$gunclesaunts <- lapply(seq_along(g1), function(i) g1[[i]][g1[[i]] %ni% res$gparents[[i]]])
    if (kin_by_sex) {
      res$gaunts <- subset_by_sex(res$gunclesaunts, 1)
      res$guncles <- subset_by_sex(res$gunclesaunts, 0)
    }
  }

  res$parents <- list_for_pid(ego_index, c("mom", "pop"))
  if (kin_by_sex) {
    res$mother <- list_for_pid(ego_index, "mom")
    res$father <- list_for_pid(ego_index, "pop")
  }

  needs_unclesaunts <- any(c("unclesaunts", "firstcousins") %in% extra_kintypes)
  unclesaunts <- NULL
  if (needs_unclesaunts) {
    g2 <- ko(res$gparents)
    unclesaunts <- lapply(seq_along(g2), function(i) g2[[i]][g2[[i]] %ni% res$parents[[i]]])
    if ("unclesaunts" %in% extra_kintypes) {
      res$unclesaunts <- unclesaunts
      if (kin_by_sex) {
        res$aunts <- subset_by_sex(res$unclesaunts, 1)
        res$uncles <- subset_by_sex(res$unclesaunts, 0)
      }
    }
  }

  res$siblings <- ko(res$parents)
  res$siblings <- lapply(seq_along(res$siblings), function(i) res$siblings[[i]][res$siblings[[i]] %ni% pid[[i]]])
  if (kin_by_sex) {
    res$sisters <- subset_by_sex(res$siblings, 1)
    res$brothers <- subset_by_sex(res$siblings, 0)
  }

  if ("firstcousins" %in% extra_kintypes) {
    res$firstcousins <- ko(unclesaunts)
    if (kin_by_sex) {
      res$firstcousinsfemale <- subset_by_sex(res$firstcousins, 1)
      res$firstcousinsmale <- subset_by_sex(res$firstcousins, 0)
    }
  }

  res$children <- ko(as.list(pid))
  if (kin_by_sex) {
    res$daughters <- subset_by_sex(res$children, 1)
    res$sons <- subset_by_sex(res$children, 0)
  }

  res$gchildren <- ko(res$children)
  if (kin_by_sex) {
    res$gdaughters <- subset_by_sex(res$gchildren, 1)
    res$gsons <- subset_by_sex(res$gchildren, 0)
  }

  if ("niblings" %in% extra_kintypes) {
    res$niblings <- ko(res$siblings)
    if (kin_by_sex) {
      res$nieces <- subset_by_sex(res$niblings, 1)
      res$nephews <- subset_by_sex(res$niblings, 0)
    }
  }

  res$spouse <- so(as.list(pid))

  if ("inlaws" %in% extra_kintypes) {
    res$parentsinlaw <- lapply(res$spouse, function(x) {
      as.vector(unlist(opop[opop$pid %in% x, c("mom", "pop"), drop = FALSE], use.names = FALSE))
    })
    if (kin_by_sex) {
      res$motherinlaw <- lapply(res$spouse, function(x) {
        as.vector(unlist(opop[opop$pid %in% x, "mom", drop = FALSE], use.names = FALSE))
      })
      res$fatherinlaw <- lapply(res$spouse, function(x) {
        as.vector(unlist(opop[opop$pid %in% x, "pop", drop = FALSE], use.names = FALSE))
      })
    }

    spouseofsiblings <- so(res$siblings)
    siblingsofspouse <- ko(res$parentsinlaw)
    siblingsofspouse <- lapply(seq_along(siblingsofspouse), function(i) {
      siblingsofspouse[[i]][siblingsofspouse[[i]] %ni% res$spouse[[i]]]
    })
    spouseofsiblingsofspouse <- so(siblingsofspouse)
    res$siblingsinlaw <- mapply(c, spouseofsiblings, siblingsofspouse, spouseofsiblingsofspouse, SIMPLIFY = FALSE)
    if (kin_by_sex) {
      res$sistersinlaw <- subset_by_sex(res$siblingsinlaw, 1)
      res$brothersinlaw <- subset_by_sex(res$siblingsinlaw, 0)
    }
  }

  for (relation_name in names(res)) {
    relation <- res[[relation_name]]
    relation[vapply(relation, function(x) length(x) == 0L, logical(1))] <- list(NA)
    names(relation) <- pid_keys
    res[[relation_name]] <- relation
  }

  res
}
