# Identify members of a kin network for an individual or individuals of interest.

Identify members of a kin network for an individual or individuals of
interest.

## Usage

``` r
retrieve_kin(
  opop = opop,
  omar = omar,
  KidsOf = KidsOf,
  pid,
  extra_kintypes,
  kin_by_sex
)
```

## Arguments

- opop:

  An R object from SOCSIM microsimulation output (population file).
  Create this object with the function read_opop().

- omar:

  An R object from SOCSIM microsimulation output (marriage file). Create
  this object with the function read_omar().

- KidsOf:

  A list object containing the children of each person in the
  population. This object is created by the function getKidsOf().

- pid:

  A character vector of person IDs, indicating persons of interest for
  whom these kin networks should be identified.

- extra_kintypes:

  A vector of character values indicating which additional types of kin
  should be obtained. For reasons of computational efficiency, the
  function will by default only identify an individual's
  great-grandparents ("ggparents" in function output), grandparents
  ("gparents"), parents, siblings, spouse, children, and grandchildren
  ("gchildren"). However, by selecting one or more of the following kin
  types, the kin network generated will also include these individuals:

  - "gunclesaunts": Great-uncles and great-aunts

  - "unclesaunts": Uncles and aunts

  - "firstcousins": First cousins (Children of uncles and aunts)

  - "niblings": Nieces and nephews (Children of siblings)

  - "inlaws": Parents-in-law (parents of spouse) and brothers and
    sisters in law (siblings of spouse and spouse of siblings)

- kin_by_sex:

  A logical value indicating whether output should include kin relations
  additionally disaggregated by the sex of the relative. Setting this
  value to TRUE will result in additional objects being generated to
  identify individuals' relatives by sex.

## Value

An R object containing a list of lists with person IDs of kin, organized
by relationship. These person ID values will be named based on the
person of interest with whom they are associated. For example, for a
list named "parents", the values will be person IDs of the parents of
individuals of interest. These values will be named according to their
children's IDs (given that their children are, in this case, the persons
of interest provided to the function input). With kin_by_sex set to TRUE
and extra_kintypes set to c(c("gunclesaunts", "unclesaunts",
"firstcousins", "niblings", "inlaws")), the full list of kin relations
identified are:

- "ggparents": great-grandparents

- "ggmothers": great-grandmothers

- "ggfathers": great-grandfathers

- "gparents": grandparents

- "gmothers": grandmothers

- "gfathers": grandfathers

- "gunclesaunts": great-uncles and great-aunts

- "guncles": great-uncles

- "gaunts": great-aunts

- "parents": parents

- "mother": mother

- "father": father

- "unclesaunts": uncles and aunts (siblings of parents)

- "uncles": uncles

- "aunts": aunts

- "siblings": siblings

- "sisters": sisters

- "brothers": brothers

- "firstcousins": first cousins

- "firstcousinsfemale": female first cousins

- "firstcousinsmale": male first cousins

- "children": children

- "daughters": daughters

- "sons": sons

- "gchildren": grandchildren

- "gdaughters": granddaughters

- "gsons": grandsons

- "niblings": nephews and nieces

- "nieces": nieces

- "nephews": nephews

- "spouse": spouse (based on final marriage, in the case of multiple
  marriages)

- "parentsinlaw": parents-in-law

- "motherinlaw": mother-in-law

- "fatherinlaw": father-in-law

- "siblingsinlaw": brothers and sisters in law

- "sistersinlaw": sisters-in-law

- "brothersinlaw": brothers-in-law

## Examples

``` r
if (FALSE) { # \dontrun{
#Individuals of interest
pid <- c("10111", "10211", "10311")
#Obtain partial kinship network, with omar and opop already in R environment
kin_network <- getKin(opop = opop, omar = omar, pid = pid, 
extra_kintypes = c("unclesaunts", "niblings"), kin_by_sex = TRUE)
} # }
```
