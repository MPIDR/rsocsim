#' rsocsim: Run SOCSIM Simulations from R
#'
#' Utilities for creating SOCSIM simulation folders, preparing inputs,
#' downloading rate files, running simulations, and reading output files.
#'
#' @keywords internal
#' @useDynLib rsocsim, .registration = TRUE
#' @importFrom Rcpp evalCpp
"_PACKAGE"

# This file is necessary so that Roxigen2 and devtools::document() generate the correct NAMESPACE file.
# See https://stackoverflow.com/questions/33395264/trouble-building-r-package-wtih-devtools-when-it-uses-rcpparmadillo

if (getRversion() >= "2.15.1") {
  utils::globalVariables(c("dod", "dob", "dod2", "census", "fem", "sex",
                            "age", "age_at_census", "age_death",
                            "age_death_g", "age_death_months",
                            "agegr", "agegr_at_census", "death_year",
                            "deno", "mother_agegr", "n", "n_den",
                            "n_num", "nume", "pid", "mom", "birth_year",
                            "mother_birth", "birth_year_factor",
                            "mother_age", "mother_agegr_factor", "year",
                            "year_gr"))
}