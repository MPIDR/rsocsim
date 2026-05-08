## Development

Github Actions is configured so that it automatically builds the documentation and website when the version number in the DESCRIPTION file is increased.

But you can also use devtools to build the documentation and website. And to build the package (The package will not be build by Github Actions).

`devtools::document()` - generate documentation from source-code comments
`devtools::build()` - compile and build package
`devtools::build(binary = TRUE)` - compile binary

`devtools::check()` - run tests --> can it be published on CRAN? Runs several tests.

Simulations will be skipped by default tests. Run the following lines to activate more extensive testing:

```{R}
Sys.setenv(RSOCSIM_RUN_INTEGRATION_TESTS="1") # run simulations
Sys.setenv(RSOCSIM_MIRROR_TEST_ARTIFACTS="1") # copy some sim-results test folder
devtools::check()
``` 

The results will then be copied to `tests\testthat\tests\testthat\_results` for inspection

`usethis::use_github_action("pkgdown")`

`usethis::use_pkgdown_github_pages()`

`devtools::document()`

`pkgdown::build_site()`


On Windows, avoid running `devtools::check()` from a UNC or mapped-network working tree. `devtools` and `pkgbuild` can resolve the package path to a UNC location such as `\\server\share\...`, and the `cmd.exe`/Rtools toolchain used by `R CMD INSTALL` does not reliably support UNC working directories. Run checks from a local drive path instead.

#### Testing without devtools
Rscript -e "testthat::test_dir('tests/testthat')"