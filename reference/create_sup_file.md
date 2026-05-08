# Create a basic .sup file for a simulation

The simulation is only a simple one. The file will be saved into the
directory 'simdir'.

## Usage

``` r
create_sup_file(simdir, simname = "socsim")
```

## Arguments

- simdir:

  A string. The directory where the .sup file will be saved.

- simname:

  A string. The base name of the simulation. Defaults to `"socsim"`.

## Value

A string. The basename of the created supervisory file, for example
`"socsim.sup"`. The file is written to `simdir`, and the function also
copies the bundled rate and initial-population input files into
`simdir`.
