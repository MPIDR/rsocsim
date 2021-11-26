# rsoc - preliminary socsim R package

## Installation

I am not entirely sure whether you need to have RTools installed. Please report back if you were able to use this package without RTools!

* install devtools
````R
install.packages("devtools")
````

* install Rcpp (I am also not really sure whether you really need rcpp. Maybe not!)
````R
install.packages("rcpp")
````

* install rsoc from Github with devtools:
````
devtools::install_github("tomthe/rsoc")
````

## Use rsoc

````
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
Clone this repository, change something, hit "Build--clean and rebuild"

renew documentation and some stuff with:
```` devtools.document()````

### Future plans

* Testing and verification
* including convenient R-functions for generating input and reading/processing output
 * possible options:
  * include those functions into this package
  * create another package that loads this package

