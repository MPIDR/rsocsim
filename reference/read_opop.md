# Read output population file into a data frame

after the end of the simulation, socsim writes every person of the
simulation into a file called result.opop \|

## Usage

``` r
read_opop(
  folder = NULL,
  supfile = "socsim.sup",
  seed = 42,
  suffix = "",
  fn = NULL
)
```

## Arguments

- folder:

  simulation base folder ("~/socsim/simulation_235/")

- supfile:

  name of supervisory-file ("socsim.sup")

- seed:

  random number seed (42)

- suffix:

  optional suffix for the results-directory (default="")

- fn:

  complete path to the file. If not provided, it will be created from
  the other arguments

## Value

Data frame with the information of the output population file

## Details

|     |        |                                                                                       |
|-----|--------|---------------------------------------------------------------------------------------|
| 1   | pid    | Person id unique identifier assigned as integer in birth order                        |
| 2   | fem    | 1 if female 0 if male                                                                 |
| 3   | group  | Group identifier 1..60 current group membership of individual                         |
| 4   | nev    | Next scheduled event                                                                  |
| 5   | dob    | Date of birth integer month number                                                    |
| 6   | mom    | Person id of mother                                                                   |
| 7   | pop    | Person id of father                                                                   |
| 8   | nesibm | Person id of next eldest sibling through mother                                       |
| 9   | nesibp | Person id of next eldest sibling through father                                       |
| 10  | lborn  | Person id of last born child                                                          |
| 11  | marid  | Id of marriage in .omar file                                                          |
| 12  | mstat  | Marital status at end of simulation integer 1=single;2=divorced; 3=widowed; 4=married |
| 13  | dod    | Date of death or 0 if alive at end of simulation                                      |
| 14  | fmult  | Fertility multiplier                                                                  |

This table explains the columns of the opop file and the columns of the
output data frame. You can either provide the complete `path` to the
file or the `folder`, supfilename, seed and suffix with which you
started the simulation
