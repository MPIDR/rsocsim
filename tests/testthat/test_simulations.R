library(testthat)
library(rsocsim)

test_that("create_simulation_folder creates directories", {
  base <- file.path(tempdir(), "rsocsim-tests")
  dir.create(base, showWarnings = FALSE, recursive = TRUE)

  simdir <- create_simulation_folder(basedir = base, simdir = "sim1")
  expect_true(dir.exists(simdir))
  expect_true(grepl("sim1", simdir, fixed = TRUE))
})

test_that("create_sup_file writes a valid .sup file", {
  simdir <- file.path(tempdir(), "rsocsim-sup")
  dir.create(simdir, showWarnings = FALSE, recursive = TRUE)

  supfile <- create_sup_file(simdir = simdir, simname = "demo")
  sup_path <- file.path(simdir, supfile)

  expect_true(file.exists(sup_path))

  content <- readLines(sup_path)
  expect_true(any(grepl("^segments\\s+1", content)))
  expect_true(any(grepl("^input_file", content)))
  expect_true(any(grepl("^run", content)))
})

test_that("simulation_time_to_years converts months to years", {
  expect_equal(simulation_time_to_years(120L, 0L, 2000L), 2010)
  expect_equal(simulation_time_to_years(132L, 12L, 2000L), 2010)
})