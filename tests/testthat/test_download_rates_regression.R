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

  init_src <- system.file("extdata", "init_new.opop", package = "rsocsim", mustWork = TRUE)
  file.copy(init_src, file.path(simdir, "presim.opop"), overwrite = TRUE)
  file.create(file.path(simdir, "presim.omar"))

  seed_env <- Sys.getenv("RSOCSIM_RATES_SEED", "random")
  seed <- if (seed_env == "random") {
    as.character(sample.int(1e9, 1))
  } else {
    seed_env
  }
  suffix <- paste0("download_rates_", tolower(country), "_", year_start, "_", year_end)

  result <- socsim(
    simdir,
    basename(download_result$infoFile),
    seed = seed,
    process_method = "inprocess",
    suffix = suffix
  )
  expect_equal(result, 1)

  output_dir <- file.path(simdir, paste0("sim_results_", seed, "_", suffix))
  opop_path <- file.path(output_dir, "result.opop")
  omar_path <- file.path(output_dir, "result.omar")

  expect_true(file.exists(opop_path))
  expect_true(file.exists(omar_path))

  opop <- read_opop(fn = opop_path)
  omar <- read_omar(fn = omar_path)

  expect_true(nrow(opop) > 0)
  expect_true(ncol(omar) == 8)

  asfr <- estimate_fertility_rates(
    opop = opop,
    final_sim_year = year_end,
    year_min = year_start,
    year_max = year_end,
    year_group = 1,
    age_min_fert = 15,
    age_max_fert = 50,
    age_group = 5
  )

  asmr <- estimate_mortality_rates(
    opop = opop,
    final_sim_year = year_end,
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

  expect_true(all(rates$rate_fertility >= 0, na.rm = TRUE))
  expect_true(all(rates$rate_mortality >= 0, na.rm = TRUE))

  artifacts_dir <- file.path(tempdir(), "rsocsim-test-artifacts", "download-rates")
  if (!dir.exists(artifacts_dir)) {
    dir.create(artifacts_dir, recursive = TRUE)
  }
  baseline_dir <- file.path("tests", "testthat", "_results")

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

  if (!file.exists(baseline_path)) {
    proposed_baseline_path <- file.path(artifacts_dir, basename(baseline_path))
    utils::write.csv(rates, proposed_baseline_path, row.names = FALSE)
    testthat::skip(paste0(
      "Baseline rates CSV missing. Wrote a proposed baseline to ",
      proposed_baseline_path,
      ". Re-run the test after adding a checked-in baseline if needed."
    ))
  }

  baseline <- utils::read.csv(baseline_path)
  expect_equal(baseline$month, rates$month)

  expect_within_tolerance(rates$rate_fertility, baseline$rate_fertility)
  expect_within_tolerance(rates$rate_mortality, baseline$rate_mortality)

  grDevices::png(plot_path, width = 900, height = 600)
  on.exit(grDevices::dev.off(), add = TRUE)
  y_values <- c(rates$rate_fertility, rates$rate_mortality)
  if (exists("baseline") && nrow(baseline) > 0) {
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
    main = sprintf("Simulated rates (%s %d-%d)", country, year_start, year_end),
    ylim = c(0, y_max)
  )
  lines(rates$month, rates$rate_mortality, col = "#D95F02", lwd = 2)

  if (exists("baseline") && nrow(baseline) > 0) {
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
})