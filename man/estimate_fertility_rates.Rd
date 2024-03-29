% Generated by roxygen2: do not edit by hand
% Please edit documentation in R/estimate_rates.R
\name{estimate_fertility_rates}
\alias{estimate_fertility_rates}
\title{Estimate yearly age-specific fertility rates (ASFR) from a SOCSIM-generated
population file}
\usage{
estimate_fertility_rates(
  opop,
  final_sim_year,
  year_min,
  year_max,
  year_group = 5,
  age_min_fert = 15,
  age_max_fert = 50,
  age_group = 5
)
}
\arguments{
\item{opop}{An R object from SOCSIM microsimulation output (population file).}

\item{final_sim_year}{numeric. Final simulated year in 'real world' time (
used to convert 'SOCSIM time' to 'real world' time.)}

\item{year_min}{numeric. Lower-bound year for which rate should be estimated.}

\item{year_max}{numeric. Upper-bound year for which rate should be estimated.}

\item{year_group}{numeric. Size of year groups to estimate rate (year_group=1
will produce single-year estimates)}

\item{age_min_fert}{numeric. Upper-bound age of female reproductive period}

\item{age_group}{numeric. Size of age groups to estimate rate (age_group=1
will produce single-age estimates)}
}
\value{
A data.frame with age-specific fertility rates by year and age.
}
\description{
Given a population file ('opop') generated by rsocsim, the
function estimates age-specific fertility rates.
}
\details{
The \code{final_sim_year} can be obtained from the .sup file and must
refer to to a real-world year.

Grouped year and age ranges (i.e., if \code{year_group > 1} or \code{age_group > 1})
are created as [year;year+year_group).
}
\examples{
\dontrun{
# Read opop file into global environment
opop <- read_opop(path = "my_path")
# Retrieve age-specific fertility rates
asfr <- get_asfr_socsim(opop = opop,
                     final_sim_year = 2021, 
                     year_min = 1750,
                     year_max = 2020,
                     year_group = 5,
                     age_min_fert = 10,
                     age_max_fert = 55,
                     age_group = 5)
}
}
