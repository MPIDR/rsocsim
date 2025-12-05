# SOCSIM Simplified for LLMs

## Overview
SOCSIM is an open-source demographic microsimulation program. It simulates core demographic events (birth, death, marriage, divorce) and social group transitions for individuals within a population. It uses a competing risk model where each individual has a scheduled "next event".

## 1. Running SOCSIM via R (`rsocsim`)

The `rsocsim` package provides a wrapper to run simulations and analyze results.

### Key Functions
*   **`socsim(folder, supfile, seed, process_method, ...)`**: The main entry point.
    *   `folder`: Base directory for simulation files.
    *   `supfile`: The supervisory configuration file (e.g., `sim.sup`).
    *   `seed`: Random number seed (integer).
    *   `process_method`: `"inprocess"` (runs in R) or `"future"` (runs in separate process).
*   **`create_initial_population(folder, size_opop, ...)`**: Generates a basic initial population (`.opop`) and empty marriage file (`.omar`) to bootstrap a simulation.
*   **`download_rates(folder, countrycode, ...)`**: Downloads standardized fertility and mortality rate files (e.g., from UN WPP) for a given country and period.

## 2. Input Files

SOCSIM requires three main types of input:

### A. Supervisory File (`.sup`)
The configuration file controlling the simulation. It defines global parameters and simulation "segments" (periods with specific rates).

**Global Directives:**
*   `segments <int>`: Number of simulation segments.
*   `input_file <stem>`: Base name for initial `.opop` and `.omar` files.
*   `output_file <stem>`: Base name for output files.
*   `hetfert <0/1>`: Enable heterogeneous fertility (fertility multipliers).
*   `bint <int>`: Minimum birth interval in months (default 9).
*   `marriage_queues <1/2>`: Marriage market system (1 = one-queue, 2 = two-queue).
*   `marriage_eval <preference/distribution>`: Method for evaluating potential spouses.
*   `child_inherits_group <rule>`: How newborns are assigned groups (e.g., `from_mother`).

**Segment Directives:**
*   `duration <months>`: Length of the segment.
*   `include <filename>`: Load rate files.
*   `run`: Execute the current segment.

### B. Rate Files
Define probabilities for demographic events. Rates are specified in blocks identified by keywords:
`event group sex marital_status [parity] [dest_group]`

**Format:**
```
death 1 M single
0   1   0.046  # Age 0-1 month: prob 0.046
0   12  0.005  # Age 1-12 months: prob 0.005
...
```
*   **Events**: `birth`, `death`, `marriage`, `divorce`, `transit`.
*   **Defaults**: Missing rates often default to simpler categories (e.g., widowed defaults to divorced, parity N defaults to parity N-1).

### C. Initial Population (`.opop`) & Marriage (`.omar`)
*   **.opop**: A space-delimited file with 14 columns describing each individual.
*   **.omar**: A space-delimited file with 8 columns describing marriages.

## 3. Simulation Mechanics (C++ Core)

### Event Scheduling
*   **Competing Risks**: Every living person has exactly *one* next scheduled event.
*   **Waiting Times**: Generated using piecewise exponential distributions based on current rates.
*   **Execution**: Events are executed month-by-month. Executing an event (or changing status) triggers rescheduling of the next event.

### Marriage Market
*   **Two-Queue System** (`marriage_queues 2`): Both males and females initiate marriage searches. If no match is found immediately, they enter a queue.
*   **One-Queue System** (`marriage_queues 1`): Only females initiate searches. They scan *all* eligible males immediately. Better for achieving specific female marriage rates.
*   **Scoring (`score3`)**: Potential spouses are evaluated based on age difference preferences (`preference`) or to match a target age-difference distribution (`distribution`).

### Groups & Transitions
*   Individuals belong to a "group" (1..60).
*   Group membership affects vital rates.
*   Transitions between groups are modeled as events (`transit`).

## 4. Output & Analysis

### Output Files
*   **`result.opop`**: Final population state (includes dead individuals).
*   **`result.omar`**: Complete marriage history.
*   **`result.otx`**: Transition history (if transitions occurred).

### Analysis Functions (R)
*   **`read_opop(folder, ...)`**: Reads `.opop` into a data frame.
    *   *Columns*: `pid`, `fem` (sex), `group`, `nev` (next event), `dob`, `mom`, `pop`, `nesibm` (next sib mom), `nesibp` (next sib dad), `lborn` (last born), `marid` (last marriage), `mstat`, `dod`, `fmult`.
*   **`read_omar(folder, ...)`**: Reads `.omar` into a data frame.
    *   *Columns*: `mid`, `wpid` (wife), `hpid` (husband), `dstart`, `dend`, `rend` (reason end), `wprior` (wife's prior mar), `hprior` (husband's prior mar).
*   **`retrieve_kin(opop, omar, pid, ...)`**: Reconstructs kinship networks (parents, grandparents, siblings, cousins, etc.) for specified individuals by traversing the linked lists in `.opop` and `.omar`.
*   **`estimate_mortality_rates(opop, ...)`**: Calculates realized age-specific mortality rates from the simulation output for validation.

## 5. Data Structures (Internal & File)

### Person Struct / .opop Columns
1.  `pid`: Person ID (integer).
2.  `sex`: 0=Male, 1=Female.
3.  `group`: Group ID.
4.  `next_event`: Code for next scheduled event.
5.  `birthdate`: Month of birth.
6.  `mother`: Mother's PID.
7.  `father`: Father's PID.
8.  `e_sib_mom`: Next eldest sibling (maternal).
9.  `e_sib_dad`: Next eldest sibling (paternal).
10. `lborn`: Last born child's PID.
11. `last_marriage`: ID of most recent marriage.
12. `mstatus`: Marital status (1=Single, 2=Divorced, 3=Widowed, 4=Married).
13. `deathdate`: Month of death (0 if alive).
14. `fmult`: Fertility multiplier (heterogeneity factor).

### Marriage Struct / .omar Columns
1.  `mid`: Marriage ID.
2.  `wpid`: Wife's PID.
3.  `hpid`: Husband's PID.
4.  `dstart`: Start month.
5.  `dend`: End month (0 if ongoing).
6.  `rend`: Reason for end (2=Divorce, 3=Death).
7.  `wprior`: Wife's previous marriage ID.
8.  `hprior`: Husband's previous marriage ID.
