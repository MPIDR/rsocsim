library(testthat)
library(rsocsim)

test_that("estimate_fertility_rates returns expected structure", {
	opop <- data.frame(
		pid = 1:6,
		fem = c(1, 0, 1, 0, 1, 1),
		group = 1,
		nev = 0,
		dob = c(120, 120, 336, 348, 180, 360),
		mom = c(0, 0, 1, 1, 0, 5),
		pop = c(0, 0, 2, 2, 0, 2),
		nesibm = 0,
		nesibp = 0,
		lborn = 0,
		marid = 0,
		mstat = 0,
		dod = c(0, 300, 0, 0, 0, 0),
		fmult = 0
	)

	asfr <- estimate_fertility_rates(
		opop = opop,
		final_sim_year = 2000,
		year_min = 1998,
		year_max = 2000,
		year_group = 1,
		age_min_fert = 15,
		age_max_fert = 50,
		age_group = 5
	)

	expect_true(all(c("year", "age", "socsim") %in% names(asfr)))
	expect_true(all(asfr$socsim >= 0, na.rm = TRUE))
})

test_that("estimate_mortality_rates returns expected structure", {
	opop <- data.frame(
		pid = 1:6,
		fem = c(1, 0, 1, 0, 1, 1),
		group = 1,
		nev = 0,
		dob = c(120, 120, 336, 348, 180, 360),
		mom = c(0, 0, 1, 1, 0, 5),
		pop = c(0, 0, 2, 2, 0, 2),
		nesibm = 0,
		nesibp = 0,
		lborn = 0,
		marid = 0,
		mstat = 0,
		dod = c(0, 300, 0, 0, 0, 0),
		fmult = 0
	)

	asmr <- estimate_mortality_rates(
		opop = opop,
		final_sim_year = 2000,
		year_min = 1995,
		year_max = 2000,
		year_group = 5,
		age_max_mort = 100,
		age_group = 5
	)

	expect_true(all(c("year", "sex", "age", "socsim") %in% names(asmr)))
	expect_true(all(is.finite(na.omit(asmr$socsim))))
})

test_that("retrieve_kin returns expected immediate kin", {
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

	expect_equal(res$mother[[1]], 1)
	expect_equal(res$father[[1]], 2)
	expect_true(4 %in% res$siblings[[1]])
	expect_true(is.na(res$spouse[[1]]))
})