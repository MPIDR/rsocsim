# rsocsim architecture

## Overview
rsocsim is an R package that wraps the SOCSIM microsimulation engine. The R
layer prepares inputs, runs simulations, and reads outputs. The C/C++ core
parses supervisory files, loads rate tables, schedules events, and advances the
simulation month-by-month.

## High-level flow
1. R user code calls `socsim()`.
2. `socsim()` switches to the simulation folder and calls `run_sim_w_file()`.
3. `run_sim_w_file()` starts the core engine via `startSocsimWithFile()` (Rcpp
   export), optionally in a separate process.
4. The core engine (`main1()` in C++) reads the .sup file, loads rate sets,
   initializes queues, and runs segments.
5. Outputs are written to result files (`.opop`, `.omar`, `.otx`, `.pyr`) and a
   log file.
6. R helper functions (e.g., `read_opop()`, `read_omar()`) load outputs for
   analysis. Higher-level helpers compute fertility/mortality rates and kinship.

## Layers and modules
### R layer (package API)
- **Simulation orchestration**: `socsim()`, `run_sim_w_file()` in
  [R/startsocsim.R](R/startsocsim.R).
- **Simulation setup**: `create_simulation_folder()`, `create_sup_file()`.
- **Input preparation**: `create_initial_population()` in
  [R/prepare_initial_population.R](R/prepare_initial_population.R).
- **Output readers**: `read_opop()`, `read_omar()` in
  [R/read_results.R](R/read_results.R).
- **Analysis utilities**: fertility/mortality estimation and kin retrieval in
  [R/estimate_rates.R](R/estimate_rates.R) and
  [R/retrieve_kin.R](R/retrieve_kin.R).
- **External data**: `download_rates()` fetches rate files from a remote API.

### C/C++ core (simulation engine)
- **Entry point**: `main1()` in [src/src/events.cpp](src/src/events.cpp). It
  sets defaults, creates output directories, opens input files, and runs the
  event loop per segment.
- **Supervisory file parsing**: `load()` and `l_process_line()` in
  [src/src/load.cpp](src/src/load.cpp). These parse directives, build rate
  tables, and handle `include` directives.
- **Event scheduling and execution**: `process_month()`, `date_and_event()`,
  `new_events_for_all()` in [src/src/events.cpp](src/src/events.cpp).
- **Rate tables and defaults**: `fill_rate_gaps()` and related helpers in
  [src/src/load.cpp](src/src/load.cpp).
- **I/O and utilities**: `io.c`, `random.c`, `utils.c`, `xevents.c` (included
  from `events.cpp`). These implement file I/O, RNG, and auxiliary logic.

## Data and file formats
- **Supervisory file (.sup)**: Top-level configuration and segment directives
  (e.g., `segments`, `duration`, `include`, `run`). Parsed by `load()`.
- **Rate files**: Plain-text fertility/mortality rates referenced via
  `include` directives in the .sup file.
- **Population file (.opop)**: One row per person with IDs, parents, dates,
  marital status, and fertility multiplier.
- **Marriage file (.omar)**: One row per marriage with spouse IDs and dates.
- **Transition history (.otx)**: Group transition records.
- **Population pyramid (.pyr)**: Aggregated age/sex distributions.
- **Log file**: Execution trace written during simulation.

## Execution model
- The core uses an event queue indexed by month. Each living person has exactly
  one scheduled next event (birth, death, marriage, transition).
- Each segment defines a duration (in months). When `run` is encountered in the
  .sup file, the engine executes one segment and then continues parsing.
- Random waiting times for events are derived from rate tables (competing risks
  model). The earliest event wins and is scheduled.

## Key assumptions and constraints
- Input file stems (e.g., `input_file init_new`) map to `.opop` and `.omar`
  files in the simulation directory.
- Output is written to a simulation-specific subfolder created at runtime.
- Rate files must match SOCSIM formatting rules to avoid parse errors.

## External dependencies
- **R** packages: `Rcpp`, `dplyr`, `tidyr`, `magrittr`, `future` (optional).
- **System**: File I/O and, optionally, separate-process execution for
  simulations (via `future` or `parallel`).

## Testing strategy
- **Unit tests (R)**: Validate helpers (folder creation, population creation,
  reading outputs, rate estimation, kin retrieval).
- **Integration tests**: Run a short simulation with bundled rate files and
  verify output folder creation.
- **Negative tests**: Input validation for functions like `download_rates()`.
