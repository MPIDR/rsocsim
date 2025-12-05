# Read output marriage file into a data frame

|     |        |                                                                    |
|-----|--------|--------------------------------------------------------------------|
| 1   | mid    | Marriage id number (unique sequential integer)                     |
| 2   | wpid   | Wife’s person id                                                   |
| 3   | hpid   | Husband’s person id                                                |
| 4   | dstart | Date marriage began                                                |
| 5   | dend   | Date marriage ended or zero if still in force at end of simulation |
| 6   | rend   | Reason marriage ended 2 = divorce; 3 = death of one partner        |
| 7   | wprior | Marriage id of wife’s next most recent prior marriage              |
| 8   | hprior | Marriage id of husband’s next most recent prior marriage           |

## Usage

``` r
read_omar(
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

you can either provide the complete `path` to the file or the `folder`,
supfilename, seed and suffix with which you started the simulation
