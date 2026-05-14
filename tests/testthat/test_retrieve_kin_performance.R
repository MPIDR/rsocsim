perf_env_flag <- function(name) {
  value <- Sys.getenv(name, "")
  if (!nzchar(value)) {
    return(FALSE)
  }

  tolower(value) %in% c("1", "true", "yes")
}

perf_env_integer <- function(name, default) {
  value <- Sys.getenv(name, "")
  if (!nzchar(value)) {
    return(default)
  }

  parsed <- suppressWarnings(as.integer(value))
  if (is.na(parsed) || parsed < 1L) {
    default
  } else {
    parsed
  }
}

perf_env_integer_vector <- function(name, default) {
  value <- Sys.getenv(name, "")
  if (!nzchar(value)) {
    return(default)
  }

  pieces <- trimws(strsplit(value, ",", fixed = TRUE)[[1]])
  parsed <- suppressWarnings(as.integer(pieces))
  parsed <- parsed[!is.na(parsed) & parsed > 0L]
  if (!length(parsed)) {
    default
  } else {
    unique(parsed)
  }
}

build_synthetic_kin_population <- function(blocks) {
  base_pid <- (seq_len(blocks) - 1L) * 10L
  base_mid <- (seq_len(blocks) - 1L) * 3L

  pid1 <- base_pid + 1L
  pid2 <- base_pid + 2L
  pid3 <- base_pid + 3L
  pid4 <- base_pid + 4L
  pid5 <- base_pid + 5L
  pid6 <- base_pid + 6L
  pid7 <- base_pid + 7L
  pid8 <- base_pid + 8L
  pid9 <- base_pid + 9L
  pid10 <- base_pid + 10L

  pid <- c(pid1, pid2, pid3, pid4, pid5, pid6, pid7, pid8, pid9, pid10)
  fem <- c(
    rep.int(1L, blocks), rep.int(0L, blocks),
    rep.int(1L, blocks), rep.int(0L, blocks),
    rep.int(0L, blocks), rep.int(1L, blocks),
    rep.int(1L, blocks), rep.int(0L, blocks),
    rep.int(1L, blocks), rep.int(0L, blocks)
  )
  dob <- c(
    rep.int(100L, blocks), rep.int(100L, blocks),
    rep.int(220L, blocks), rep.int(220L, blocks),
    rep.int(220L, blocks), rep.int(220L, blocks),
    rep.int(340L, blocks), rep.int(340L, blocks),
    rep.int(340L, blocks), rep.int(340L, blocks)
  )
  mom <- c(
    rep.int(0L, blocks), rep.int(0L, blocks),
    pid1, pid1,
    rep.int(0L, blocks), rep.int(0L, blocks),
    pid3, pid3,
    pid6, pid6
  )
  pop <- c(
    rep.int(0L, blocks), rep.int(0L, blocks),
    pid2, pid2,
    rep.int(0L, blocks), rep.int(0L, blocks),
    pid5, pid5,
    pid4, pid4
  )
  marid <- c(
    base_mid + 1L, base_mid + 1L,
    base_mid + 2L, base_mid + 3L,
    base_mid + 2L, base_mid + 3L,
    rep.int(0L, blocks), rep.int(0L, blocks),
    rep.int(0L, blocks), rep.int(0L, blocks)
  )

  opop <- data.frame(
    pid = pid,
    fem = fem,
    group = rep.int(1L, length(pid)),
    nev = rep.int(0L, length(pid)),
    dob = dob,
    mom = mom,
    pop = pop,
    nesibm = rep.int(0L, length(pid)),
    nesibp = rep.int(0L, length(pid)),
    lborn = rep.int(0L, length(pid)),
    marid = marid,
    mstat = ifelse(marid == 0L, 1L, 4L),
    dod = rep.int(0L, length(pid)),
    fmult = rep.int(0, length(pid))
  )

  omar <- data.frame(
    mid = c(base_mid + 1L, base_mid + 2L, base_mid + 3L),
    wpid = c(pid1, pid3, pid6),
    hpid = c(pid2, pid5, pid4),
    dstart = rep.int(0L, blocks * 3L),
    dend = rep.int(0L, blocks * 3L),
    rend = rep.int(0L, blocks * 3L),
    wprior = rep.int(0L, blocks * 3L),
    hprior = rep.int(0L, blocks * 3L)
  )

  benchmark_pid <- c(pid3, pid4, pid7, pid9)

  list(
    opop = opop,
    omar = omar,
    benchmark_pid = benchmark_pid,
    persons = nrow(opop)
  )
}

build_kids_of_lookup <- function(opop) {
  mother_kids <- split(opop$pid[opop$mom != 0L], as.character(opop$mom[opop$mom != 0L]))
  father_kids <- split(opop$pid[opop$pop != 0L], as.character(opop$pop[opop$pop != 0L]))
  parent_ids <- union(names(mother_kids), names(father_kids))

  kids_of <- stats::setNames(vector("list", length(parent_ids)), parent_ids)
  for (parent_id in parent_ids) {
    kids_of[[parent_id]] <- c(mother_kids[[parent_id]], father_kids[[parent_id]])
  }

  kids_of
}

measure_elapsed <- function(fun, reps) {
  timings <- numeric(reps)
  for (idx in seq_len(reps)) {
    invisible(gc())
    timings[[idx]] <- system.time(fun())[["elapsed"]]
  }
  timings
}

test_that("retrieve_kin opt-in benchmark remains roughly linear and benefits from KidsOf reuse", {
  testthat::skip_on_cran()
  if (!perf_env_flag("RSOCSIM_RUN_PERF_TESTS")) {
    testthat::skip("Set RSOCSIM_RUN_PERF_TESTS=true to run retrieve_kin performance tests.")
  }

  sizes <- perf_env_integer_vector(
    name = "RSOCSIM_PERF_POP_SIZES",
    default = c(50000L, 100000L, 200000L)
  )
  reps <- perf_env_integer(name = "RSOCSIM_PERF_REPS", default = 3L)
  pid_limit <- perf_env_integer(name = "RSOCSIM_PERF_PID_LIMIT", default = 4000L)
  extra_kintypes <- c("unclesaunts", "firstcousins", "niblings", "inlaws")

  warmup <- build_synthetic_kin_population(50L)
  invisible(retrieve_kin(
    opop = warmup$opop,
    omar = warmup$omar,
    pid = warmup$benchmark_pid[seq_len(min(length(warmup$benchmark_pid), 20L))],
    extra_kintypes = extra_kintypes,
    kin_by_sex = TRUE,
    KidsOf = list()
  ))

  benchmark_rows <- lapply(sizes, function(target_persons) {
    blocks <- max(1L, ceiling(target_persons / 10L))
    synthetic <- build_synthetic_kin_population(blocks)
    pid <- synthetic$benchmark_pid[seq_len(min(length(synthetic$benchmark_pid), pid_limit))]
    kids_of <- build_kids_of_lookup(synthetic$opop)

    default_runner <- function() {
      retrieve_kin(
        opop = synthetic$opop,
        omar = synthetic$omar,
        pid = pid,
        extra_kintypes = extra_kintypes,
        kin_by_sex = TRUE,
        KidsOf = NULL
      )
    }
    cached_runner <- function() {
      retrieve_kin(
        opop = synthetic$opop,
        omar = synthetic$omar,
        pid = pid,
        extra_kintypes = extra_kintypes,
        kin_by_sex = TRUE,
        KidsOf = kids_of
      )
    }

    default_times <- measure_elapsed(default_runner, reps = reps)
    cached_times <- measure_elapsed(cached_runner, reps = reps)

    data.frame(
      persons = synthetic$persons,
      pid_count = length(pid),
      default_median = stats::median(default_times),
      cached_median = stats::median(cached_times),
      reuse_ratio = stats::median(cached_times) / stats::median(default_times),
      default_sec_per_100k = stats::median(default_times) / synthetic$persons * 100000,
      cached_sec_per_100k = stats::median(cached_times) / synthetic$persons * 100000
    )
  })

  benchmark_df <- do.call(rbind, benchmark_rows)
  scaling_df <- benchmark_df[benchmark_df$persons >= 10000L, , drop = FALSE]

  testthat::expect_true(all(benchmark_df$reuse_ratio <= 1.25))
  if (nrow(scaling_df) >= 2L && reps >= 2L && min(scaling_df$default_median) >= 1) {
    testthat::expect_true(
      max(scaling_df$default_sec_per_100k) <= min(scaling_df$default_sec_per_100k) * 4
    )
  }

  benchmark_text <- paste(capture.output(print(benchmark_df, row.names = FALSE)), collapse = "\n")
  message("retrieve_kin benchmark summary:\n", benchmark_text)
})
