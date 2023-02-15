# rsocsim - Socsim R package

Socsim stands for Social Simulator.
It is a microsimulation to simulate populations and their kinship structure.


## Installation

### From Source

To get the latest version, you need to have RTools and devtools installed
and install from source directly from this repository

* install devtools
````R
install.packages("devtools")
````

* install rsocsim from Github with devtools:
````R
devtools::install_github("MPIDR/rsocsim")
````

### Install precompiled package (Windows only)

If the compilation from suorce fails, you can also install a pre-compiled binary. This might be easier, but it might be not the most up-to-date version. You can find the most up-to-date binary release on https://github.com/tomthe/rsoc/releases

````R
> url = "https://github.com/tomthe/rsoc/releases/download/1.3/rsocsim_1.3.zip"
> install.packages(url, repos = NULL, type = "win.binary")
````

## Use rsocsim


### Getting started

````R
library("rsocsim")

# create a new folder for all the files related to a simulation.
# this will be in your home- or user-directory:
folder = rsocsim::create_simulation_folder()

# create a new supplement-file. Supplement-files tell socsim what
# to simulate. create_sup_file will create a very basic supplement filee
# and it copies some rate-files that will also be needed into the 
# simulation folder:
supfile = rsocsim::create_sup_file(folder)

# Choose a random-number seed:
seed = 300

# Start the simulation:
rsocsim::socsim(folder,supfile,seed,process_method = "inprocess")

````

The results will be saved into a subfolder of the  simulation-`folder´.
For more sophisticated simulations you will need to edit the supplement
file and provide rate files for fertility, mortality and marriage rates.



````R
library("rsocsim")

# specify the working directory, where your supfile and ratefiles are:
folder = "D:\\dev\\r\\socsimprojects\\CousinDiversity" 

# name of the supplement-file, relative to the above folder:
supfile = "CousinDiversity.sup" 

# Random number generator seed:
seed="33" 

# socsim starts a simulation with the specified sup-file
rsocsim::socsim(folder,supfile,seed)

# if you run several simulations, previous simulations may influence/corrupt
# later simulations. Use one of the options "future" or "clustercall" for
# process_method to prevent this. "future" has problems on computers with more
# than 64 cores. Note that these options prevent SOCSIM from writing to the
# R-terminal.

rsocsim::socsim(folder,supfile,seed,process_method = "future")
rsocsim::socsim(folder,supfile,seed,process_method = "clustercall")


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

