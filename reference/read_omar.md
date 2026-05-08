# Read output marriage file into a data frame

When `fn` contains multiple file paths, or when `seed` contains multiple
values and `fn` is `NULL`, the matching result files are read and
row-bound into a single data frame. To keep identifiers unique across
simulations, positive ID columns are offset by
`(index - 1) * id_offset`, while sentinel zeros remain unchanged.

## Usage

``` r
read_omar(
  folder = NULL,
  supfile = "socsim.sup",
  seed = 42,
  suffix = "",
  fn = NULL,
  id_offset = 10000000L,
  quiet = FALSE
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

- id_offset:

  positive integer stride used to offset IDs when combining multiple
  files. Ignored for single-file reads. Default is 10 million, which
  allows combining up to 214 files with a total population of 10 million
  each.

- quiet:

  logical. If `FALSE`, emit a message with the file path being read.

## Value

A data frame with columns `mid`, `wpid`, `hpid`, `dstart`, `dend`,
`rend`, `wprior`, and `hprior`, matching the SOCSIM `result.omar` file.
If the file is missing or empty, a zero-row data frame with these
columns is returned.

## Details

|  |  |  |
|----|----|----|
| 1 | mid | Marriage id number (unique sequential integer) |
| 2 | wpid | Wife’s person id |
| 3 | hpid | Husband’s person id |
| 4 | dstart | Date marriage began |
| 5 | dend | Date marriage ended or zero if still in force at end of simulation |
| 6 | rend | Reason marriage ended 2 = divorce; 3 = death of one partner |
| 7 | wprior | Marriage id of wife’s next most recent prior marriage |
| 8 | hprior | Marriage id of husband’s next most recent prior marriage |

you can either provide the complete `path` to the file or the `folder`,
supfilename, seed and suffix with which you started the simulation
