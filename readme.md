# rsocsim - Socsim R package

Socsim ('Social Simulator') is an open source stochastic microsimulation platform used to produce synthetic populations with plausible kinship structures using demographic rates as input.
We stand on the shoulders of giants: Socsim was originally developed for Unix at UC Berkeley [1-2], where it has been mantained from decades. The current release of `rsocsim` aims to be OS-agnostic and, for the most part, back-compatible with the original Socsim distribution (https://lab.demog.berkeley.edu/socsim/).

`rsocsim` is still under development. You might find some bugs or unexpected behavior, the API might change without warnings in the next weeks.
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

## Background and applications

SOCSIM is an open source and extensible demographic microsimulation program [1,3].^[The following description was adapted from the Supplementary Materials of [4].] 
It is written in the C programming language and relies heavily on arrays of linked lists to keep track of kinship relationships and to store information about simulated individuals. 
The simulator takes as input initial population files and monthly age-specific fertility and mortality demographic rates. 
The individual is the unit of analysis of the simulator. 
Each person is subject to a set of rates, expressed as monthly probabilities of events, given certain demographic characteristics, like age and sex. 
Every month, each individual faces the risk of experiencing a number of events, including childbirth, death, and marriage. 
The selection of the event and the waiting time until the event occurs are determined stochastically using a competing risk model. 
Some other constraints are included in the simulation program in order to draw events only for individuals that are eligible for the events (e.g. to allow for a minimum interval of time between births from the same mother, to avoid social taboos such as incest, etc.). 
Each event for which the individual is at risk is modeled as a piece-wise exponential distribution. 
The waiting time until each event occurs is randomly generated according to the associated demographic rates. 
The individual’s next event is the one with the shortest waiting time. 
At the end of the simulation, population files that contain a list of everyone who has ever lived in the population are created. 
In these files, each individual is an observation in a rectangular data file with records of demographic characteristics for the individual, and identification numbers for key kinship relations.
SOCSIM models “closed” populations. 
Individuals may enter and exit the simulation only by (simulated) birth and death. 


SOCSIM has been used extensively in social science research to study, among other things, dynamics of kin availability [5-7], generational overlap [4,8], and kin loss [9-11].

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


## References

1. Hammel, E., Hutchinson, D., Wachter, K., Lundy, R., and Deuel, R. (1976). The SOCSIM Demographic-Sociological Microsimulation Program: Operating Manual. University of California Berkeley. Institute of International Studies.

2. Wachter, K.W. (1997). Kinship resources for the elderly. Philosophical Transactions of the Royal Society of London. Series B: Biological Sciences 352(1363):1811–1817. doi:10.1098/rstb.1997.0166.

3. Mason, Carl. (2016). Socsim Oversimplified. Accesed on 24 Feb 2023 from: https://lab.demog.berkeley.edu/socsim/CurrentDocs/socsimOversimplified.pdf

4. Alburez‐Gutierrez, D., Mason, C., and Zagheni, E. (2021). The “Sandwich Generation” Revisited: Global Demographic Drivers of Care Time Demands. Population and Development Review 47(4):997–1023. doi:10.1111/padr.12436.

5. Hammel, E.A. (2005). Demographic dynamics and kinship in anthropological populations. Proceedings of the National Academy of Sciences 102(6):2248–2253. doi:10.1073/pnas.0409762102.

6. Verdery, A.M. and Margolis, R. (2017). Projections of white and black older adults without living kin in the United States, 2015 to 2060. Proceedings of the National Academy of Sciences 114(42):11109–11114. doi:10.1073/pnas.1710341114.

7. Murphy, M. (2011). Long-Term Effects of the Demographic Transition on Family and Kinship Networks in Britain. Population and Development Review 37:55–80. doi:10.1111/j.1728-4457.2011.00378.x.

8. Margolis, R. and Verdery, A.M. (2019). A Cohort Perspective on the Demography of Grandparenthood: Past, Present, and Future Changes in Race and Sex Disparities in the United States. Demography 56(4):1495–1518. doi:10.1007/s13524-019-00795-1.

9. Zagheni, E. (2011). The Impact of the HIV/AIDS Epidemic on Kinship Resources for Orphans in Zimbabwe. Population and Development Review 37(4):761–783. doi:10.1111/j.1728-4457.2011.00456.x.

10. Verdery, A.M., Smith-Greenaway, E., Margolis, R., and Daw, J. (2020). Tracking the reach of COVID-19 kin loss with a bereavement multiplier applied to the United States. Proceedings of the National Academy of Sciences 117(30):17695. doi:10.1073/pnas.2007476117.

11. Snyder, M., Alburez-Gutierrez, D., Williams, I., and Zagheni, E. (2022). Estimates from 31 countries show the significant impact of COVID-19 excess mortality on the incidence of family bereavement. Proceedings of the National Academy of Sciences 119(26):e2202686119. doi:10.1073/pnas.2202686119.
