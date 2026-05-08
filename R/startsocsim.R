#' Run a single Socsim simulation with a given supervisory file and directory
#'
#' @param folder A string. This is the base directory of the simulation. Every
#'   .sup and rate file should be named relative to this directory. 
#' @param supfile A string. The name of the .sup file to start the simulation,
#'   relative to the directory.
#' @param seed A string. The seed for the RNG, so expects an integer. Defaults
#'   to "42".
#' @param process_method A string. Whether and how SOCSIM should be started in
#'   its own process or in the running R process. Defaults to "inprocess". Use
#'   one of:
#'    * "future" - the safest option. A new process will be started via the
#'      "future" package
#'    * "inprocess" - SOCSIM runs in the R-process. Beware if you run several
#'      different simulations, they may affect later simulations.
#'    * "clustercall" - if the future package is not available, try this method
#'      instead.
#' @param compatibility_mode A string.
#' @param suffix A string.
#' @return Returns `1L` when the simulation finishes successfully. If the
#'   simulation errors before completion, the function returns `NULL` after
#'   issuing warnings. Result files are written to the directory
#'   `sim_results_<basename(supfile)>_<seed>_<suffix>` inside `folder`.
#' @export
socsim <- function(folder, supfile, seed = "42", process_method = "inprocess",
                   compatibility_mode = "1", suffix = "") {
  seed = as.character(seed)
  compatibility_mode = as.character(compatibility_mode)
  input_supfile_path <- socsim_input_supfile_path(folder, supfile)
  socsim_message("Starting SOCSIM simulation.")
  socsim_message("Base directory: ", folder)
  socsim_message("Input supervisory file: ", input_supfile_path)
  socsim_message("RNG seed: ", seed)
  previous_wd = getwd()
  result = NULL
  # If 'supfile' contains more than a basename, startSocsimWithFile() will
  # crash. This is only a workaround.
  remove_supfile <- FALSE
  if (!identical(basename(supfile), supfile)) {
    if (identical(dirname(supfile), "folder")) {
      supfile <- basename(supfile)
    } else {
      warning("The argument 'supfile' contained more than a basename. We copy the .sup file to the 'folder' directory (to avoid startSocsimWithFile() from crashing) and will remove it after the simulation finishes.")
      file.copy(from = file.path(folder, supfile), to = folder, overwrite = TRUE)
      remove_supfile <- TRUE
    }
  }
  tryCatch(expr = {
    setwd(folder)
    result = run_sim_w_file(supfile = supfile,
                            seed = seed,
                            compatibility_mode = compatibility_mode,
                            suffix = suffix,
                            method = process_method)
    if (!is.null(result) && identical(as.integer(result), 1L)) {
      report_socsim_console_summary(folder = ".",
                                    supfile = supfile,
                                    seed = seed,
                                    suffix = suffix)
    }
  },
  error = function(w){
    warning("Error during execution of simulation!")
    warning(w)
  },
  finally = {
    if (remove_supfile) {
      unlink(file.path(folder, basename(supfile)))
    }
    socsim_message("Restoring working directory: ", previous_wd)
    setwd(previous_wd)
  }
  )
  return(result)
  
}

socsim_env_flag <- function(name) {
  value <- Sys.getenv(name, "")
  if (!nzchar(value)) {
    return(NULL)
  }

  tolower(value) %in% c("1", "true", "yes")
}

socsim_console_enabled <- function() {
  env_override <- socsim_env_flag("RSOCSIM_TEST_CONSOLE_OUTPUT")
  if (!is.null(env_override)) {
    return(env_override)
  }

  isTRUE(getOption("rsocsim.console_output", TRUE))
}

socsim_message <- function(...) {
  if (socsim_console_enabled()) {
    message(...)
  }
}

socsim_is_absolute_path <- function(path) {
  grepl("^[A-Za-z]:[/\\\\]|^[/\\\\]{2}|^/", path)
}

socsim_input_supfile_path <- function(folder, supfile) {
  if (socsim_is_absolute_path(supfile)) {
    return(normalizePath(supfile, winslash = "/", mustWork = FALSE))
  }

  normalizePath(file.path(folder, supfile), winslash = "/", mustWork = FALSE)
}

report_socsim_console_summary <- function(folder, supfile, seed, suffix = "") {
  output_dir <- normalizePath(
    socsim_result_dir(folder = folder,
                      supfile = supfile,
                      seed = seed,
                      suffix = suffix),
    winslash = "/",
    mustWork = FALSE
  )
  pyramid_file <- socsim_result_file(folder = folder,
                                     supfile = supfile,
                                     seed = seed,
                                     suffix = suffix,
                                     filename = "result.pyr")

  socsim_message("Output directory: ", output_dir)
  if (!file.exists(pyramid_file)) {
    return(invisible(output_dir))
  }

  pyramid_lines <- extract_console_population_pyramid(pyramid_file)
  if (length(pyramid_lines) > 0L) {
    socsim_message("Population pyramid:")
    writeLines(pyramid_lines)
  }

  invisible(output_dir)
}

extract_console_population_pyramid <- function(pyramid_file) {
  pyramid_lines <- readLines(pyramid_file, warn = FALSE)
  if (!length(pyramid_lines)) {
    return(character())
  }

  all_groups_idx <- grep("All groups :", pyramid_lines, fixed = TRUE)
  if (!length(all_groups_idx)) {
    return(character())
  }

  header_candidates <- grep("Population Pyramid", pyramid_lines, fixed = TRUE)
  header_candidates <- header_candidates[header_candidates < utils::tail(all_groups_idx, 1L)]
  if (!length(header_candidates)) {
    return(character())
  }

  start_idx <- utils::tail(header_candidates, 1L)
  scale_idx <- grep("^   ------\\+", pyramid_lines)
  scale_candidates <- scale_idx[scale_idx > utils::tail(all_groups_idx, 1L)]
  if (!length(scale_candidates)) {
    return(character())
  }

  scale_start_idx <- scale_candidates[1L]
  scale_end_idx <- min(scale_start_idx + 1L, length(pyramid_lines))
  block <- pyramid_lines[start_idx:scale_end_idx]

  if (length(block) < 6L) {
    return(block)
  }

  header_part <- block[1:3]
  age_rows <- block[4:(length(block) - 2L)]
  scale_part <- block[(length(block) - 1L):length(block)]

  if (length(age_rows) > 1L) {
    keep_idx <- seq(1L, length(age_rows), by = 2L)
    if (utils::tail(keep_idx, 1L) != length(age_rows)) {
      keep_idx <- c(keep_idx, length(age_rows))
    }
    age_rows <- age_rows[unique(keep_idx)]
  }

  c(header_part, age_rows, scale_part)
}

#' Run a single Socsim simulation.
#'
#' @details
#' See [socsim()] for documentation of the arguments.
#'
#' @inheritParams socsim
#' @param method Internal alias for `process_method`.
#' @noRd
run_sim_w_file <- function(supfile, seed = "42", compatibility_mode = "1",
                           suffix = "", method = "inprocess") {
    outfn <- socsim_result_file(folder = ".",
                  supfile = supfile,
                  seed = seed,
                  suffix = suffix,
                  filename = "logfile.log")
  socsim_message("Progress can be monitored in the logfile: ",
                 normalizePath(outfn, winslash = "/", mustWork = FALSE))
  tail_log <- isTRUE(getOption("rsocsim.tail_log", FALSE))

    if (method == "inprocess") {
      socsim_message("Running SOCSIM in the current R process.")
        startSocsimWithFile(supfile = supfile,
                            seed = seed,
                            compatibility_mode = compatibility_mode,
                            result_suffix = suffix)
        return(1)
    } else if (method == "future") {
    if (!requireNamespace("future", quietly = TRUE)) {
      stop("The 'future' package is required for process_method = 'future'.")
    }
        socsim_message("Running SOCSIM in a separate R process via future::multisession.")
        old_plan <- future::plan()
        on.exit(future::plan(old_plan), add = TRUE)
        future::plan(future::multisession, workers = 1)
        f1 <- future::future({
            startSocsimWithFile(supfile, seed, compatibility_mode, result_suffix = suffix)
        }, seed = TRUE)
        socsim_message("Simulation started.")
        if (tail_log) {
          socsim_message("Live tailing logfile: ", outfn)
        }
        wait_for_future_simulation(f1, outfn, tail_log = tail_log)
        future::value(f1)
        socsim_message("Simulation finished.")
        return(1)
    } else if (method == "clustercall") {
        if (!requireNamespace("parallel", quietly = TRUE)) {
          stop("The 'parallel' package is required for process_method = 'clustercall'.")
        }
        socsim_message("Running SOCSIM in a separate R process via parallel::clusterCall.")
        worker_outfn <- paste0(outfn, ".worker")
        cl <- parallel::makeCluster(spec = 1, type = "PSOCK", outfile = worker_outfn)
        on.exit(parallel::stopCluster(cl), add = TRUE)
        parallel::clusterExport(cl, "startSocsimWithFile")
        parallel::clusterCall(cl,
                              startSocsimWithFile,
                              supfile = supfile,
                              seed = seed,
                              compatibility_mode = compatibility_mode,
                              result_suffix = suffix)
        if (tail_log) {
          print_last_line_of_logfile(outfn)
        }
        socsim_message("Simulation finished.")
        return(1)
    } else {
        stop("No valid process_method argument given.")
    }
}

wait_for_future_simulation <- function(future_result, logfilename, tail_log = TRUE) {
  lastline <- ""
  while (!future::resolved(future_result)) {
    Sys.sleep(1)
    if (tail_log) {
      lastline <- print_last_line_of_logfile(logfilename, lastline)
    }
  }

  if (tail_log) {
    print_last_line_of_logfile(logfilename, lastline)
  }
}

print_last_line_of_logfile = function(logfilename, lastline = "") {
  if (!file.exists(logfilename)) {
    return(lastline)
  }

  tryCatch({
    con = file(logfilename, "r")
    on.exit(close(con), add = TRUE)
    line2 <- lastline
    while ( TRUE ) {
      line = readLines(con, n = 1)
      if ( length(line) == 0 ) {
        if (!identical(lastline, line2) && nzchar(line2)) {
          message(line2)
        }
        break
      }
      line2 <- line
    }
    return(line2)
  }, error = function(e) {
    warning("Error while reading logfile '", logfilename, "': ", conditionMessage(e))
    return(lastline)
  })
}


# Internal helper for development workflows that rely on an external SOCSIM
# executable.
run1simulationwithfile_from_binary <- function(folder, supfile,seed="42",compatibility_mode="1",socsim_path=NULL) {
  if (is.null(socsim_path)) {
    stop("Please supply 'socsim_path' explicitly. Downloading executables is not supported by this package build.")
  }
  socsim_path <- normalizePath(socsim_path, mustWork = TRUE)
  if (dir.exists(socsim_path)) {
    stop("'socsim_path' must point to an executable file, not a directory.")
  }
  seed = toString(seed)
  print("Start run1simulationwithfile")
  print(folder)
  print(supfile)
  print(paste0("socsim_path: ", socsim_path))
  print(seed)
  previous_wd = getwd()
  setwd(paste0(folder))
  
  print(paste0("command: ",socsim_path,args=paste0(" ",supfile," ", seed," ", compatibility_mode)))
  
  print(system2(socsim_path, args = c(supfile, seed, compatibility_mode)))
  print(previous_wd)
  setwd(previous_wd)
  return(1)
}

#' Create a directory structure for the simulation
#'
#' Create a two-level directory structure. If the first-level argument is NULL,
#' we look for and, if needed, create the directory 'socsim' in the current
#' temporary directory. If the second-level argument is NULL, we create a directory
#' named like 'socsim_sim_' followed by a random component in the first-level
#' directory.
#'
#' @param basedir A string. Optional. First-level directory where the
#'   simulation-specific directory will be created. Defaults to
#'   `file.path(tempdir(), "socsim")`.
#' @param simdir A string. Optional. Simulation-specific directory which will
#'   be created within 'basedir'. Defaults to 'socsim_sim_' plus a random
#'   component created with [tempfile()].
#' @return A string. The full path to the simulation-specific directory.
#' @export
create_simulation_folder <- function(basedir = NULL, simdir = NULL) {
  # If no 'basedir' is given, we default to a subdirectory of tempdir().
  if (is.null(basedir)) { basedir <- file.path(tempdir(), "socsim") }
    # If 'basedir' does not exist, create it.
  if (!dir.exists(basedir)) { dir.create(basedir, recursive = TRUE) }
    if (is.null(simdir)) {
        # If no 'simdir' is given, create a random name that starts with
        # 'socsim_sim_'.
        subdir = tempfile(pattern = "socsim_sim_", tmpdir = basedir)
    } else {
        subdir = file.path(basedir, simdir)
    }
    if (!dir.exists(subdir)) { dir.create(subdir) }
    return(subdir)
}

#' Create a basic .sup file for a simulation
#' 
#' The simulation is only a simple one. The file will be saved into the
#' directory 'simdir'.
#' 
#' @param simdir A string. The directory where the .sup file will be saved.
#' @param simname A string. The base name of the simulation. Defaults to
#'   `"socsim"`.
#' @return A string. The basename of the created supervisory file, for example
#'   `"socsim.sup"`. The file is written to `simdir`, and the function also
#'   copies the bundled rate and initial-population input files into `simdir`.
#' @export
create_sup_file <- function(simdir, simname = "socsim") {
  sup_content <- "
*Supervisory file for a stable population
* 20220120
marriage_queues 1
bint 10
segments 1
marriage_eval distribution
marriage_after_childbirth 1
input_file init_new
*
duration 1200
include SWEfert2022
include SWEmort2022
run
"
  sup_fn <- paste0(simname, ".sup")
  cat(sup_content, file = file.path(simdir, sup_fn))
  fn_SWEfert2022_source <- system.file("extdata", "SWEfert2022",
				       package = "rsocsim", mustWork = TRUE)
  fn_SWEfert2022_dest <- file.path(simdir, "SWEfert2022")
  fn_SWEmort2022_source <- system.file("extdata", "SWEmort2022",
				       package = "rsocsim", mustWork = TRUE)
  fn_SWEmort2022_dest <- file.path(simdir, "SWEmort2022")
  fn_init_source <- system.file("extdata", "init_new.opop",
				package = "rsocsim", mustWork = TRUE)
  fn_init_dest <- file.path(simdir, "init_new.opop")
  file.copy(fn_SWEfert2022_source, fn_SWEfert2022_dest)
  file.copy(fn_SWEmort2022_source, fn_SWEmort2022_dest)
  file.copy(fn_init_source, fn_init_dest)
  return(sup_fn)
}

#' Read the content of the supervisory file 
#'
#' @param simdir A string. Base directory of the simulation.
#' @param sup_fn A string. File name of the .sup file.
#' @return A character vector with one element per line of the supervisory
#'   file.
#' @export
get_supervisory_content <- function(simdir, sup_fn) {
  if (is.null(sup_fn)) {
    sup_fn <- "socsim.sup"
  }
  sup_content <- readLines(file.path(simdir, sup_fn))
  return(sup_content)
}

#' Calculate for how many years the simulation ran
#'
#' @param simulation_time An integer. The number of periods (months) the
#'   simulation ran.
#' @param pre_simulation_time An integer. The number of periods (months) the
#'   simulation ran before getting to a stable population. This is subtracted
#'   from 'simulation_time' in order to arrive at the "real" simulation time
#' @param start_year An integer. The year the simulation started.
#' @return A numeric scalar giving the calendar year reached at the end of the
#'   simulated period after subtracting `pre_simulation_time / 12`. The value
#'   can include a fractional year.
#' @export
simulation_time_to_years <- function(simulation_time, pre_simulation_time, start_year) {
    stopifnot(all(is.integer(simulation_time), is.integer(pre_simulation_time), is.integer(start_year)))
    return(start_year + (simulation_time - pre_simulation_time)/12)
}
