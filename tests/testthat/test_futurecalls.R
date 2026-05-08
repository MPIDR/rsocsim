library(testthat)
library(rsocsim)

test_that("socsim runs a short simulation (smoke test)", {
  testthat::skip_on_cran()
  if (Sys.getenv("RSOCSIM_RUN_INTEGRATION_TESTS") != "1") {
    testthat::skip("Integration test disabled. Set RSOCSIM_RUN_INTEGRATION_TESTS=1 to enable.")
  }

  if (!exists("startSocsimWithFile", where = asNamespace("rsocsim"), mode = "function")) {
    testthat::skip("Compiled backend not available.")
  }

  simdir <- file.path(tempdir(), "rsocsim-integration")
  dir.create(simdir, showWarnings = FALSE, recursive = TRUE)

  fert_src <- system.file("extdata", "SWEfert2022", package = "rsocsim", mustWork = TRUE)
  mort_src <- system.file("extdata", "SWEmort2022", package = "rsocsim", mustWork = TRUE)
  init_src <- system.file("extdata", "init_new.opop", package = "rsocsim", mustWork = TRUE)

  file.copy(fert_src, file.path(simdir, "SWEfert2022"), overwrite = TRUE)
  file.copy(mort_src, file.path(simdir, "SWEmort2022"), overwrite = TRUE)
  file.copy(init_src, file.path(simdir, "init_new.opop"), overwrite = TRUE)
  file.create(file.path(simdir, "init_new.omar"))

  sup_content <- c(
    "marriage_queues 1",
    "bint 10",
    "segments 1",
    "marriage_eval distribution",
    "marriage_after_childbirth 1",
    "input_file init_new",
    "duration 12",
    "include SWEfert2022",
    "include SWEmort2022",
    "run"
  )
  sup_path <- file.path(simdir, "socsim.sup")
  writeLines(sup_content, sup_path)

  result <- socsim(simdir, "socsim.sup", seed = "1", process_method = "inprocess", suffix = "test")
  expect_equal(result, 1)

  output_dir <- file.path(simdir, "sim_results_socsim.sup_1_test")
  expect_true(dir.exists(output_dir))
})

test_that("readers fall back to legacy result directory naming", {
  simdir <- file.path(tempdir(), "rsocsim-readers-legacy")
  dir.create(simdir, showWarnings = FALSE, recursive = TRUE)

  legacy_dir <- file.path(simdir, "sim_results_7_legacy")
  dir.create(legacy_dir, showWarnings = FALSE, recursive = TRUE)

  writeLines("1 1 1 1 0 0 0 0", file.path(legacy_dir, "result.omar"))
  writeLines("1 1 1 1 0 0 0 0 0 0 0 1 0 1", file.path(legacy_dir, "result.opop"))

  omar <- read_omar(folder = simdir, supfile = "socsim.sup", seed = 7, suffix = "legacy")
  opop <- read_opop(folder = simdir, supfile = "socsim.sup", seed = 7, suffix = "legacy")

  expect_equal(nrow(omar), 1)
  expect_equal(nrow(opop), 1)
})

test_that("socsim_parallel runs multiple short simulations", {
  testthat::skip_on_cran()
  if (Sys.getenv("RSOCSIM_RUN_INTEGRATION_TESTS") != "1") {
    testthat::skip("Integration test disabled. Set RSOCSIM_RUN_INTEGRATION_TESTS=1 to enable.")
  }
  if (!requireNamespace("future", quietly = TRUE)) {
    testthat::skip("The 'future' package is not available.")
  }
  if (!exists("startSocsimWithFile", where = asNamespace("rsocsim"), mode = "function")) {
    testthat::skip("Compiled backend not available.")
  }

  simdir <- file.path(tempdir(), "rsocsim-parallel-integration")
  dir.create(simdir, showWarnings = FALSE, recursive = TRUE)

  fert_src <- system.file("extdata", "SWEfert2022", package = "rsocsim", mustWork = TRUE)
  mort_src <- system.file("extdata", "SWEmort2022", package = "rsocsim", mustWork = TRUE)
  init_src <- system.file("extdata", "init_new.opop", package = "rsocsim", mustWork = TRUE)

  file.copy(fert_src, file.path(simdir, "SWEfert2022"), overwrite = TRUE)
  file.copy(mort_src, file.path(simdir, "SWEmort2022"), overwrite = TRUE)
  file.copy(init_src, file.path(simdir, "init_new.opop"), overwrite = TRUE)
  file.create(file.path(simdir, "init_new.omar"))

  sup_content <- c(
    "marriage_queues 1",
    "bint 10",
    "segments 1",
    "marriage_eval distribution",
    "marriage_after_childbirth 1",
    "input_file init_new",
    "duration 12",
    "include SWEfert2022",
    "include SWEmort2022",
    "run"
  )
  writeLines(sup_content, file.path(simdir, "socsim.sup"))

  result <- socsim_parallel(folder = simdir,
                            supfile = "socsim.sup",
                            seed = c("11", "12"),
                            suffix = c("parallel_a", "parallel_b"),
                            backend = "future",
                            workers = 2)

  expect_equal(nrow(result), 2)
  expect_true(all(result$status == "success"))
  expect_true(all(dir.exists(result$output_dir)))
})

test_that("socsim_parallel with three seeds can be read into one opop data frame", {
  testthat::skip_on_cran()
  if (Sys.getenv("RSOCSIM_RUN_INTEGRATION_TESTS") != "1") {
    testthat::skip("Integration test disabled. Set RSOCSIM_RUN_INTEGRATION_TESTS=1 to enable.")
  }
  if (!requireNamespace("future", quietly = TRUE)) {
    testthat::skip("The 'future' package is not available.")
  }
  if (!exists("startSocsimWithFile", where = asNamespace("rsocsim"), mode = "function")) {
    testthat::skip("Compiled backend not available.")
  }

  simdir <- file.path(tempdir(), "rsocsim-parallel-read-opop-integration")
  dir.create(simdir, showWarnings = FALSE, recursive = TRUE)

  fert_src <- system.file("extdata", "SWEfert2022", package = "rsocsim", mustWork = TRUE)
  mort_src <- system.file("extdata", "SWEmort2022", package = "rsocsim", mustWork = TRUE)
  init_src <- system.file("extdata", "init_new.opop", package = "rsocsim", mustWork = TRUE)

  file.copy(fert_src, file.path(simdir, "SWEfert2022"), overwrite = TRUE)
  file.copy(mort_src, file.path(simdir, "SWEmort2022"), overwrite = TRUE)
  file.copy(init_src, file.path(simdir, "init_new.opop"), overwrite = TRUE)
  file.create(file.path(simdir, "init_new.omar"))

  sup_content <- c(
    "marriage_queues 1",
    "bint 10",
    "segments 1",
    "marriage_eval distribution",
    "marriage_after_childbirth 1",
    "input_file init_new",
    "duration 12",
    "include SWEfert2022",
    "include SWEmort2022",
    "run"
  )
  writeLines(sup_content, file.path(simdir, "socsim.sup"))

  seeds <- c("21", "22", "23")
  suffix <- "parallel_read_opop"
  id_offset <- 10000000L

  result <- socsim_parallel(folder = simdir,
                            supfile = "socsim.sup",
                            seed = seeds,
                            suffix = suffix,
                            backend = "future",
                            workers = min(3L, future::availableCores()))

  expect_equal(nrow(result), 3)
  expect_true(all(result$status == "success"))
  expect_true(all(dir.exists(result$output_dir)))

  individual <- lapply(seeds, function(current_seed) {
    read_opop(folder = simdir,
              supfile = "socsim.sup",
              seed = current_seed,
              suffix = suffix,
              quiet = TRUE)
  })
  combined <- read_opop(folder = simdir,
                        supfile = "socsim.sup",
                        seed = seeds,
                        suffix = suffix,
                        id_offset = id_offset,
                        quiet = TRUE)

  chunk_sizes <- vapply(individual, nrow, integer(1))
  expect_true(all(chunk_sizes > 0L))
  expect_equal(nrow(combined), sum(chunk_sizes))

  id_columns <- c("pid", "mom", "pop", "nesibm", "nesibp", "lborn", "marid")
  row_ends <- cumsum(chunk_sizes)
  row_starts <- row_ends - chunk_sizes + 1L

  for (i in seq_along(individual)) {
    expected_chunk <- individual[[i]]
    offset <- (i - 1L) * id_offset
    if (offset > 0L) {
      for (column in id_columns) {
        positive_idx <- !is.na(expected_chunk[[column]]) & expected_chunk[[column]] > 0L
        expected_chunk[[column]][positive_idx] <- as.integer(expected_chunk[[column]][positive_idx] + offset)
      }
    }

    actual_chunk <- combined[row_starts[i]:row_ends[i], , drop = FALSE]
    rownames(actual_chunk) <- NULL
    rownames(expected_chunk) <- NULL

    expect_equal(actual_chunk,
                 expected_chunk,
                 info = paste("Combined opop chunk mismatch for seed", seeds[i]))
  }
})









