## Test environments

* https://win-builder.r-project.org/
* https://mac.r-project.org/macbuilder/
* local Windows 11 x64, R 4.4.3 (2025-02-28 ucrt)

## Comments

* Win-builder gives me 1 note: "SOCSIM and microsimulation might be misspelled". They are not misspelled, just uncommon.
* Some functions do not have an @examples section... but all functions are covered by extensive tests which can be run by setting the environment variable `RSOCSIM_RUN_INTEGRATION_TESTS="1"`. These tests run whole simulations and take some time to finish.
* The package is being submitted to CRAN for the ~~first~~ second time.
* As requested, I changed the readme so that the links to CRAN packages are now in the canonical form.