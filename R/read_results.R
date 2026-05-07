#' Read output marriage file into a data frame
#' 
#' \tabular{rll}{
#'  1   \tab mid \tab Marriage id number (unique sequential integer) \cr
#'  2   \tab wpid \tab Wife’s person id \cr
#'  3   \tab hpid \tab Husband’s person id \cr
#'  4   \tab dstart \tab Date marriage began \cr
#'  5   \tab dend \tab Date marriage ended or zero if still in force at end of simulation \cr
#'  6   \tab rend \tab Reason marriage ended 2 = divorce; 3 = death of one partner \cr
#'  7   \tab wprior \tab Marriage id of wife’s next most recent prior marriage \cr
#'  8   \tab hprior \tab Marriage id of husband’s next most recent prior marriage \cr
#' }
#' 
#' you can either provide the complete `path` to the file or the `folder`, supfilename, seed and suffix with which you
#' started the simulation
#' 
#' @param folder simulation base folder ("~/socsim/simulation_235/")
#' @param supfile name of supervisory-file ("socsim.sup")
#' @param seed random number seed (42)
#' @param suffix optional suffix for the results-directory (default="")
#' @param fn complete path to the file. If not provided, it will be created from the other arguments
#' @param quiet logical. If `FALSE`, emit a message with the file path being read.
#'
#' @return Data frame with the information of the output marriage file
#'
#' @md
#' 
#' @export
read_omar <- function(folder = NULL, supfile = "socsim.sup", seed = 42, suffix = "", fn = NULL, quiet = FALSE) {
  if (is.null(fn)) {
    fn <- socsim_result_file(folder = folder,
                             supfile = supfile,
                             seed = seed,
                             suffix = suffix,
                             filename = "result.omar")
    if (!file.exists(fn)) {
      legacy_fn <- socsim_legacy_result_file(folder = folder,
                                             supfile = supfile,
                                             seed = seed,
                                             suffix = suffix,
                                             filename = "result.omar")
      if (file.exists(legacy_fn)) {
        fn <- legacy_fn
      }
    }
  }

  if (!quiet) {
    message("read marriage file: ", fn)
  }

  omar_names <- c("mid", "wpid", "hpid", "dstart", "dend", "rend", "wprior", "hprior")
  file_size <- if (file.exists(fn)) file.info(fn)$size else NA_real_

  if (!file.exists(fn) || is.na(file_size) || file_size == 0) {
    warning("marriage file missing or empty; returning empty data frame", call. = FALSE)
    return(data.frame(
      mid = integer(),
      wpid = integer(),
      hpid = integer(),
      dstart = integer(),
      dend = integer(),
      rend = integer(),
      wprior = integer(),
      hprior = integer()
    ))
  }

  utils::read.table(
    file = fn,
    header = FALSE,
    as.is = TRUE,
    col.names = omar_names,
    colClasses = rep("integer", length(omar_names))
  )
}

#' Read output population file into a data frame
#' 
#' after the end of the simulation, socsim writes every person of the simulation into
#' a file called result.opop                                                              |
#' 
#' \tabular{rll}{
#' 1   \tab pid \tab Person id unique identifier assigned as integer in birth order \cr
#' 2   \tab fem \tab 1 if female 0 if male \cr
#' 3   \tab group \tab Group identifier 1..60 current group membership of individual \cr
#' 4   \tab nev \tab Next scheduled event \cr
#' 5   \tab dob \tab Date of birth integer month number \cr
#' 6   \tab mom \tab Person id of mother \cr
#' 7   \tab pop \tab Person id of father \cr
#' 8   \tab nesibm \tab Person id of next eldest sibling through mother \cr
#' 9   \tab nesibp \tab Person id of next eldest sibling through father \cr
#' 10  \tab lborn \tab Person id of last born child \cr
#' 11  \tab marid \tab Id of marriage in .omar file \cr
#' 12  \tab mstat \tab Marital status at end of simulation integer 1=single;2=divorced; 3=widowed; 4=married \cr
#' 13  \tab dod \tab Date of death or 0 if alive at end of simulation \cr
#' 14  \tab fmult \tab Fertility multiplier \cr
#' }
#'
#' This table explains the columns of the opop file and the columns of the output data frame.
#' You can either provide the complete `path` to the file or the `folder`, supfilename, seed and suffix with which you
#' started the simulation
#' 
#' @param folder simulation base folder ("~/socsim/simulation_235/")
#' @param supfile name of supervisory-file ("socsim.sup")
#' @param seed random number seed (42)
#' @param suffix optional suffix for the results-directory (default="")
#' @param fn complete path to the file. If not provided, it will be created from the other arguments
#' @param quiet logical. If `FALSE`, emit a message with the file path being read.
#'
#' @return Data frame with the information of the output population file
#' 
#' @md
#' 
#' @export
read_opop <- function(folder = NULL, supfile = "socsim.sup", seed = 42, suffix = "", fn = NULL, quiet = FALSE) {
  if (is.null(fn)) {
    fn <- socsim_result_file(folder = folder,
                             supfile = supfile,
                             seed = seed,
                             suffix = suffix,
                             filename = "result.opop")
    if (!file.exists(fn)) {
      legacy_fn <- socsim_legacy_result_file(folder = folder,
                                             supfile = supfile,
                                             seed = seed,
                                             suffix = suffix,
                                             filename = "result.opop")
      if (file.exists(legacy_fn)) {
        fn <- legacy_fn
      }
    }
  }

  if (!quiet) {
    message("read population file: ", fn)
  }

  opop_names <- c("pid", "fem", "group", "nev", "dob", "mom", "pop", "nesibm", "nesibp",
                  "lborn", "marid", "mstat", "dod", "fmult")
  file_size <- if (file.exists(fn)) file.info(fn)$size else NA_real_

  if (!file.exists(fn) || is.na(file_size) || file_size == 0) {
    warning("population file missing or empty; returning empty data frame", call. = FALSE)
    return(data.frame(
      pid = integer(),
      fem = integer(),
      group = integer(),
      nev = integer(),
      dob = integer(),
      mom = integer(),
      pop = integer(),
      nesibm = integer(),
      nesibp = integer(),
      lborn = integer(),
      marid = integer(),
      mstat = integer(),
      dod = integer(),
      fmult = numeric()
    ))
  }

  utils::read.table(
    file = fn,
    header = FALSE,
    as.is = TRUE,
    col.names = opop_names,
    colClasses = c(rep("integer", length(opop_names) - 1L), "numeric")
  )
}
