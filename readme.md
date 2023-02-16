# rsocsim - Socsim R package

Socsim stands for Social Simulator.
It is a microsimulation to simulate populations and their kinship structure.

rsocsim is still under development. You might find some bugs or unexpected behavior,
the API might change without warnings in the next weeks.
If you encounter an error or bug, we are happy to hear from you in the issues.

## Installation

### From Source



To get the latest version, you need to have the package devtools and a 
Rcpp compatible compiler installed ([RTools](https://cran.r-project.org/bin/windows/Rtools/index.html)
on Windows, Xcode command line tools on Mac: `xcode-select --install`, 
`sudo apt-get install r-base-dev` on Linux,
all according to https://teuder.github.io/rcpp4everyone_en/020_install.html)


* install devtools
````R
install.packages("devtools")
````

Then you can use devtools to install rsocsim from Github:
````R
devtools::install_github("MPIDR/rsocsim")
````

### Install precompiled package (Windows only)

If the compilation from source fails, you can also install a pre-compiled binary. This might be easier, but it might be not the most up-to-date version. You can find the most up-to-date binary release on https://github.com/tomthe/rsoc/releases

````R
url = "https://github.com/tomthe/rsoc/releases/download/1.3/rsocsim_1.3.zip"
install.packages(url, repos = NULL, type = "win.binary")
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
rsocsim::socsim(folder,supfile,seed)

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


````

## Develop the package

You will need RTools, devtools and rcpp.
Clone this repository, change something, hit "Build-->clean and rebuild"
(If that fails because the package is in use, you can restart the R-session (CTRL-SHIFT-F10)).


renew documentation and some stuff with:
```
devtools.document()
```

build source and binary package:
```
devtools::build(binary=FALSE)
devtools::build(binary=TRUE)
```

### Future plans

* Testing and verification
* including convenient R-functions for generating input and reading/processing output
  * possible options:
    * include those functions into this package
    * create another package that loads this package

