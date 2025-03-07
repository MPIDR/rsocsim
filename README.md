# rsocsim - Socsim R package

<img align="left" width="144" alt="2023-MPIDR_rsocsim-web-144" src="https://user-images.githubusercontent.com/5939967/221559559-1233e070-0e0a-43a1-b451-a3215c0b7d4c.png">

Socsim ('Social Simulator') is an open source stochastic microsimulation platform used to produce synthetic populations with plausible kinship structures using demographic rates as input.
We stand on the shoulders of giants: Socsim was originally developed for Unix at UC Berkeley [1-2], where it has been mantained for decades. The current release of `rsocsim` aims to be OS-agnostic and, for the most part, back-compatible with the original Socsim distribution (https://lab.demog.berkeley.edu/socsim/).

`rsocsim` is still under development. You might find some bugs or unexpected behavior, the API might change without warnings in the next weeks.
If you encounter an error or bug, we are happy to hear from you in the issues.

## Installation

### From a binary package

Download the correct binary package from the [release section on Github](https://github.com/MPIDR/rsocsim/releases):
Currently, you can use [rsocsim_1.5.9.tgz](https://github.com/MPIDR/rsocsim/releases/download/v1.5.9/rsocsim_1.5.9.tgz) for Mac and [rsocsim_1.5.9.tar.gz](https://github.com/MPIDR/rsocsim/releases/download/v1.5.9/rsocsim_1.5.9.tar.gz)
for Windows.

Then execute the following line in R and choose the downloaded file:
```
install.packages(file.choose(),repos=NULL,type="binary")
```

### From Source (requires a compiler)

To get the latest version, you need to have a Rcpp compatible compiler installed
and a way to install packages from Github.

Install a Rcpp compatible compiler:

- on Windows: [RTools](https://cran.r-project.org/bin/windows/Rtools/index.html);
- on Mac: `xcode-select --install`;
- on Linux: `sudo apt-get install r-base-dev`.
 
For more help, see [Rcpp for everyone](https://teuder.github.io/rcpp4everyone_en/020_install.html).

In order to install packages from Github, you can use the {remotes} or
{devtools} packages. {remotes} ([CRAN link](https://cran.r-project.org/web/packages/remotes/index.html))
only provides the capability to install packages from different code
repositories. {devtools} ([CRAN link](https://cran.r-project.org/web/packages/devtools/index.html))
includes many more capabilities needed in package development (e.g.
(re-)generating documentation or running tests). {devtools} includes {remotes}.


Install {remotes} or {devtools}:
````R
install.packages("remotes")
````

Install rsocsim from Github:
````R
# The command is the same in {devtools} and {remotes}.
remotes::install_github("MPIDR/rsocsim@v1.5.9")
````

## Use rsocsim


### Getting started

````R
library("rsocsim")

# create a new folder for all the files related to a simulation.
# this will be in your home- or user-directory:
folder = rsocsim::create_simulation_folder()

# create a new supervisory-file.
# supervisory-files tell socsim what to simulate. 
# create_sup_file will create a very basic supervisory file.
# It also copies some rate-files that will be needed into the 
# simulation folder:
supfile = rsocsim::create_sup_file(folder)

# Choose a random-number seed:
seed = 300

# Start the simulation:
rsocsim::socsim(folder,supfile,seed)

````

The results will be saved into a subfolder of the  simulation-`folder´.
For more sophisticated simulations you will need to edit the supervisory
file and provide rate files for fertility, mortality and marriage rates.

If you run more than one simulation, previous simulations may corrupt
later simulations. Use the option `process_method = "future"` to prevent this:
```
rsocsim::socsim(folder,supfile,seed,process_method = "future")
```

### Tutorial

We created a practical workshop "Demographic microsimulations in R using SOCSIM: Modelling population and kinship dynamics" which you can access here:
https://github.com/alburezg/rsocsim_workshop_paa

## Background and applications

The following description of SOCSIM was adapted from the Supplementary Materials of [4]. 
SOCSIM is an open source and extensible demographic microsimulation program [1,3]. 
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
### Style guide

The goal of this section is to improve the readability of the code and reduce complaints from modern compilers (parts of the code are 40ish years old). We (try to) follow Allman style, but as a guideline not a hard requirement. Basically, this means:

- one level of indentation = 4 spaces;
- spaces not tabs;
- braces after a function declaration or a control flow statement go on a new line;
- first-level braces at the beginning of a line appear in the first column;
- closing braces appear in the same column as their counterpart.
- spaces...:
    - ... around binary operators;
    - ... after commas;
    - ... before opening parentheses and after closing parentheses.

If you use vim, the default C formatting should be fine (':h C-indenting').

We reformatted the .cpp files because indentation was all over the place, but we left the .c files largely alone such as not to anger the compiler gods.

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
