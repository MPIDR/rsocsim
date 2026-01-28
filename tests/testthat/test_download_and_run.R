library(testthat)
library(rsocsim)

test_that("download_rates validates inputs", {
  tmpdir <- tempdir()

  expect_error(
    download_rates("nonexistent_dir", "SE"),
    "does not exist"
  )

  expect_error(
    download_rates(tmpdir, "SWE"),
    "2-character"
  )

  expect_error(
    download_rates(tmpdir, "SE", yearStart = 2020, yearEnd = 2010),
    "yearStart"
  )
})