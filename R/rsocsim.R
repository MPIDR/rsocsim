#' @useDynLib rsocsim
#' @importFrom Rcpp evalCpp
#' @exportPattern "^[[:alpha:]]+"
NULL

# This file is necessary so that Roxigen2 and devtools::document() generate the correct NAMESPACE file.
# See https://stackoverflow.com/questions/33395264/trouble-building-r-package-wtih-devtools-when-it-uses-rcpparmadillo

if (getRversion() >= "2.15.1") {
  utils::globalVariables(c("dod", "dob", "dod2", "census", "fem", "sex",
                            "age_at_census", "agegr_at_census", "n",
                            "pid", "mom", "birth_year", "mother_birth",
                            "birth_year_factor", "mother_age", "mother_agegr_factor"))
}