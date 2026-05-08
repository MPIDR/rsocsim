#' Read output marriage file into a data frame
#'
#' When `fn` contains multiple file paths, or when `seed` contains multiple
#' values and `fn` is `NULL`, the matching result files are read and row-bound
#' into a single data frame. To keep identifiers unique across simulations,
#' positive ID columns are offset by `(index - 1) * id_offset`, while sentinel
#' zeros remain unchanged.
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
#' @param id_offset positive integer stride used to offset IDs when combining
#'   multiple files. Ignored for single-file reads. Default is 10 million, which allows combining up
#'   to 214 files with a total population of 10 million each.
#' @param quiet logical. If `FALSE`, emit a message with the file path being read.
#'
#' @return A data frame with columns `mid`, `wpid`, `hpid`, `dstart`, `dend`,
#'   `rend`, `wprior`, and `hprior`, matching the SOCSIM `result.omar` file.
#'   If the file is missing or empty, a zero-row data frame with these columns
#'   is returned.
#'
#' @md
#' 
#' @export
read_omar <- function(folder = NULL, supfile = "socsim.sup", seed = 42, suffix = "", fn = NULL, id_offset = 10000000L, quiet = FALSE) {
  if (read_results_has_multiple_inputs(fn) || (is.null(fn) && read_results_has_multiple_inputs(seed))) {
    jobs <- read_results_normalize_inputs(folder = folder,
                                          supfile = supfile,
                                          seed = seed,
                                          suffix = suffix,
                                          fn = fn)
    read_results_validate_batch_size(nrow(jobs), id_offset)
    out <- lapply(seq_len(nrow(jobs)), function(i) {
      row <- jobs[i, , drop = FALSE]
      data <- read_omar_single(folder = row$folder,
                               supfile = row$supfile,
                               seed = row$seed,
                               suffix = row$suffix,
                               fn = if (is.na(row$fn)) NULL else row$fn,
                               quiet = quiet)
      read_results_apply_offset(data,
                                id_columns = c("mid", "wpid", "hpid", "wprior", "hprior"),
                                offset = (i - 1L) * as.integer(id_offset),
                                id_offset = id_offset,
                                source_label = row$label)
    })
    return(do.call(rbind, out))
  }

  read_omar_single(folder = folder,
                   supfile = supfile,
                   seed = seed,
                   suffix = suffix,
                   fn = fn,
                   quiet = quiet)
}

#' Read output population file into a data frame
#'
#' When `fn` contains multiple file paths, or when `seed` contains multiple
#' values and `fn` is `NULL`, the matching result files are read and row-bound
#' into a single data frame. To keep identifiers unique across simulations,
#' positive ID columns are offset by `(index - 1) * id_offset`, while sentinel
#' zeros remain unchanged.
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
#' @param id_offset positive integer stride used to offset IDs when combining
#'   multiple files. Ignored for single-file reads. Default is 10 million, which allows combining up
#'   to 214 files with a total population of 10 million each.
#' @param quiet logical. If `FALSE`, emit a message with the file path being read.
#'
#' @return A data frame with columns `pid`, `fem`, `group`, `nev`, `dob`,
#'   `mom`, `pop`, `nesibm`, `nesibp`, `lborn`, `marid`, `mstat`, `dod`, and
#'   `fmult`, matching the SOCSIM `result.opop` file. If the file is missing
#'   or empty, a zero-row data frame with these columns is returned.
#' 
#' @md
#' 
#' @export
read_opop <- function(folder = NULL, supfile = "socsim.sup", seed = 42, suffix = "", fn = NULL, id_offset = 10000000L, quiet = FALSE) {
  if (read_results_has_multiple_inputs(fn) || (is.null(fn) && read_results_has_multiple_inputs(seed))) {
    jobs <- read_results_normalize_inputs(folder = folder,
                                          supfile = supfile,
                                          seed = seed,
                                          suffix = suffix,
                                          fn = fn)
    read_results_validate_batch_size(nrow(jobs), id_offset)
    out <- lapply(seq_len(nrow(jobs)), function(i) {
      row <- jobs[i, , drop = FALSE]
      data <- read_opop_single(folder = row$folder,
                               supfile = row$supfile,
                               seed = row$seed,
                               suffix = row$suffix,
                               fn = if (is.na(row$fn)) NULL else row$fn,
                               quiet = quiet)
      read_results_apply_offset(data,
                                id_columns = c("pid", "mom", "pop", "nesibm", "nesibp", "lborn", "marid"),
                                offset = (i - 1L) * as.integer(id_offset),
                                id_offset = id_offset,
                                source_label = row$label)
    })
    return(do.call(rbind, out))
  }

  read_opop_single(folder = folder,
                   supfile = supfile,
                   seed = seed,
                   suffix = suffix,
                   fn = fn,
                   quiet = quiet)
}

read_results_has_multiple_inputs <- function(x) {
  if (is.null(x)) {
    return(FALSE)
  }

  length(read_results_flatten_input(x)) > 1L
}

read_results_flatten_input <- function(x) {
  if (is.null(x)) {
    return(NULL)
  }

  unlist(x, use.names = FALSE)
}

read_results_validate_batch_size <- function(n_inputs, id_offset) {
  if (!is.numeric(id_offset) || length(id_offset) != 1L || is.na(id_offset) || id_offset <= 0 || id_offset != trunc(id_offset)) {
    stop("Argument 'id_offset' must be one positive integer value.", call. = FALSE)
  }

  max_files <- floor(.Machine$integer.max / as.double(id_offset))
  if (n_inputs > max_files) {
    warning(sprintf("Combining %s files with id_offset=%s can overflow integer IDs; lower 'id_offset' and try again.",
                    n_inputs,
                    format(id_offset, scientific = FALSE)),
            call. = FALSE)
    stop("ID offset configuration would overflow integer IDs.", call. = FALSE)
  }
}

read_results_normalize_inputs <- function(folder, supfile, seed, suffix, fn) {
  if (!is.null(fn)) {
    fn_values <- as.character(read_results_flatten_input(fn))
    return(data.frame(folder = rep(NA_character_, length(fn_values)),
                      supfile = rep(NA_character_, length(fn_values)),
                      seed = rep(NA_character_, length(fn_values)),
                      suffix = rep(NA_character_, length(fn_values)),
                      fn = fn_values,
                      label = fn_values,
                      stringsAsFactors = FALSE))
  }

  seed_values <- as.character(read_results_flatten_input(seed))
  target_length <- length(seed_values)
  recycle_arg <- function(x, name) {
    values <- read_results_flatten_input(x)
    if (is.null(values)) {
      values <- NA_character_
    }
    if (!length(values) %in% c(1L, target_length)) {
      stop("Arguments 'folder', 'supfile', 'seed', and 'suffix' must all have length 1 or the same length.", call. = FALSE)
    }
    rep_len(as.character(values), target_length)
  }

  data.frame(folder = recycle_arg(folder, "folder"),
             supfile = recycle_arg(supfile, "supfile"),
             seed = seed_values,
             suffix = recycle_arg(suffix, "suffix"),
             fn = rep(NA_character_, target_length),
             label = paste0("seed=", seed_values),
             stringsAsFactors = FALSE)
}

read_results_apply_offset <- function(data, id_columns, offset, id_offset, source_label) {
  if (!nrow(data)) {
    return(data)
  }

  max_id <- 0L
  for (column in id_columns) {
    values <- data[[column]]
    positive_values <- values[!is.na(values) & values > 0L]
    if (length(positive_values)) {
      max_id <- max(max_id, max(positive_values))
    }
  }

  if (max_id > id_offset) {
    stop(sprintf("Cannot combine '%s' because IDs exceed id_offset=%s. Increase 'id_offset' to at least the maximum positive ID in each file.",
                 source_label,
                 format(id_offset, scientific = FALSE)),
         call. = FALSE)
  }

  if (offset > 0L && max_id > (.Machine$integer.max - offset)) {
    stop(sprintf("Cannot combine '%s' because applying the configured id_offset would overflow integer IDs.",
                 source_label),
         call. = FALSE)
  }

  if (offset == 0L) {
    return(data)
  }

  for (column in id_columns) {
    values <- data[[column]]
    positive_idx <- !is.na(values) & values > 0L
    values[positive_idx] <- as.integer(values[positive_idx] + offset)
    data[[column]] <- values
  }

  data
}

read_results_resolve_file <- function(folder, supfile, seed, suffix, filename) {
  fn <- socsim_result_file(folder = folder,
                           supfile = supfile,
                           seed = seed,
                           suffix = suffix,
                           filename = filename)
  if (!file.exists(fn)) {
    legacy_fn <- socsim_legacy_result_file(folder = folder,
                                           supfile = supfile,
                                           seed = seed,
                                           suffix = suffix,
                                           filename = filename)
    if (file.exists(legacy_fn)) {
      fn <- legacy_fn
    }
  }

  fn
}

read_omar_empty <- function() {
  data.frame(
    mid = integer(),
    wpid = integer(),
    hpid = integer(),
    dstart = integer(),
    dend = integer(),
    rend = integer(),
    wprior = integer(),
    hprior = integer()
  )
}

read_omar_single <- function(folder, supfile, seed, suffix, fn, quiet) {
  if (is.null(fn)) {
    fn <- read_results_resolve_file(folder = folder,
                                    supfile = supfile,
                                    seed = seed,
                                    suffix = suffix,
                                    filename = "result.omar")
  }

  if (!quiet) {
    message("read marriage file: ", fn)
  }

  omar_names <- c("mid", "wpid", "hpid", "dstart", "dend", "rend", "wprior", "hprior")
  file_size <- if (file.exists(fn)) file.info(fn)$size else NA_real_

  if (!file.exists(fn) || is.na(file_size) || file_size == 0) {
    warning("marriage file missing or empty; returning empty data frame", call. = FALSE)
    return(read_omar_empty())
  }

  utils::read.table(
    file = fn,
    header = FALSE,
    as.is = TRUE,
    col.names = omar_names,
    colClasses = rep("integer", length(omar_names))
  )
}

read_opop_empty <- function() {
  data.frame(
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
  )
}

read_opop_single <- function(folder, supfile, seed, suffix, fn, quiet) {
  if (is.null(fn)) {
    fn <- read_results_resolve_file(folder = folder,
                                    supfile = supfile,
                                    seed = seed,
                                    suffix = suffix,
                                    filename = "result.opop")
  }

  if (!quiet) {
    message("read population file: ", fn)
  }

  opop_names <- c("pid", "fem", "group", "nev", "dob", "mom", "pop", "nesibm", "nesibp",
                  "lborn", "marid", "mstat", "dod", "fmult")
  file_size <- if (file.exists(fn)) file.info(fn)$size else NA_real_

  if (!file.exists(fn) || is.na(file_size) || file_size == 0) {
    warning("population file missing or empty; returning empty data frame", call. = FALSE)
    return(read_opop_empty())
  }

  utils::read.table(
    file = fn,
    header = FALSE,
    as.is = TRUE,
    col.names = opop_names,
    colClasses = c(rep("integer", length(opop_names) - 1L), "numeric")
  )
}
