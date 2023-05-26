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
We rewrote the old C/C++ code for compatibility with modern compilers, the Windows operating system and R. We added R functions that make it easy to get started with a simulation and read the created synthetic populations into R data structures.




# Statement of need

* 
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

rsocsim 

`Gala` is an Astropy-affiliated Python package for galactic dynamics. Python
enables wrapping low-level languages (e.g., C) for speed without losing
flexibility or ease-of-use in the user-interface. The API for `Gala` was
designed to provide a class-based and user-friendly interface to fast (C or
Cython-optimized) implementations of common operations such as gravitational
potential and force evaluation, orbit integration, dynamical transformations,
and chaos indicators for nonlinear dynamics. `Gala` also relies heavily on and
interfaces well with the implementations of physical units and astronomical
coordinate systems in the `Astropy` package [@astropy] (`astropy.units` and
`astropy.coordinates`).

`Gala` was designed to be used by both astronomical researchers and by
students in courses on gravitational dynamics or astronomy. It has already been
used in a number of scientific publications [@Pearson:2017] and has also been
used in graduate courses on Galactic dynamics to, e.g., provide interactive
visualizations of textbook material [@Binney:2008]. The combination of speed,
design, and support for Astropy functionality in `Gala` will enable exciting
scientific explorations of forthcoming data releases from the *Gaia* mission
[@gaia] by students and experts alike.

# Mathematics

Single dollars ($) are required for inline mathematics e.g. $f(x) = e^{\pi/x}$

Double dollars make self-standing equations:

$$\Theta(x) = \left\{\begin{array}{l}
0\textrm{ if } x < 0\cr
1\textrm{ else}
\end{array}\right.$$

You can also use plain \LaTeX for equations
\begin{equation}\label{eq:fourier}
\hat f(\omega) = \int_{-\infty}^{\infty} f(x) e^{i\omega x} dx
\end{equation}
and refer to \autoref{eq:fourier} from text.

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
