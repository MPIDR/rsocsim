library(testthat)
library(rsocsim)

test_that("Download rates and run simulation", {
  dir.create("tmp/tmp", recursive = TRUE)
  tmpdir <- file.path(getwd(), "tmp/tmp")
  
  expect_true(dir.exists(tmpdir))
  
  rsocsim::download_rates(tmpdir, "MM", yearStart = 1997, yearEnd = 2010, source = "UN")
  
  # Assuming the download_rates function creates a specific file, check if it exists
  expect_true(file.exists(file.path(tmpdir, "MM_rates_info.sup")))
  
  result <- rsocsim::socsim(tmpdir, "MM_rates_info.sup")
  
  # Add assertions based on expected results from the simulation
  expect_is(result, "data.frame") # Example assertion, adjust based on actual expected output
})