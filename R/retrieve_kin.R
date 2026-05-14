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
#' @return A named list whose components are kinship categories such as
#' `parents`, `siblings`, or `children`. Each component is itself a named list
#' of integer person IDs, organized by relationship. These person ID values will be named
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

  pid_vector <- opop$pid
  pid_index <- seq_len(nrow(opop))
  names(pid_index) <- as.character(pid_vector)
  mom_pid <- opop$mom
  pop_pid <- opop$pop
  sex_vector <- opop$fem
  marid <- opop$marid

  row_index_of <- function(ids) {
    unname(pid_index[as.character(ids)])
  }

  map_rows <- function(row_index, values) {
    out <- rep.int(NA_integer_, length(row_index))
    valid <- !is.na(row_index)
    out[valid] <- values[row_index[valid]]
    out
  }

  list_from_vectors <- function(row_index, vectors) {
    lapply(row_index, function(idx) {
      if (is.na(idx)) {
        return(vector(mode = "numeric", length = 0L))
      }
      as.vector(unlist(lapply(vectors, `[`, idx), use.names = FALSE))
    })
  }

  rows_to_pid <- function(relatives) {
    lapply(relatives, function(x) {
      if (!length(x)) {
        return(vector(mode = "numeric", length = 0L))
      }
      pid_vector[x]
    })
  }

  subset_pid_by_sex <- function(relatives, sex_value) {
    sex_lookup <- sex_vector
    names(sex_lookup) <- as.character(pid_vector)
    lapply(relatives, function(x) {
      x[sex_lookup[as.character(x)] == sex_value]
    })
  }

  subset_row_by_sex <- function(relatives, sex_value) {
    lapply(relatives, function(x) {
      valid <- !is.na(x) & sex_vector[x] == sex_value
      x[valid]
    })
  }

  spouse_pid_lookup <- rep(NA, nrow(opop))
  marriage_index <- match(marid, omar$mid)
  valid_marriage <- !is.na(marriage_index)
  wife_rows <- valid_marriage & sex_vector == 1
  husband_rows <- valid_marriage & sex_vector == 0
  spouse_pid_lookup[wife_rows] <- omar$hpid[marriage_index[wife_rows]]
  spouse_pid_lookup[husband_rows] <- omar$wpid[marriage_index[husband_rows]]
  spouse_row_lookup <- row_index_of(spouse_pid_lookup)

  spouse_pid_lookup_named <- spouse_pid_lookup
  names(spouse_pid_lookup_named) <- as.character(pid_vector)

  mom_row <- row_index_of(mom_pid)
  pop_row <- row_index_of(pop_pid)

  so_pid <- function(relatives) {
    lapply(relatives, function(x) {
      as.vector(unname(spouse_pid_lookup_named[as.character(x)]))
    })
  }

  so_row <- function(relatives) {
    lapply(relatives, function(x) {
      if (!length(x)) {
        return(vector(mode = "numeric", length = 0L))
      }
      as.vector(spouse_row_lookup[x])
    })
  }

  if (length(KidsOf) && is.null(names(KidsOf))) {
    names(KidsOf) <- as.character(seq_along(KidsOf))
  }

  kids_lookup_rows <- NULL
  use_row_lookup <- TRUE

  if (is.null(KidsOf) || length(KidsOf) == 0L) {
    mother_rows <- which(!is.na(mom_row))
    father_rows <- which(!is.na(pop_row))
    mother_split <- split(mother_rows, mom_row[mother_rows])
    father_split <- split(father_rows, pop_row[father_rows])
    parent_rows <- union(names(mother_split), names(father_split))
    kids_lookup_rows <- vector("list", nrow(opop))
    if (length(parent_rows)) {
      parent_rows_int <- as.integer(parent_rows)
      kids_lookup_rows[parent_rows_int] <- Map(c, mother_split[parent_rows], father_split[parent_rows])
    }
  } else {
    parent_rows <- row_index_of(names(KidsOf))
    use_row_lookup <- !anyNA(parent_rows)

    if (use_row_lookup) {
      child_lengths <- lengths(KidsOf)
      total_children <- sum(child_lengths)
      child_rows <- vector("list", length(KidsOf))

      if (total_children) {
        flat_children <- unlist(KidsOf, use.names = FALSE)
        flat_child_rows <- row_index_of(flat_children)
        valid_children <- !is.na(flat_children)

        if (any(valid_children & is.na(flat_child_rows))) {
          use_row_lookup <- FALSE
        } else {
          child_groups <- split(flat_child_rows, rep.int(seq_along(KidsOf), child_lengths))
          child_rows[as.integer(names(child_groups))] <- child_groups
        }
      }

      if (use_row_lookup) {
        kids_lookup_rows <- vector("list", nrow(opop))
        kids_lookup_rows[parent_rows] <- child_rows
      }
    }
  }

  ko_pid <- function(relatives) {
    lapply(relatives, function(x) {
      ids <- as.character(x[!is.na(x)])
      unique(as.vector(unlist(KidsOf[ids], use.names = FALSE)))
    })
  }

  ko_row <- function(relatives) {
    lapply(relatives, function(x) {
      parent_rows <- x[!is.na(x)]
      if (!length(parent_rows)) {
        return(vector(mode = "numeric", length = 0L))
      }
      unique(as.vector(unlist(kids_lookup_rows[parent_rows], use.names = FALSE)))
    })
  }

  FM_row <- map_rows(pop_row, mom_row)
  MM_row <- map_rows(mom_row, mom_row)
  MF_row <- map_rows(mom_row, pop_row)
  FF_row <- map_rows(pop_row, pop_row)

  FFM_row <- map_rows(pop_row, FM_row)
  FMM_row <- map_rows(pop_row, MM_row)
  FMF_row <- map_rows(pop_row, MF_row)
  FFF_row <- map_rows(pop_row, FF_row)

  MFM_row <- map_rows(mom_row, FM_row)
  MMM_row <- map_rows(mom_row, MM_row)
  MMF_row <- map_rows(mom_row, MF_row)
  MFF_row <- map_rows(mom_row, FF_row)

  ego_rows <- row_index_of(pid)
  parent_rows <- list_from_vectors(ego_rows, list(mom_row, pop_row))
  gparent_rows <- list_from_vectors(ego_rows, list(MM_row, MF_row, FM_row, FF_row))
  ggparent_rows <- list_from_vectors(ego_rows, list(MMM_row, MMF_row, MFM_row, MFF_row, FMM_row, FMF_row, FFM_row, FFF_row))

  ko <- if (use_row_lookup) ko_row else ko_pid
  subset_by_sex <- if (use_row_lookup) subset_row_by_sex else subset_pid_by_sex
  so <- if (use_row_lookup) so_row else so_pid
  convert_relatives <- if (use_row_lookup) rows_to_pid else identity
  ego_relatives <- if (use_row_lookup) as.list(ego_rows) else as.list(pid)
  parents_rel <- if (use_row_lookup) parent_rows else list_from_vectors(ego_rows, list(mom_pid, pop_pid))
  gparents_rel <- if (use_row_lookup) gparent_rows else rows_to_pid(gparent_rows)
  ggparents_rel <- if (use_row_lookup) ggparent_rows else rows_to_pid(ggparent_rows)

  res <- list()
  res$ggparents <- rows_to_pid(ggparent_rows)
  res$gparents <- rows_to_pid(gparent_rows)

  if (kin_by_sex) {
    res$gmothers <- rows_to_pid(list_from_vectors(ego_rows, list(MM_row, FM_row)))
    res$gfathers <- rows_to_pid(list_from_vectors(ego_rows, list(MF_row, FF_row)))
    res$ggmothers <- rows_to_pid(list_from_vectors(ego_rows, list(MMM_row, FMM_row, FFM_row, MFM_row)))
    res$ggfathers <- rows_to_pid(list_from_vectors(ego_rows, list(FFF_row, MMF_row, MFF_row, FMF_row)))
  }

  if ("gunclesaunts" %in% extra_kintypes) {
    gunclesaunts <- ko(ggparents_rel)
    gunclesaunts <- lapply(seq_along(gunclesaunts), function(i) gunclesaunts[[i]][gunclesaunts[[i]] %ni% gparents_rel[[i]]])
    res$gunclesaunts <- convert_relatives(gunclesaunts)
    if (kin_by_sex) {
      res$gaunts <- convert_relatives(subset_by_sex(gunclesaunts, 1))
      res$guncles <- convert_relatives(subset_by_sex(gunclesaunts, 0))
    }
  }

  res$parents <- list_from_vectors(ego_rows, list(mom_pid, pop_pid))
  if (kin_by_sex) {
    res$mother <- list_from_vectors(ego_rows, list(mom_pid))
    res$father <- list_from_vectors(ego_rows, list(pop_pid))
  }

  needs_unclesaunts <- any(c("unclesaunts", "firstcousins") %in% extra_kintypes)
  unclesaunts <- NULL
  if (needs_unclesaunts) {
    unclesaunts <- ko(gparents_rel)
    unclesaunts <- lapply(seq_along(unclesaunts), function(i) unclesaunts[[i]][unclesaunts[[i]] %ni% parents_rel[[i]]])
    if ("unclesaunts" %in% extra_kintypes) {
      res$unclesaunts <- convert_relatives(unclesaunts)
      if (kin_by_sex) {
        res$aunts <- convert_relatives(subset_by_sex(unclesaunts, 1))
        res$uncles <- convert_relatives(subset_by_sex(unclesaunts, 0))
      }
    }
  }

  siblings <- ko(parents_rel)
  self_ids <- if (use_row_lookup) ego_rows else pid
  siblings <- lapply(seq_along(siblings), function(i) siblings[[i]][siblings[[i]] %ni% self_ids[[i]]])
  res$siblings <- convert_relatives(siblings)
  if (kin_by_sex) {
    res$sisters <- convert_relatives(subset_by_sex(siblings, 1))
    res$brothers <- convert_relatives(subset_by_sex(siblings, 0))
  }

  if ("firstcousins" %in% extra_kintypes) {
    firstcousins <- ko(unclesaunts)
    res$firstcousins <- convert_relatives(firstcousins)
    if (kin_by_sex) {
      res$firstcousinsfemale <- convert_relatives(subset_by_sex(firstcousins, 1))
      res$firstcousinsmale <- convert_relatives(subset_by_sex(firstcousins, 0))
    }
  }

  children <- ko(ego_relatives)
  res$children <- convert_relatives(children)
  if (kin_by_sex) {
    res$daughters <- convert_relatives(subset_by_sex(children, 1))
    res$sons <- convert_relatives(subset_by_sex(children, 0))
  }

  gchildren <- ko(children)
  res$gchildren <- convert_relatives(gchildren)
  if (kin_by_sex) {
    res$gdaughters <- convert_relatives(subset_by_sex(gchildren, 1))
    res$gsons <- convert_relatives(subset_by_sex(gchildren, 0))
  }

  if ("niblings" %in% extra_kintypes) {
    niblings <- ko(siblings)
    res$niblings <- convert_relatives(niblings)
    if (kin_by_sex) {
      res$nieces <- convert_relatives(subset_by_sex(niblings, 1))
      res$nephews <- convert_relatives(subset_by_sex(niblings, 0))
    }
  }

  spouse_rel <- so(ego_relatives)
  res$spouse <- convert_relatives(spouse_rel)

  if ("inlaws" %in% extra_kintypes) {
    spouse_rows <- if (use_row_lookup) spouse_rel else lapply(spouse_rel, row_index_of)
    parentsinlaw_rows <- lapply(spouse_rows, function(x) {
      spouse_idx <- x[!is.na(x)]
      if (!length(spouse_idx)) {
        return(vector(mode = "numeric", length = 0L))
      }
      c(mom_row[spouse_idx], pop_row[spouse_idx])
    })
    res$parentsinlaw <- lapply(spouse_rows, function(x) {
      spouse_idx <- x[!is.na(x)]
      if (!length(spouse_idx)) {
        return(vector(mode = "numeric", length = 0L))
      }
      c(mom_pid[spouse_idx], pop_pid[spouse_idx])
    })
    if (kin_by_sex) {
      res$motherinlaw <- lapply(spouse_rows, function(x) {
        spouse_idx <- x[!is.na(x)]
        if (!length(spouse_idx)) {
          return(vector(mode = "numeric", length = 0L))
        }
        mom_pid[spouse_idx]
      })
      res$fatherinlaw <- lapply(spouse_rows, function(x) {
        spouse_idx <- x[!is.na(x)]
        if (!length(spouse_idx)) {
          return(vector(mode = "numeric", length = 0L))
        }
        pop_pid[spouse_idx]
      })
    }

    spouseofsiblings <- so(siblings)
    parentsinlaw_rel <- if (use_row_lookup) parentsinlaw_rows else res$parentsinlaw
    siblingsofspouse <- ko(parentsinlaw_rel)
    siblingsofspouse <- lapply(seq_along(siblingsofspouse), function(i) {
      siblingsofspouse[[i]][siblingsofspouse[[i]] %ni% spouse_rel[[i]]]
    })
    spouseofsiblingsofspouse <- so(siblingsofspouse)
    siblingsinlaw <- mapply(c, spouseofsiblings, siblingsofspouse, spouseofsiblingsofspouse, SIMPLIFY = FALSE)
    res$siblingsinlaw <- convert_relatives(siblingsinlaw)
    if (kin_by_sex) {
      res$sistersinlaw <- convert_relatives(subset_by_sex(siblingsinlaw, 1))
      res$brothersinlaw <- convert_relatives(subset_by_sex(siblingsinlaw, 0))
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
