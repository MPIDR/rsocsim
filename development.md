## Development

Github Actions is configured so that it automatically builds the documentation and website when the version number in the DESCRIPTION file is increased.

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