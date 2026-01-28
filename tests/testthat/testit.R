library(testthat)
library(rsocsim)

test_that("create_initial_population writes opop and omar", {
	simdir <- file.path(tempdir(), "rsocsim-pop")
	dir.create(simdir, showWarnings = FALSE, recursive = TRUE)

	create_initial_population(folder = simdir, size_opop = 20, output_base = "init")

	opop_path <- file.path(simdir, "init.opop")
	omar_path <- file.path(simdir, "init.omar")

	expect_true(file.exists(opop_path))
	expect_true(file.exists(omar_path))

	opop <- read_opop(fn = opop_path)
	expect_equal(ncol(opop), 14)
	expect_equal(nrow(opop), 20)
	expect_true(all(c("pid", "fem", "group", "dob") %in% names(opop)))

	write.table(
		data.frame(1, 1, 2, 0, 0, 0, 0, 0),
		omar_path,
		row.names = FALSE,
		col.names = FALSE
	)
	omar <- read_omar(fn = omar_path)
	expect_equal(ncol(omar), 8)
	expect_equal(nrow(omar), 1)
})