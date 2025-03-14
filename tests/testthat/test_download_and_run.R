library(testthat)
library(rsocsim)

test_that("Download rates and run simulation", {
  dir.create("tmp/tmp", recursive = TRUE)
  tmpdir <- file.path(getwd(), "tmp/tmp")
  
  expect_true(dir.exists(tmpdir))
  
  rsocsim::download_rates(tmpdir, "MM", yearStart = 1997, yearEnd = 2010, source = "UN")
  
  # Assuming the download_rates function creates a specific file, check if it exists
  expect_true(file.exists(file.path(tmpdir, "rates", "MM_rates_info.sup")))
  
  rsocsim::create_initial_population(tmpdir, size_opop = 1000, output_base = "presim")

  result <- rsocsim::socsim(tmpdir, "MM_rates_info.sup")
  print(paste("Result:", result))
  # Add assertions based on expected results from the simulation
  expect_is(result, 1)
})