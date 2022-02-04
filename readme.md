# rsoc - preliminary socsim R package

## Installation

### From Source

To get the latest version, you need to have RTools and devtools installed
and install from source directly from this repository

* install devtools
````R
install.packages("devtools")
````

* install rsoc from Github with devtools:
````R
devtools::install_github("tomthe/rsoc")
````

### Install precompiled package (Windows only)

If the compilation from suorce fails, you can also install a pre-compiled binary. This might be easier, but it might be not the most up-to-date version. You can find the most up-to-date binary release on https://github.com/tomthe/rsoc/releases

````R
> url = "https://github.com/tomthe/rsoc/releases/download/binary/rsoc_1.1.zip"
> install.packages(url, repos = NULL, type = "win.binary")
````

## Use rsoc


### Getting started

````R
library("rsoc")

# create a new folder for all the files related to a simulation.
# this will be in your home- or user-directory:
folder = rsoc::create_simulation_folder()
#folder = "C:/Users/tom/socsim/socsim_sim_3792/"

# create a new supplement-file. Supplement-files tell socsim what
# to simulate. create_sup_file will create a very basic supplement filee
# and it copies some rate-files that will also be needed into the 
# simulation folder:
supfile = rsoc::create_sup_file(folder)

# Choose a random-number seed:
seed = 300

# Start the simulation:
rsoc::run1simulationwithfile(folder,supfile,seed,process_method = "inprocess")

````

The results will be saved into a subfolder of the  simulation-`folder´.
For more sophisticated simulations you will need to edit the supplement
file and provide rate files for fertility, mortality and marriage rates.



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

You will need RTools, devtools and rcpp.
Clone this repository, change something, hit "Build-->clean and rebuild"

renew documentation and some stuff with:
```` devtools.document()````

### Future plans

* Testing and verification
* including convenient R-functions for generating input and reading/processing output
  * possible options:
    * include those functions into this package
    * create another package that loads this package

