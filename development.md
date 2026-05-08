# NA

## Development

Github Actions is configured so that it automatically builds the
documentation and website when the version number in the DESCRIPTION
file is increased.

But you can also use devtools to build the documentation and website.
And to build the package (The package will not be build by Github
Actions).

`devtools::document()` - generate documentation from source-code
comments `devtools::build()` - compile and build package
`devtools::build(binary = TRUE)` - compile binary

`devtools::check()` - run tests –\> can it be published on CRAN? Runs
several tests.

Simulations will be skipped by default tests. Run the following lines to
activate more extensive testing:

`{R} Sys.setenv(RSOCSIM_RUN_INTEGRATION_TESTS="1") # run simulations Sys.setenv(RSOCSIM_MIRROR_TEST_ARTIFACTS="1") # copy some sim-results test folder Sys.setenv(RSOCSIM_TEST_CONSOLE_OUTPUT="1") # output to R console during simulation-tests devtools::check()`

To also show the SOCSIM progress messages and population pyramids during
local test runs, set:

`{R} Sys.setenv(RSOCSIM_TEST_CONSOLE_OUTPUT="1")`

For local runs with `devtools::test()` or
[`testthat::test_file()`](https://testthat.r-lib.org/reference/test_file.html),
mirrored artifacts are copied into `tests/testthat/_results`.

When running `devtools::check()`, tests execute in a temporary
`*.Rcheck` copy of the package, so the mirrored files are created under
that temporary tree instead of your working checkout.

`usethis::use_github_action("pkgdown")`

`usethis::use_pkgdown_github_pages()`

`devtools::document()`

[`pkgdown::build_site()`](https://pkgdown.r-lib.org/reference/build_site.html)

On Windows, avoid running `devtools::check()` from a UNC or
mapped-network working tree. `devtools` and `pkgbuild` can resolve the
package path to a UNC location such as `\\server\share\...`, and the
`cmd.exe`/Rtools toolchain used by `R CMD INSTALL` does not reliably
support UNC working directories. Run checks from a local drive path
instead.

#### Testing without devtools

Rscript -e “testthat::test_dir(‘tests/testthat’)”
