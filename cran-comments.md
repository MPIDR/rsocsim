## Test environments

* https://win-builder.r-project.org/ R version 4.6.0 (2026-04-24 ucrt) (one note about misspellings)
* https://mac.r-project.org/macbuilder/ (0 notes)
* local Windows 11 x64, R 4.4.3 (2025-02-28 ucrt) (1 note: unable to verify current time)
 
## comments

* I addressed all the comments I received for the previous submussion from Benjamin Altmann:
    * supressable output: replaced print statements with a function that checks the environment variable RSOCSIM_TEST_CONSOLE_OUTPUT and then uses message(), depending on the value of RSOCSIM_TEST_CONSOLE_OUTPUT
    * immediate on.exit() instead of finally() to reset working the directory
    * remove \dontrun or replace \dontrun with \donttest in documentation
* full simulation tests still only run with the env variable `RSOCSIM_RUN_INTEGRATION_TESTS="1"`
