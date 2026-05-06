#' Build the directory name used for simulation outputs
#'
#' @param supfile A string. The supervisory file name.
#' @param seed A string. The RNG seed.
#' @param suffix A string. Optional suffix for the result directory.
#' @return A string. The result directory name relative to the simulation folder.
#' @noRd
socsim_result_dirname <- function(supfile, seed, suffix = "") {
  paste0("sim_results_", basename(supfile), "_", as.character(seed), "_", suffix)
}

#' Build the full path to a simulation output directory
#'
#' @param folder A string. Simulation folder.
#' @param supfile A string. The supervisory file name.
#' @param seed A string. The RNG seed.
#' @param suffix A string. Optional suffix for the result directory.
#' @return A string. The full output directory path.
#' @noRd
socsim_result_dir <- function(folder, supfile, seed, suffix = "") {
  file.path(folder, socsim_result_dirname(supfile = supfile, seed = seed, suffix = suffix))
}

#' Build the full path to a simulation output file
#'
#' @param folder A string. Simulation folder.
#' @param supfile A string. The supervisory file name.
#' @param seed A string. The RNG seed.
#' @param suffix A string. Optional suffix for the result directory.
#' @param filename A string. Output file name inside the result directory.
#' @return A string. The full output file path.
#' @noRd
socsim_result_file <- function(folder, supfile, seed, suffix = "", filename) {
  file.path(socsim_result_dir(folder = folder,
                              supfile = supfile,
                              seed = seed,
                              suffix = suffix),
            filename)
}

#' Build the legacy output directory name used by earlier package versions
#'
#' @param supfile A string. The supervisory file name.
#' @param seed A string. The RNG seed.
#' @param suffix A string. Optional suffix for the result directory.
#' @return A string. The legacy result directory name relative to the
#'   simulation folder.
#' @noRd
socsim_legacy_result_dirname <- function(supfile, seed, suffix = "") {
  paste0("sim_results_", as.character(seed), "_", suffix)
}

#' Build the legacy full path to a simulation output file
#'
#' @param folder A string. Simulation folder.
#' @param supfile A string. The supervisory file name.
#' @param seed A string. The RNG seed.
#' @param suffix A string. Optional suffix for the result directory.
#' @param filename A string. Output file name inside the result directory.
#' @return A string. The legacy full output file path.
#' @noRd
socsim_legacy_result_file <- function(folder, supfile, seed, suffix = "", filename) {
  file.path(folder,
            socsim_legacy_result_dirname(supfile = supfile, seed = seed, suffix = suffix),
            filename)
}

#' Normalize simulation jobs for parallel execution
#'
#' @param folder A character vector of simulation folders.
#' @param supfile A character vector of supervisory file names.
#' @param seed A character vector of RNG seeds.
#' @param compatibility_mode A character vector of compatibility modes.
#' @param suffix A character vector of result suffixes.
#' @return A data frame with one row per simulation job.
#' @noRd
normalize_socsim_parallel_jobs <- function(folder,
                                           supfile,
                                           seed,
                                           compatibility_mode,
                                           suffix) {
  lengths <- c(length(folder),
               length(supfile),
               length(seed),
               length(compatibility_mode),
               length(suffix))
  target_length <- max(lengths)
  if (!all(lengths %in% c(1L, target_length))) {
    stop("Arguments 'folder', 'supfile', 'seed', 'compatibility_mode', and 'suffix' must all have length 1 or the same length.")
  }
  if (target_length < 1L) {
    stop("At least one simulation job must be supplied.")
  }

  jobs <- data.frame(
    folder = rep_len(as.character(folder), target_length),
    supfile = rep_len(as.character(supfile), target_length),
    seed = rep_len(as.character(seed), target_length),
    compatibility_mode = rep_len(as.character(compatibility_mode), target_length),
    suffix = rep_len(as.character(suffix), target_length),
    stringsAsFactors = FALSE
  )

  if (anyNA(jobs$folder) || anyNA(jobs$supfile) || anyNA(jobs$seed) ||
      anyNA(jobs$compatibility_mode) || anyNA(jobs$suffix)) {
    stop("Parallel simulation jobs cannot contain missing values.")
  }

  jobs$output_dir <- socsim_result_dir(folder = jobs$folder,
                                       supfile = jobs$supfile,
                                       seed = jobs$seed,
                                       suffix = jobs$suffix)
  collision_keys <- paste(jobs$folder,
                          basename(jobs$supfile),
                          jobs$seed,
                          jobs$suffix,
                          sep = "\r")
  if (anyDuplicated(collision_keys)) {
    stop("Parallel simulation jobs would write to the same output directory. Ensure each job has a unique combination of folder, supfile basename, seed, and suffix.")
  }

  jobs
}

#' Run several SOCSIM simulations
#'
#' @param folder A character vector. Simulation folders. Scalars are recycled.
#' @param supfile A character vector. Supervisory files. Scalars are recycled.
#' @param seed A character vector. RNG seeds. Scalars are recycled.
#' @param compatibility_mode A character vector. Compatibility mode values.
#'   Scalars are recycled.
#' @param suffix A character vector. Optional result directory suffixes. Scalars
#'   are recycled.
#' @param backend A string. Use "future" to run jobs in parallel worker
#'   processes or "sequential" to run jobs one after another.
#' @param workers An integer. Number of workers to use with
#'   `backend = "future"`.
#' @return A data frame with one row per job and columns describing the input,
#'   output directory, status, and any error message.
#' @export
socsim_parallel <- function(folder,
                            supfile,
                            seed = "42",
                            compatibility_mode = "1",
                            suffix = "",
                            backend = c("future", "sequential"),
                            workers = NULL) {
  backend <- match.arg(backend)
  jobs <- normalize_socsim_parallel_jobs(folder = folder,
                                         supfile = supfile,
                                         seed = seed,
                                         compatibility_mode = compatibility_mode,
                                         suffix = suffix)

  run_job <- function(job) {
    result <- tryCatch(
      expr = {
        value <- socsim(folder = job$folder,
                        supfile = job$supfile,
                        seed = job$seed,
                        process_method = "inprocess",
                        compatibility_mode = job$compatibility_mode,
                        suffix = job$suffix)
        list(status = "success",
             result = value,
             error_message = NA_character_)
      },
      error = function(e) {
        list(status = "error",
             result = NA_integer_,
             error_message = conditionMessage(e))
      }
    )

    c(as.list(job), result)
  }

  job_rows <- split(jobs, seq_len(nrow(jobs)))
  if (backend == "future") {
    if (!requireNamespace("future", quietly = TRUE)) {
      stop("The 'future' package is required for backend = 'future'.")
    }
    if (is.null(workers)) {
      workers <- future::availableCores()
    }
    old_plan <- future::plan(future::multisession, workers = workers)
    on.exit(future::plan(old_plan), add = TRUE)
    futures <- lapply(job_rows, function(job) {
      future::future(run_job(job), seed = TRUE)
    })
    results <- lapply(futures, future::value)
  } else {
    results <- lapply(job_rows, run_job)
  }

  results_df <- do.call(rbind, lapply(results, function(result) {
    data.frame(folder = result$folder,
               supfile = result$supfile,
               seed = result$seed,
               compatibility_mode = result$compatibility_mode,
               suffix = result$suffix,
               output_dir = result$output_dir,
               status = result$status,
               result = as.integer(result$result),
               error_message = if (is.null(result$error_message)) NA_character_ else result$error_message,
               stringsAsFactors = FALSE)
  }))
  rownames(results_df) <- NULL
  results_df
}