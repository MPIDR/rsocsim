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

test_that("read_results combines multiple fn inputs with ID offsets", {
  opop_run <- data.frame(
    pid = 1:4,
    fem = c(1L, 0L, 1L, 0L),
    group = c(1L, 1L, 1L, 1L),
    nev = c(0L, 0L, 0L, 0L),
    dob = c(120L, 120L, 300L, 300L),
    mom = c(0L, 0L, 1L, 1L),
    pop = c(0L, 0L, 2L, 2L),
    nesibm = c(0L, 0L, 0L, 3L),
    nesibp = c(0L, 0L, 0L, 3L),
    lborn = c(4L, 4L, 0L, 0L),
    marid = c(1L, 1L, 0L, 0L),
    mstat = c(4L, 4L, 1L, 1L),
    dod = c(0L, 0L, 0L, 0L),
    fmult = c(0, 0, 0, 0)
  )
  omar_run <- data.frame(
    mid = 1L,
    wpid = 1L,
    hpid = 2L,
    dstart = 0L,
    dend = 0L,
    rend = 0L,
    wprior = 0L,
    hprior = 0L
  )

  opop_path1 <- tempfile(fileext = ".opop")
  opop_path2 <- tempfile(fileext = ".opop")
  omar_path1 <- tempfile(fileext = ".omar")
  omar_path2 <- tempfile(fileext = ".omar")

  write.table(opop_run, file = opop_path1, row.names = FALSE, col.names = FALSE)
  write.table(opop_run, file = opop_path2, row.names = FALSE, col.names = FALSE)
  write.table(omar_run, file = omar_path1, row.names = FALSE, col.names = FALSE)
  write.table(omar_run, file = omar_path2, row.names = FALSE, col.names = FALSE)

  opop_combined <- read_opop(fn = c(opop_path1, opop_path2), id_offset = 10L, quiet = TRUE)
  omar_combined <- read_omar(fn = c(omar_path1, omar_path2), id_offset = 10L, quiet = TRUE)

  expect_equal(opop_combined$pid, c(1L, 2L, 3L, 4L, 11L, 12L, 13L, 14L))
  expect_equal(opop_combined$mom, c(0L, 0L, 1L, 1L, 0L, 0L, 11L, 11L))
  expect_equal(opop_combined$pop, c(0L, 0L, 2L, 2L, 0L, 0L, 12L, 12L))
  expect_equal(opop_combined$nesibm, c(0L, 0L, 0L, 3L, 0L, 0L, 0L, 13L))
  expect_equal(opop_combined$nesibp, c(0L, 0L, 0L, 3L, 0L, 0L, 0L, 13L))
  expect_equal(opop_combined$lborn, c(4L, 4L, 0L, 0L, 14L, 14L, 0L, 0L))
  expect_equal(opop_combined$marid, c(1L, 1L, 0L, 0L, 11L, 11L, 0L, 0L))

  expect_equal(omar_combined$mid, c(1L, 11L))
  expect_equal(omar_combined$wpid, c(1L, 11L))
  expect_equal(omar_combined$hpid, c(2L, 12L))

  kin <- retrieve_kin(
    opop = opop_combined,
    omar = omar_combined,
    pid = 13,
    extra_kintypes = character(),
    kin_by_sex = TRUE,
    KidsOf = list()
  )

  expect_equal(kin$parents[[1]], c(11L, 12L))
  expect_equal(kin$siblings[[1]], 14L)
})

test_that("read_results combines multiple seed inputs via resolved result paths", {
  simdir <- tempfile(pattern = "read-results-")
  dir.create(simdir)

  dir1 <- file.path(simdir, "sim_results_1_")
  dir2 <- file.path(simdir, "sim_results_2_")
  dir.create(dir1)
  dir.create(dir2)

  opop_seed1 <- data.frame(1L, 1L, 1L, 0L, 100L, 0L, 0L, 0L, 0L, 0L, 0L, 1L, 0L, 1)
  opop_seed2 <- data.frame(1L, 0L, 1L, 0L, 101L, 0L, 0L, 0L, 0L, 0L, 0L, 1L, 0L, 1)

  write.table(opop_seed1,
              file = file.path(dir1, "result.opop"),
              row.names = FALSE,
              col.names = FALSE)
  write.table(opop_seed2,
              file = file.path(dir2, "result.opop"),
              row.names = FALSE,
              col.names = FALSE)

  combined <- read_opop(folder = simdir,
                        supfile = "socsim.sup",
                        seed = c(1, 2),
                        id_offset = 10L,
                        quiet = TRUE)

  expect_equal(combined$pid, c(1L, 11L))
  expect_equal(combined$fem, c(1L, 0L))
  expect_equal(combined$dob, c(100L, 101L))
})

test_that("read_results stops when id_offset is too small for file IDs", {
  opop_path1 <- tempfile(fileext = ".opop")
  opop_path2 <- tempfile(fileext = ".opop")
  opop_run <- data.frame(11L, 1L, 1L, 0L, 100L, 0L, 0L, 0L, 0L, 0L, 0L, 1L, 0L, 1)

  write.table(opop_run, file = opop_path1, row.names = FALSE, col.names = FALSE)
  write.table(opop_run, file = opop_path2, row.names = FALSE, col.names = FALSE)

  expect_error(
    read_opop(fn = c(opop_path1, opop_path2), id_offset = 10L, quiet = TRUE),
    "IDs exceed id_offset=10"
  )
})

test_that("read_results warns and stops before integer overflow", {
  opop_path <- tempfile(fileext = ".opop")
  opop_run <- data.frame(1L, 1L, 1L, 0L, 100L, 0L, 0L, 0L, 0L, 0L, 0L, 1L, 0L, 1)

  write.table(opop_run, file = opop_path, row.names = FALSE, col.names = FALSE)

  expect_warning(
    expect_error(
      read_opop(fn = c(opop_path, opop_path), id_offset = .Machine$integer.max, quiet = TRUE),
      "overflow integer IDs"
    ),
    "lower 'id_offset'"
  )
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

test_that("retrieve_kin matches internally built and supplied KidsOf outputs", {
  opop <- data.frame(
    pid = 1:10,
    fem = c(1, 0, 1, 0, 0, 1, 1, 0, 1, 0),
    group = 1,
    nev = 0,
    dob = c(100, 100, 220, 220, 220, 220, 340, 340, 340, 340),
    mom = c(0, 0, 1, 1, 0, 0, 3, 3, 6, 6),
    pop = c(0, 0, 2, 2, 0, 0, 5, 5, 4, 4),
    nesibm = 0,
    nesibp = 0,
    lborn = 0,
    marid = c(1, 1, 2, 3, 2, 3, 0, 0, 0, 0),
    mstat = c(4, 4, 4, 4, 4, 4, 1, 1, 1, 1),
    dod = 0,
    fmult = 0
  )

  omar <- data.frame(
    mid = 1:3,
    wpid = c(1, 3, 6),
    hpid = c(2, 5, 4),
    dstart = 0,
    dend = 0,
    rend = 0,
    wprior = 0,
    hprior = 0
  )

  mother_kids <- split(opop$pid[opop$mom != 0], as.character(opop$mom[opop$mom != 0]))
  father_kids <- split(opop$pid[opop$pop != 0], as.character(opop$pop[opop$pop != 0]))
  parent_ids <- union(names(mother_kids), names(father_kids))
  kids_of <- stats::setNames(vector("list", length(parent_ids)), parent_ids)
  for (parent_id in parent_ids) {
    kids_of[[parent_id]] <- c(mother_kids[[parent_id]], father_kids[[parent_id]])
  }

  res_internal <- retrieve_kin(
    opop = opop,
    omar = omar,
    pid = c(7, 9),
    extra_kintypes = c("gunclesaunts", "unclesaunts", "firstcousins", "niblings", "inlaws"),
    kin_by_sex = TRUE,
    KidsOf = NULL
  )

  res_supplied <- retrieve_kin(
    opop = opop,
    omar = omar,
    pid = c(7, 9),
    extra_kintypes = c("gunclesaunts", "unclesaunts", "firstcousins", "niblings", "inlaws"),
    kin_by_sex = TRUE,
    KidsOf = kids_of
  )

  expect_equal(res_supplied, res_internal)
})
