## Development

Github Actions is configured so that it automatically builds the documentation and website when the version number in the DESCRIPTION file is increased.

On Windows, avoid running `devtools::check()` from a UNC or mapped-network working tree. `devtools` and `pkgbuild` can resolve the package path to a UNC location such as `\\server\share\...`, and the `cmd.exe`/Rtools toolchain used by `R CMD INSTALL` does not reliably support UNC working directories. Run checks from a local drive path instead.

But you can also use devtools to build the documentation and website. And to build the package (The package will not be build by Github Actions).

`devtools::document()` - generate documentation from source-code comments
`devtools::build()` - compile and build package
`devtools::build(binary = TRUE)` - compile binary

`devtools::check()` - run tests --> can it be published on CRAN? Runs several tests.


`usethis::use_github_action("pkgdown")`

`usethis::use_pkgdown_github_pages()`

`devtools::document()`

`pkgdown::build_site()`


#### Testing without devtools
Rscript -e "testthat::test_dir('tests/testthat')"