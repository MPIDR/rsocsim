test_that("read_results preserves parsed data for populated result files", {
  opop_expected <- data.frame(
    pid = 1:2,
    fem = c(1L, 0L),
    group = c(1L, 1L),
    nev = c(0L, 0L),
    dob = c(100L, 101L),
    mom = c(0L, 0L),
    pop = c(0L, 0L),
    nesibm = c(0L, 0L),
    nesibp = c(0L, 0L),
    lborn = c(0L, 0L),
    marid = c(0L, 0L),
    mstat = c(1L, 1L),
    dod = c(0L, 0L),
    fmult = c(1L, 1L)
  )
  omar_expected <- data.frame(
    mid = 1L,
    wpid = 1L,
    hpid = 2L,
    dstart = 10L,
    dend = 0L,
    rend = 0L,
    wprior = 0L,
    hprior = 0L
  )

  opop_path <- tempfile(fileext = ".opop")
  omar_path <- tempfile(fileext = ".omar")

  write.table(opop_expected, file = opop_path, row.names = FALSE, col.names = FALSE)
  write.table(omar_expected, file = omar_path, row.names = FALSE, col.names = FALSE)

  expect_equal(read_opop(fn = opop_path), opop_expected)
  expect_equal(read_omar(fn = omar_path), omar_expected)
})

test_that("retrieve_kin preserves current immediate kin output", {
  opop <- data.frame(
    pid = 1:4,
    fem = c(1, 0, 1, 0),
    group = 1,
    nev = 0,
    dob = c(120, 120, 300, 300),
    mom = c(0, 0, 1, 1),
    pop = c(0, 0, 2, 2),
    nesibm = 0,
    nesibp = 0,
    lborn = 0,
    marid = c(1, 1, 0, 0),
    mstat = c(4, 4, 1, 1),
    dod = 0,
    fmult = 0
  )

  omar <- data.frame(
    mid = 1,
    wpid = 1,
    hpid = 2,
    dstart = 0,
    dend = 0,
    rend = 0,
    wprior = 0,
    hprior = 0
  )

  res <- retrieve_kin(
    opop = opop,
    omar = omar,
    KidsOf = list(),
    pid = c(3),
    extra_kintypes = c("niblings", "inlaws"),
    kin_by_sex = TRUE
  )

  expect_equal(res$parents[[1]], c(1, 2))
  expect_equal(res$mother[[1]], 1)
  expect_equal(res$father[[1]], 2)
  expect_equal(res$siblings[[1]], 4)
  expect_true(is.na(res$sisters[[1]]))
  expect_equal(res$brothers[[1]], 4)
  expect_true(is.na(res$children[[1]]))
  expect_true(is.na(res$spouse[[1]]))
  expect_true(is.na(res$niblings[[1]]))
  expect_true(is.na(res$parentsinlaw[[1]]))
  expect_true(is.na(res$siblingsinlaw[[1]]))
})

test_that("read_results returns typed empty frames for missing files", {
  missing_opop <- file.path(tempdir(), "missing-result.opop")
  missing_omar <- file.path(tempdir(), "missing-result.omar")

  expect_warning(opop_empty <- read_opop(fn = missing_opop, quiet = TRUE), "population file missing or empty")
  expect_warning(omar_empty <- read_omar(fn = missing_omar, quiet = TRUE), "marriage file missing or empty")

  expect_equal(names(opop_empty), c("pid", "fem", "group", "nev", "dob", "mom", "pop", "nesibm", "nesibp", "lborn", "marid", "mstat", "dod", "fmult"))
  expect_equal(names(omar_empty), c("mid", "wpid", "hpid", "dstart", "dend", "rend", "wprior", "hprior"))
  expect_equal(nrow(opop_empty), 0)
  expect_equal(nrow(omar_empty), 0)
  expect_type(opop_empty$fmult, "double")
})

test_that("retrieve_kin derives first cousins without explicit unclesaunts", {
  opop <- data.frame(
    pid = 1:7,
    fem = c(1, 0, 1, 0, 0, 1, 0),
    group = 1,
    nev = 0,
    dob = c(100, 100, 220, 220, 220, 340, 360),
    mom = c(0, 0, 1, 1, 0, 3, 0),
    pop = c(0, 0, 2, 2, 0, 5, 4),
    nesibm = 0,
    nesibp = 0,
    lborn = 0,
    marid = c(1, 1, 2, 0, 2, 0, 0),
    mstat = c(4, 4, 4, 1, 4, 1, 1),
    dod = 0,
    fmult = 0
  )

  omar <- data.frame(
    mid = 1:2,
    wpid = c(1, 3),
    hpid = c(2, 5),
    dstart = 0,
    dend = 0,
    rend = 0,
    wprior = 0,
    hprior = 0
  )

  res <- retrieve_kin(
    opop = opop,
    omar = omar,
    pid = c(6),
    extra_kintypes = "firstcousins",
    kin_by_sex = FALSE
  )

  expect_equal(res$firstcousins[[1]], 7)
})

test_that("retrieve_kin honors supplied non-empty KidsOf", {
  opop <- data.frame(
    pid = 1:4,
    fem = c(1, 0, 1, 0),
    group = 1,
    nev = 0,
    dob = c(120, 120, 300, 300),
    mom = c(0, 0, 1, 1),
    pop = c(0, 0, 2, 2),
    nesibm = 0,
    nesibp = 0,
    lborn = 0,
    marid = c(1, 1, 0, 0),
    mstat = c(4, 4, 1, 1),
    dod = 0,
    fmult = 0
  )

  omar <- data.frame(
    mid = 1,
    wpid = 1,
    hpid = 2,
    dstart = 0,
    dend = 0,
    rend = 0,
    wprior = 0,
    hprior = 0
  )

  res <- retrieve_kin(
    opop = opop,
    omar = omar,
    KidsOf = list("3" = 999),
    pid = c(3),
    kin_by_sex = FALSE
  )

  expect_equal(res$children[[1]], 999)
})