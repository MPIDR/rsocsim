# Run a single Socsim simulation with a given supervisory file and directory

Run a single Socsim simulation with a given supervisory file and
directory

## Usage

``` r
socsim(
  folder,
  supfile,
  seed = "42",
  process_method = "inprocess",
  compatibility_mode = "1",
  suffix = ""
)
```

## Arguments

- folder:

  A string. This is the base directory of the simulation. Every .sup and
  rate file should be named relative to this directory.

- supfile:

  A string. The name of the .sup file to start the simulation, relative
  to the directory.

- seed:

  A string. The seed for the RNG, so expects an integer. Defaults to
  "42".

- process_method:

  A string. Whether and how SOCSIM should be started in its own process
  or in the running R process. Defaults to "inprocess". Use one of:

  - "future" - the safest option. A new process will be started via the
    "future" package

  - "inprocess" - SOCSIM runs in the R-process. Beware if you run
    several different simulations, they may affect later simulations.

  - "clustercall" - if the future package is not available, try this
    method instead.

- compatibility_mode:

  A string.

- suffix:

  A string.

## Value

Returns the name of the directory to which the results will be written.
