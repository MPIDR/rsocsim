# Run a single socsim-simulation with a socsim binary

Run a single socsim-simulation with a socsim binary

## Usage

``` r
run1simulationwithfile_from_binary(
  folder,
  supfile,
  seed = "42",
  compatibility_mode = "1",
  socsim_path = NULL
)
```

## Arguments

- folder:

  base-directory of the simulation.

- supfile:

  the .sup file to start the simulation, relative to the folder

- seed:

  RNG seed as string, Default="42"

- compatibility_mode:

  compatibility mode as string, Default="1"

- socsim_path:

  path+filename of a socsim-executable. Download one from
  https://github.com/tomthe/socsim/releases/download/0.3/socsim.exe

## Value

The results will be written into the specified folder
