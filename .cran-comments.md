## Test environments

* local Windows 11 x64, R 4.4.3 (2025-02-28 ucrt)
* https://mac.r-project.org/macbuilder/

## R CMD check results

* `R CMD check --as-cran --no-manual` on the source tarball returned 0 ERRORs, 0 WARNINGs, and 2 NOTEs.
* NOTE: `New submission`.
* NOTE: `checking for future file timestamps ... unable to verify current time`.

## Comments

* The package is being submitted to CRAN for the first time.
* The `future file timestamps` note appears to be environment-specific to this local Windows machine.
* A full remote check on win-builder is the next validation step before upload.
* Has been tested successfully with https://mac.r-project.org/macbuilder/ with 0 notes.
* Some functions do not have an @examples section... but all functions are covered by extensive tests which can be run by setting the environment variable `RSOCSIM_RUN_INTEGRATION_TESTS="1"`.