library(testthat)
library(rsocsim)

# Helper: compare numeric vectors with tolerances
expect_within_tolerance <- function(current, baseline, abs_tol = NULL, rel_tol = NULL) {
  abs_tol_env <- suppressWarnings(as.numeric(Sys.getenv("RSOCSIM_RATES_ABS_TOL", "0.06")))
  rel_tol_env <- suppressWarnings(as.numeric(Sys.getenv("RSOCSIM_RATES_REL_TOL", "0.6")))
  abs_tol <- if (is.null(abs_tol) || is.na(abs_tol)) abs_tol_env else abs_tol
  rel_tol <- if (is.null(rel_tol) || is.na(rel_tol)) rel_tol_env else rel_tol
  stopifnot(length(current) == length(baseline))
  diff <- abs(current - baseline)
  allowed <- pmax(abs_tol, rel_tol * abs(baseline))
  expect_true(all(diff <= allowed),
    info = paste0("Differences exceeded tolerance. max diff=", max(diff),
      "; max allowed=", max(allowed))
  )
}

regression_results_dir <- function() {
  results_dir <- testthat::test_path("_results")
  if (!dir.exists(results_dir)) {
    dir.create(results_dir, recursive = TRUE)
  }
  results_dir
}

mirror_test_artifact <- function(path, results_dir = regression_results_dir()) {
  mirror_enabled <- tolower(Sys.getenv("RSOCSIM_MIRROR_TEST_ARTIFACTS", "")) %in% c("1", "true", "yes")
  if (!mirror_enabled || !file.exists(path)) {
    return(invisible(NULL))
  }

  if (!dir.exists(results_dir)) {
    dir.create(results_dir, recursive = TRUE)
  }

  file.copy(path, file.path(results_dir, basename(path)), overwrite = TRUE)
  invisible(path)
}

read_or_create_baseline <- function(current_rates, baseline_path) {
  if (!file.exists(baseline_path)) {
    utils::write.csv(current_rates, baseline_path, row.names = FALSE)
  }

  utils::read.csv(baseline_path)
}

reference_initial_population_size <- function() {
  init_src <- system.file("extdata", "init_new.opop", package = "rsocsim", mustWork = TRUE)
  nrow(read_opop(fn = init_src, quiet = TRUE))
}

write_deterministic_initial_population <- function(folder, output_base, size_opop, rng_seed) {
  set.seed(rng_seed)
  create_initial_population(folder = folder,
                            size_opop = as.integer(size_opop),
                            output_base = output_base)
}

summarize_simulated_rates <- function(opop, final_sim_year, year_start, year_end) {
  asfr <- estimate_fertility_rates(
    opop = opop,
    final_sim_year = final_sim_year,
    year_min = year_start,
    year_max = year_end,
    year_group = 1,
    age_min_fert = 15,
    age_max_fert = 50,
    age_group = 5
  )

  asmr <- estimate_mortality_rates(
    opop = opop,
    final_sim_year = final_sim_year,
    year_min = year_start,
    year_max = year_end,
    year_group = 1,
    age_max_mort = 100,
    age_group = 5
  )

  fert_summary <- aggregate(socsim ~ year, asfr, mean, na.rm = TRUE)
  mort_summary <- aggregate(socsim ~ year, asmr, mean, na.rm = TRUE)

  rates <- merge(fert_summary, mort_summary, by = "year", suffixes = c("_fert", "_mort"))
  year_numeric <- as.character(rates$year)
  year_numeric <- as.numeric(sub("^\\[?([0-9]+).*", "\\1", year_numeric))
  rates$month <- (year_numeric - min(year_numeric, na.rm = TRUE)) * 12
  rates <- rates[order(rates$month), c("month", "socsim_fert", "socsim_mort")]
  names(rates) <- c("month", "rate_fertility", "rate_mortality")
  rates
}

plot_rates_comparison <- function(rates, baseline, plot_path, plot_title, results_dir) {
  grDevices::png(plot_path, width = 900, height = 600)
  on.exit({
    grDevices::dev.off()
    mirror_test_artifact(plot_path, results_dir)
  }, add = TRUE)

  y_values <- c(rates$rate_fertility, rates$rate_mortality)
  if (nrow(baseline) > 0) {
    y_values <- c(y_values, baseline$rate_fertility, baseline$rate_mortality)
  }
  y_values <- y_values[is.finite(y_values)]
  y_max <- if (length(y_values) > 0) {
    stats::quantile(y_values, probs = 0.995, names = FALSE, na.rm = TRUE)
  } else {
    1
  }
  y_max <- max(y_max, 0.1)

  plot(rates$month, rates$rate_fertility, type = "l",
    col = "#2C7FB8", lwd = 2,
    xlab = "Month", ylab = "Rate",
    main = plot_title,
    ylim = c(0, y_max)
  )
  lines(rates$month, rates$rate_mortality, col = "#D95F02", lwd = 2)

  if (nrow(baseline) > 0) {
    lines(rates$month, baseline$rate_fertility, col = "#2C7FB8", lwd = 2, lty = 2)
    lines(rates$month, baseline$rate_mortality, col = "#D95F02", lwd = 2, lty = 2)
    legend("topright",
      legend = c("Fertility (current)", "Mortality (current)",
        "Fertility (baseline)", "Mortality (baseline)"),
      col = c("#2C7FB8", "#D95F02", "#2C7FB8", "#D95F02"),
      lwd = 2, lty = c(1, 1, 2, 2), bty = "n"
    )
  } else {
    legend("topright", legend = c("Fertility", "Mortality"),
      col = c("#2C7FB8", "#D95F02"), lwd = 2, bty = "n"
    )
  }
}

test_that("download_rates regression: simulate, estimate, compare, plot", {
  testthat::skip_on_cran()
  if (Sys.getenv("RSOCSIM_RUN_INTEGRATION_TESTS") != "1") {
    testthat::skip(paste(
      "Integration test disabled.",
      "Enable by setting RSOCSIM_RUN_INTEGRATION_TESTS=1.",
      "In PowerShell: $Env:RSOCSIM_RUN_INTEGRATION_TESTS=\"1\"",
      "In R: Sys.setenv(RSOCSIM_RUN_INTEGRATION_TESTS=\"1\")"
    ))
  }

  if (!exists("startSocsimWithFile", where = asNamespace("rsocsim"), mode = "function")) {
    testthat::skip("Compiled backend not available.")
  }

  country <- Sys.getenv("RSOCSIM_RATES_COUNTRY", "SE")
  year_start <- suppressWarnings(as.integer(Sys.getenv("RSOCSIM_RATES_YEAR_START", "1970")))
  year_end <- suppressWarnings(as.integer(Sys.getenv("RSOCSIM_RATES_YEAR_END", "2030")))
  presim_months <- suppressWarnings(as.integer(Sys.getenv("RSOCSIM_RATES_PRESIM_MONTHS", "600")))
  source <- Sys.getenv("RSOCSIM_RATES_SOURCE", "UN")
  api_url <- Sys.getenv(
    "RSOCSIM_RATES_API_URL",
    "https://user.demogr.mpg.de/theile/socsimratesAPI/v1/"
  )

  if (!is.character(country) || nchar(country) != 2) {
    testthat::skip("Invalid RSOCSIM_RATES_COUNTRY; must be 2-letter ISO code.")
  }
  if (is.na(year_start) || is.na(year_end) || year_start > year_end) {
    testthat::skip("Invalid RSOCSIM_RATES_YEAR_START/END; must be numeric with start <= end.")
  }
  if (is.na(presim_months) || presim_months <= 0) {
    testthat::skip("Invalid RSOCSIM_RATES_PRESIM_MONTHS; must be positive integer.")
  }

  simdir <- file.path(tempdir(), "rsocsim-download-rates-regression")
  dir.create(simdir, showWarnings = FALSE, recursive = TRUE)
  print(paste(c("simdir: ", simdir)))

  download_result <- tryCatch(
    download_rates(
      folder = simdir,
      countrycode = country,
      yearStart = year_start,
      yearEnd = year_end,
      source = source,
      apiUrl = api_url,
      presim_duration_months = presim_months
    ),
    error = function(e) {
      testthat::skip(paste("download_rates failed:", e$message))
    }
  )

  write_deterministic_initial_population(folder = simdir,
                                         output_base = "presim",
                                         size_opop = 2L * reference_initial_population_size(),
                                         rng_seed = 20260508L)

  seed_env <- Sys.getenv("RSOCSIM_RATES_SEED", "20260508")
  seed <- if (identical(seed_env, "random")) "20260508" else seed_env
  suffix <- paste0("download_rates_", tolower(country), "_", year_start, "_", year_end)

  result <- socsim(
    simdir,
    basename(download_result$infoFile),
    seed = seed,
    process_method = "inprocess",
    suffix = suffix
  )
  expect_equal(result, 1)

  output_dir <- file.path(
    simdir,
    paste0("sim_results_", basename(download_result$infoFile), "_", seed, "_", suffix)
  )
  opop_path <- file.path(output_dir, "result.opop")
  omar_path <- file.path(output_dir, "result.omar")

  expect_true(file.exists(opop_path))
  expect_true(file.exists(omar_path))

  opop <- read_opop(fn = opop_path)
  omar <- read_omar(fn = omar_path)

  expect_true(nrow(opop) > 0)
  expect_true(ncol(omar) == 8)

  rates <- summarize_simulated_rates(opop = opop,
                                     final_sim_year = year_end,
                                     year_start = year_start,
                                     year_end = year_end)

  expect_true(all(rates$rate_fertility >= 0, na.rm = TRUE))
  expect_true(all(rates$rate_mortality >= 0, na.rm = TRUE))

  artifacts_dir <- file.path(tempdir(), "rsocsim-test-artifacts", "download-rates")
  if (!dir.exists(artifacts_dir)) {
    dir.create(artifacts_dir, recursive = TRUE)
  }
  baseline_dir <- regression_results_dir()

  date_tag <- format(Sys.Date(), "%Y%m%d")
  current_path <- file.path(
    artifacts_dir,
    sprintf("download_rates_current_%s_%s_%d_%d_seed_%s.csv", date_tag, country, year_start, year_end, seed)
  )
  baseline_path <- file.path(
    baseline_dir,
    sprintf("download_rates_baseline_%s_%d_%d.csv", country, year_start, year_end)
  )
  plot_path <- file.path(
    artifacts_dir,
    sprintf("download_rates_plot_%s_%s_%d_%d_seed_%s.png", date_tag, country, year_start, year_end, seed)
  )

  utils::write.csv(rates, current_path, row.names = FALSE)
  mirror_test_artifact(current_path, baseline_dir)

  baseline <- read_or_create_baseline(rates, baseline_path)
  expect_equal(baseline$month, rates$month)

  expect_within_tolerance(rates$rate_fertility, baseline$rate_fertility)
  expect_within_tolerance(rates$rate_mortality, baseline$rate_mortality)

  plot_rates_comparison(rates = rates,
                        baseline = baseline,
                        plot_path = plot_path,
                        plot_title = sprintf("Simulated rates (%s %d-%d)", country, year_start, year_end),
                        results_dir = baseline_dir)
})

test_that("download_rates parallel regression: USA five seeds combined, compare, plot", {
  testthat::skip_on_cran()
  if (Sys.getenv("RSOCSIM_RUN_INTEGRATION_TESTS") != "1") {
    testthat::skip(paste(
      "Integration test disabled.",
      "Enable by setting RSOCSIM_RUN_INTEGRATION_TESTS=1.",
      "In PowerShell: $Env:RSOCSIM_RUN_INTEGRATION_TESTS=\"1\"",
      "In R: Sys.setenv(RSOCSIM_RUN_INTEGRATION_TESTS=\"1\")"
    ))
  }
  if (!requireNamespace("future", quietly = TRUE)) {
    testthat::skip("The 'future' package is not available.")
  }
  if (!exists("startSocsimWithFile", where = asNamespace("rsocsim"), mode = "function")) {
    testthat::skip("Compiled backend not available.")
  }

  country <- "US"
  year_start <- suppressWarnings(as.integer(Sys.getenv("RSOCSIM_RATES_YEAR_START", "1970")))
  year_end <- suppressWarnings(as.integer(Sys.getenv("RSOCSIM_RATES_YEAR_END", "2030")))
  presim_months <- suppressWarnings(as.integer(Sys.getenv("RSOCSIM_RATES_PRESIM_MONTHS", "600")))
  source <- Sys.getenv("RSOCSIM_RATES_SOURCE", "UN")
  api_url <- Sys.getenv(
    "RSOCSIM_RATES_API_URL",
    "https://user.demogr.mpg.de/theile/socsimratesAPI/v1/"
  )

  if (is.na(year_start) || is.na(year_end) || year_start > year_end) {
    testthat::skip("Invalid RSOCSIM_RATES_YEAR_START/END; must be numeric with start <= end.")
  }
  if (is.na(presim_months) || presim_months <= 0) {
    testthat::skip("Invalid RSOCSIM_RATES_PRESIM_MONTHS; must be positive integer.")
  }

  simdir <- file.path(tempdir(), "rsocsim-download-rates-us-parallel-regression")
  dir.create(simdir, showWarnings = FALSE, recursive = TRUE)
  print(paste(c("simdir: ", simdir)))

  download_result <- tryCatch(
    download_rates(
      folder = simdir,
      countrycode = country,
      yearStart = year_start,
      yearEnd = year_end,
      source = source,
      apiUrl = api_url,
      presim_duration_months = presim_months
    ),
    error = function(e) {
      testthat::skip(paste("download_rates failed:", e$message))
    }
  )

  small_population_size <- max(5000L, reference_initial_population_size() %/% 2L)
  write_deterministic_initial_population(folder = simdir,
                                         output_base = "presim",
                                         size_opop = small_population_size,
                                         rng_seed = 20260509L)

  seeds <- c("20260511", "20260512", "20260513", "20260514", "20260515")
  supfile_name <- basename(download_result$infoFile)
  suffix <- paste0("download_rates_parallel_", tolower(country), "_", year_start, "_", year_end)

  result <- socsim_parallel(
    folder = simdir,
    supfile = supfile_name,
    seed = seeds,
    suffix = suffix,
    backend = "future",
    workers = min(5L, future::availableCores())
  )

  expect_equal(nrow(result), length(seeds))
  expect_true(all(result$status == "success"))
  expect_true(all(dir.exists(result$output_dir)))

  individual_rows <- vapply(seeds, function(current_seed) {
    nrow(read_opop(folder = simdir,
                   supfile = supfile_name,
                   seed = current_seed,
                   suffix = suffix,
                   quiet = TRUE))
  }, integer(1))
  opop <- read_opop(folder = simdir,
                    supfile = supfile_name,
                    seed = seeds,
                    suffix = suffix,
                    quiet = TRUE)

  expect_true(all(individual_rows > 0L))
  expect_equal(nrow(opop), sum(individual_rows))

  rates <- summarize_simulated_rates(opop = opop,
                                     final_sim_year = year_end,
                                     year_start = year_start,
                                     year_end = year_end)

  expect_true(all(rates$rate_fertility >= 0, na.rm = TRUE))
  expect_true(all(rates$rate_mortality >= 0, na.rm = TRUE))

  artifacts_dir <- file.path(tempdir(), "rsocsim-test-artifacts", "download-rates-parallel")
  if (!dir.exists(artifacts_dir)) {
    dir.create(artifacts_dir, recursive = TRUE)
  }
  baseline_dir <- regression_results_dir()

  date_tag <- format(Sys.Date(), "%Y%m%d")
  current_path <- file.path(
    artifacts_dir,
    sprintf("download_rates_parallel_current_%s_%s_%d_%d_seeds_%d.csv", date_tag, country, year_start, year_end, length(seeds))
  )
  baseline_path <- file.path(
    baseline_dir,
    sprintf("download_rates_parallel_baseline_%s_%d_%d.csv", country, year_start, year_end)
  )
  plot_path <- file.path(
    artifacts_dir,
    sprintf("download_rates_parallel_plot_%s_%s_%d_%d_seeds_%d.png", date_tag, country, year_start, year_end, length(seeds))
  )

  utils::write.csv(rates, current_path, row.names = FALSE)
  mirror_test_artifact(current_path, baseline_dir)

  baseline <- read_or_create_baseline(rates, baseline_path)
  expect_equal(baseline$month, rates$month)

  expect_within_tolerance(rates$rate_fertility, baseline$rate_fertility)
  expect_within_tolerance(rates$rate_mortality, baseline$rate_mortality)

  plot_rates_comparison(rates = rates,
                        baseline = baseline,
                        plot_path = plot_path,
                        plot_title = sprintf("Parallel simulated rates (%s %d-%d)", country, year_start, year_end),
                        results_dir = baseline_dir)
})