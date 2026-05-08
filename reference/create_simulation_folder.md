# Create a directory structure for the simulation

Create a two-level directory structure. If the first-level argument is
NULL, we look for and, if needed, create the directory 'socsim' in the
current temporary directory. If the second-level argument is NULL, we
create a directory named like 'socsim_sim\_' followed by a random
component in the first-level directory.

## Usage

``` r
create_simulation_folder(basedir = NULL, simdir = NULL)
```

## Arguments

- basedir:

  A string. Optional. First-level directory where the
  simulation-specific directory will be created. Defaults to
  `file.path(tempdir(), "socsim")`.

- simdir:

  A string. Optional. Simulation-specific directory which will be
  created within 'basedir'. Defaults to 'socsim_sim\_' plus a random
  component created with
  [`tempfile()`](https://rdrr.io/r/base/tempfile.html).

## Value

A string. The full path to the simulation-specific directory.
