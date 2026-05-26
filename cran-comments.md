## Test environments

* https://win-builder.r-project.org/ R version 4.6.0 (2026-04-24 ucrt) (one note about misspellings)
* https://mac.r-project.org/macbuilder/ (0 notes)
* local Windows 11 x64, R 4.4.3 (2025-02-28 ucrt) (1 note: unable to verify current time)

## Comments

* Win-builder gives me 1 note: "Possibly misspelled words in DESCRIPTION:" "SOCSIM" and "microsimulation". They are not misspelled, just uncommon.
* Some functions do not have an @examples section... but all functions are covered by extensive tests which can be run by setting the environment variable `RSOCSIM_RUN_INTEGRATION_TESTS="1"`. These tests run whole simulations and take some time to finish.
* The package is being submitted to CRAN for the ~~first~~ second time.
* As requested, I changed the readme so that the links to CRAN packages are now in the canonical form.