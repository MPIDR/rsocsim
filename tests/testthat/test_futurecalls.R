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









