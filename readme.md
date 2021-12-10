# rsoc - preliminary socsim R package

## Installation

### From Source

To get the latest version, you need to have RTools and devtools installed
and install from source directly from this repository

* install devtools
````R
install.packages("devtools")
````

* install Rcpp (I am also not really sure whether you really need rcpp. Maybe not!)
````R
install.packages("rcpp")
````

* install rsoc from Github with devtools:
````R
devtools::install_github("tomthe/rsoc")
````

### Install precompiled package (Windows only)

You can also install a pre-compiled binary. This might be easier, but it might
be not the most up-to-date version. You can find the most up-to-date binary release
on https://github.com/tomthe/rsoc/releases

````R
> url = "https://github.com/tomthe/rsoc/releases/download/binary/rsoc_1.1.zip"
> install.packages(url, repos = NULL, type = "win.binary")
````

## Use rsoc

````R
library("rsoc")

# specify the working directory, where your supfile and ratefiles are:
folder = "D:\\dev\\r\\socsimprojects\\CousinDiversity" 

# name of the supplement-file, relative to the above folder:
supfile = "CousinDiversity.sup" 

# Random number generator seed:
seed="33" 

# run1simulationwithfile starts a simulation with the specified sup-file
rsoc::run1simulationwithfile(folder,supfile,seed)

# if you run several simulations, previous simulations may influence/corrupt
# later simulations. Use one of the options "future" or "clustercall" for
# process_method to prevent this. "future" has problems on computers with more
# than 64 cores. Note that these options prevent SOCSIM from writing to the
# R-terminal.

rsoc::run1simulationwithfile(folder,supfile,seed,process_method = "future")
rsoc::run1simulationwithfile(folder,supfile,seed,process_method = "clustercall")


````

## Develop the package

You need RTools, devtools and rcpp.
Clone this repository, change something, hit "Build-->clean and rebuild"

renew documentation and some stuff with:
```` devtools.document()````

### Future plans

* Testing and verification
* including convenient R-functions for generating input and reading/processing output
  * possible options:
    * include those functions into this package
    * create another package that loads this package

