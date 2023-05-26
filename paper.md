---
title: 'rsocsim: An R package for demographic microsimulations'
tags:
  - R
  - demography
  - microsimulation
  - population dynamics
  - kinship
authors:
  - name: Tom Theile
    orcid: 0000-0002-3957-2474
    affiliation: 1 # (Multiple affiliations must be quoted)
    corresponding: true # (This is how to denote the corresponding author)
  - name: Diego Alburez-Gutierrez
    affiliation: 1
    orcid: 0000-0003-0573-9093
  - name: Liliana P. Calderón-Bernal
    affiliation: 1
    orcid: 0000-0003-0573-9093
  - name: Mallika Snyder
    affiliation: 2
  - name: Emilio Zagheni
    affiliation: 1
    orcid: 0000-0002-7660-8368
affiliations:
 - name: Departement of Digital and Computational Demography, Max Planck Institute for Demographic Research, Rostock, Germany 
   index: 1
 - name: Department of Demography, University of California Berkeley, Berkeley, CA, USA
   index: 2
date: 27 April 2023
bibliography: paper.bib

# Optional fields if submitting to a AAS journal too, see this blog post:
# https://blog.joss.theoj.org/2018/12/a-new-collaboration-with-aas-publishing
# aas-doi: 10.3847/xxxxx <- update this with the DOI from AAS once you know it.
# aas-journal: Astrophysical Journal <- The name of the AAS journal.
---

# Summary

Microsimulations of virtual populations enable a deep look into the effects of different fertility-, mortality and marriage-rates on the structure of kinship networks and individual life events.

Socsim is a microsimulation software developed since 1973 (citation?). Since then, it was used in many publications. It was extended continously to serve the needs of many different demographic studies.

Although R (citation) was often used to analyze the simulation outputs, socsim was a standalone program that only compiled with older C compilers for particular operation systems.
-- The software might have lost relevance with the retirement of old maintainers.
We rewrote the old software and made it usable on other platforms. We put it into an easily installable R-package together with useful utility functions.


# Statement of need - bullet points

* simulation mechanisms
  * input initial population, rates and options
  * output population (every person), marriages, population pyramids,?
  * A person has not many "eigenschaften": Age, Sex, group, fertility_multiplier, single/marriaged/widowed/divorced, mother, father, next sibling
  * Events, scheduling of births, deaths, group transitions, etc. according to rates
  * Different implementations of marriage markets.
* implementation details
  * the main simulation happens in C/C++
  * simulation code had to be modified to be multiplatform. 
    * A new rng had to be used, as the previous was only available on Linux
    * new strategies were implemented, to be able to retrieve kinship networks withouth marriage rates?
    * output to R console
    * raised max age from 100 years to 200 years
    * bugfixes
  * R functions to start a simulation
  * R functions to read and analyze the output of a simulation
  * RCPP is used to compile and run the C/C++ code from R. This eases the distribution of the package, as we do not have to build binaries for every platform, while it is still very easy to install even for inexperienced users of R.
* Socsim has been used widely, but it was difficult without having an account at berkeley. Now it is easy to install and all the source is now licenced under GPL v3

# Statement of Need

Socsim is an open source simulation framework designed to model population dynamics and study various demographic phenomena. It provides a flexible and customizable environment for simulating the evolution of populations over time. This chapter discusses the key features and implementation details of Socsim, highlighting its significance in population studies.

### Simulation Mechanisms

Socsim offers a range of simulation mechanisms to capture population dynamics. Users can input an initial population with specified attributes such as age, sex, group affiliation, fertility multiplier, marital status, and kinship relationships. The simulation allows for the scheduling of events, including births, deaths, group transitions, and marriages, according to user-defined rates. Different implementations of marriage markets are also available, enabling the modeling of diverse marriage systems.

The output of the simulation includes detailed population data, including information on each individual, marriages, and population pyramids. Each person in the simulated population is characterized by essential attributes such as age, sex, group affiliation, fertility multiplier, marital status, parent-child relationships, and sibling relationships. These outputs facilitate the analysis of population dynamics and the exploration of various demographic phenomena.

### Implementation Details

The main simulation engine of Socsim is implemented in C/C++, ensuring efficient and high-performance execution. To enhance its usability and accessibility, several modifications were made to the simulation code. These modifications include:

- **Multiplatform Compatibility**: The code was adapted to be compatible with multiple platforms, ensuring that users can run the simulation on different operating systems. This involved the utilization of a new random number generator (RNG) that is available across various platforms, as the previous RNG was limited to Linux.

- **Enhanced Functionality**: Several new strategies and features were implemented to extend the capabilities of Socsim. For instance, the framework now allows users to retrieve kinship networks without explicitly specifying marriage rates. Bug fixes were also incorporated to improve the overall reliability and accuracy of the simulation.

- **Integration with R**: Socsim provides a set of R functions that facilitate the execution, control, and analysis of simulations. Users can easily initiate a simulation from within R, leveraging the framework's capabilities through a user-friendly interface. Additionally, specialized R functions are available to read and analyze the output generated by Socsim simulations.

- **RCPP Integration**: The integration of RCPP (R and C++ Integration) allows for seamless compilation and execution of the C/C++ code within the R environment. This integration simplifies the distribution of the Socsim package, as users can install it effortlessly, regardless of their platform. Moreover, even users with limited experience in R can easily utilize the framework's functionalities.

### Increased Accessibility and Licensing

Socsim has gained popularity among researchers studying population dynamics. However, previously, it required an account at Berkeley to access and use the framework effectively. To address this limitation and improve accessibility, Socsim has undergone significant changes. Firstly, it is now easy to install, allowing researchers to utilize the framework without any account restrictions. Furthermore, all source code of Socsim has been licensed under the GNU General Public License version 3 (GPL v3), ensuring the software's open source nature and promoting collaboration and innovation in population studies.

## Conclusion

Socsim serves as a valuable open source simulation framework for population dynamics, enabling researchers to model and analyze complex demographic phenomena. Its flexible simulation mechanisms, implementation details, integration with R, and increased accessibility through licensing changes make it a powerful tool for studying population dynamics. By providing an open source platform, Socsim promotes transparency, reproducibility, and collaboration in population research, fostering advancements in the field.













# Citations

Citations to entries in paper.bib should be in
[rMarkdown](http://rmarkdown.rstudio.com/authoring_bibliographies_and_citations.html)
format.

If you want to cite a software repository URL (e.g. something on GitHub without a preferred
citation) then you can do it with the example BibTeX entry below for @fidgit.

For a quick reference, the following citation commands can be used:
- `@author:2001`  ->  "Author et al. (2001)"
- `[@author:2001]` -> "(Author et al., 2001)"
- `[@author1:2001; @author2:2001]` -> "(Author1 et al., 2001; Author2 et al., 2002)"

# Figures

Figures can be included like this:
![Caption for example figure.\label{fig:example}](figure.png)
and referenced from text using \autoref{fig:example}.

Figure sizes can be customized by adding an optional second parameter:
![Caption for example figure.](figure.png){ width=20% }

# Acknowledgements

We acknowledge the immense work that went into socsim since its first version in 1973.
Ken Wachter
Carl Mason
Emilio Zagheni
?

# References
