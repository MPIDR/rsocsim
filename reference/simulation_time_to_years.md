# Calculate for how many years the simulation ran

Calculate for how many years the simulation ran

## Usage

``` r
simulation_time_to_years(simulation_time, pre_simulation_time, start_year)
```

## Arguments

- simulation_time:

  An integer. The number of periods (months) the simulation ran.

- pre_simulation_time:

  An integer. The number of periods (months) the simulation ran before
  getting to a stable population. This is subtracted from
  'simulation_time' in order to arrive at the "real" simulation time

- start_year:

  An integer. The year the simulation started.

## Value

A numeric scalar giving the calendar year reached at the end of the
simulated period after subtracting `pre_simulation_time / 12`. The value
can include a fractional year.
