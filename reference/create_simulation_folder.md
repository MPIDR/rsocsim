# Create a directory structure for the simulation

Create a two-level directory structure. If the first-level argument is
NULL, we look for and, if needed, created the directory 'socsim' in the
user's home directory. If the second-level argument is NULL, we create a
directory named 'socsim_sim_some random component' in the first-level
directory.

## Usage

``` r
create_simulation_folder(basedir = NULL, simdir = NULL)
```

## Arguments

- basedir:

  A string. Optional. First-level directory where the
  simulation-specific directory will be created. Defaults to
  '\$HOME/socsim'.

- simdir:

  A string. Optional. Simulation-specific directory which will be
  created within 'basedir'. Defaults to 'socsim_sim\_' plus a random
  component created with
  [`tempfile()`](https://rdrr.io/r/base/tempfile.html).

## Value

A string. The full path to the simulation-specific directory.
