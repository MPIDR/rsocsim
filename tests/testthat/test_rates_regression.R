library(testthat)
library(rsocsim)

# Helper: compare numeric vectors with tolerances
expect_within_tolerance <- function(current, baseline, abs_tol = 0.05, rel_tol = 0.5) {
  stopifnot(length(current) == length(baseline))
  diff <- abs(current - baseline)
  allowed <- pmax(abs_tol, rel_tol * abs(baseline))
  expect_true(all(diff <= allowed),
              info = paste0("Differences exceeded tolerance. max diff=", max(diff),
                            "; max allowed=", max(allowed)))
}

test_that("rates regression: simulate, estimate, compare, plot", {
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

  simdir <- file.path(tempdir(), "rsocsim-rates-regression")
  dir.create(simdir, showWarnings = FALSE, recursive = TRUE)
  print(paste(c("simdir: ",simdir)))

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
    "duration 480",
    "include SWEfert2022",
    "include SWEmort2022",
    "run"
  )
  sup_path <- file.path(simdir, "socsim.sup")
  writeLines(sup_content, sup_path)

  seed_env <- Sys.getenv("RSOCSIM_RATES_SEED", "random")
  seed <- if (seed_env == "random") {
    as.character(sample.int(1e9, 1))
  } else {
    seed_env
  }
  suffix <- "rates"
  result <- socsim(simdir, "socsim.sup", seed = seed, process_method = "inprocess", suffix = suffix)
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

  # Estimate yearly fertility and mortality rates
  asfr <- estimate_fertility_rates(
    opop = opop,
    final_sim_year = 2010,
    year_min = 1960,
    year_max = 2010,
    year_group = 1,
    age_min_fert = 15,
    age_max_fert = 50,
    age_group = 5
  )

  asmr <- estimate_mortality_rates(
    opop = opop,
    final_sim_year = 2010,
    year_min = 1960,
    year_max = 2010,
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

  results_dir <- file.path("tests", "testthat", "_results")
  if (!dir.exists(results_dir)) {
    dir.create(results_dir, recursive = TRUE)
  }

  date_tag <- format(Sys.Date(), "%Y%m%d")
  current_path <- file.path(results_dir, sprintf("rates_current_%s_seed_%s.csv", date_tag, seed))
  baseline_override <- Sys.getenv("RSOCSIM_RATES_BASELINE", "")
  if (nzchar(baseline_override)) {
    baseline_path <- baseline_override
  } else {
    baseline_candidates <- list.files(
      results_dir,
      pattern = "^rates_baseline_\\d{8}_seed_.*\\.csv$",
      full.names = TRUE
    )
    baseline_path <- if (length(baseline_candidates) > 0) {
      sort(baseline_candidates, decreasing = TRUE)[1]
    } else {
      file.path(results_dir, sprintf("rates_baseline_%s_seed_%s.csv", date_tag, seed))
    }
  }
  plot_path <- file.path(results_dir, sprintf("rates_plot_%s_seed_%s.png", date_tag, seed))

  utils::write.csv(rates, current_path, row.names = FALSE)

  if (!file.exists(baseline_path)) {
    utils::write.csv(rates, baseline_path, row.names = FALSE)
    testthat::skip(paste0(
      "Baseline rates CSV missing. Created ", basename(baseline_path),
      ". Re-run the test to compare against the baseline."
    ))
  }

  baseline <- utils::read.csv(baseline_path)
  expect_equal(baseline$month, rates$month)

  expect_within_tolerance(rates$rate_fertility, baseline$rate_fertility)
  expect_within_tolerance(rates$rate_mortality, baseline$rate_mortality)

    # Plot and save
    grDevices::png(plot_path, width = 900, height = 600)
    on.exit(grDevices::dev.off(), add = TRUE)
    plot(rates$month, rates$rate_fertility, type = "l",
      col = "#2C7FB8", lwd = 2,
      xlab = "Month", ylab = "Rate",
      main = "Simulated rates (fertility vs mortality)")
    lines(rates$month, rates$rate_mortality, col = "#D95F02", lwd = 2)
    if (exists("baseline") && nrow(baseline) > 0) {
      lines(rates$month, baseline$rate_fertility, col = "#2C7FB8", lwd = 2, lty = 2)
      lines(rates$month, baseline$rate_mortality, col = "#D95F02", lwd = 2, lty = 2)
      legend("topright",
       legend = c("Fertility (current)", "Mortality (current)",
            "Fertility (baseline)", "Mortality (baseline)"),
       col = c("#2C7FB8", "#D95F02", "#2C7FB8", "#D95F02"),
       lwd = 2, lty = c(1, 1, 2, 2), bty = "n")
    } else {
      legend("topright", legend = c("Fertility", "Mortality"),
       col = c("#2C7FB8", "#D95F02"), lwd = 2, bty = "n")
    }
})
