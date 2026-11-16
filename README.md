# Sudoku CSP Solver

A Constraint Satisfaction Problem (CSP) based Sudoku solver written in C++20, supporting both standard Sudoku and irregular (Jigsaw) Sudoku.

The solver combines constraint propagation, heuristic-based variable selection, and backtracking search to efficiently solve 9×9 Sudoku puzzles.

## Features

- **CSP Modeling** — Models Sudoku as a binary Constraint Satisfaction Problem.
- **Irregular Sudoku Support** — Supports custom region configurations through an external Region Map.
- **Constraint Graph** — Builds an adjacency-list representation of cell constraints.
- **MRV (Minimum Remaining Values)** — Selects the variable with the smallest remaining domain.
- **Degree Heuristic** — Prioritizes variables with the highest number of unassigned neighbors when MRV produces a tie.
- **LCV (Least Constraining Value)** — Prioritizes values that eliminate the fewest possibilities from neighboring variables.
- **Forward Checking** — Removes inconsistent values from neighboring domains after an assignment.
- **AC-3 (Arc Consistency Algorithm 3)** — Performs constraint propagation to reduce domains and detect contradictions.
- **Backtracking Search** — Explores candidate assignments with automatic domain restoration.
- **Bitmask Domain Representation** — Uses compact bitmasks for efficient domain operations.
- **Command-Line Interface** — Supports input files, region maps, output files, and solver statistics.
- **Input Validation** — Validates puzzle values, region sizes, and region connectivity.

## Algorithm Overview

The solver models Sudoku as a Binary CSP.

### 1. Variables

A standard 9×9 Sudoku contains 81 variables:

$$
X = \{X_{0,0}, \, X_{0,1}, \, \ldots, \, X_{8,8} \}
$$

Each variable represents one cell in the Sudoku grid.

### 2. Domains

Each unassigned cell initially has the domain:

$$
D(X_{r,c}) = \{1, \, 2, \, \ldots, \, 9 \}
$$

Pre-filled cells have singleton domains containing their given values.

### 3. Constraints

Two cells must contain different values if they satisfy at least one of the following conditions:

- They belong to the same row.
- They belong to the same column.
- They belong to the same region.

Therefore, the binary constraint is:

$$
X_i \neq X_j
$$

### 4. Constraint Graph

Each Sudoku cell is represented as a node.

An undirected edge connects two nodes whenever their corresponding cells share a constraint.

The graph is stored using an adjacency-list representation.

### 5. Search and Propagation

The solver uses the following techniques:

| Technique        | Purpose                                                       |
|---               |---                                                            |
| MRV              | Select the variable with the smallest domain                  |
| Degree Heuristic | Break MRV ties by selecting the most constrained variable     |
| LCV              | Order candidate values by their impact on neighboring domains |
| Forward Checking | Eliminate assigned values from neighboring domains            |
| AC-3             | Enforce binary arc consistency                                |
| Backtracking     | Explore assignments and recover from contradictions           |

The search terminates when all variables have singleton domains or when no valid assignment exists.

**Note:** AC-3 operates on pairwise inequality constraints. It does not implement generalized arc consistency for global AllDifferent constraints.

## Project Structure

```text
SudokuCSP/
│
├── CMakeLists.txt
├── README.md
├── main.cpp
├── .gitignore
│
├── include/
│   └── Sudoku.hpp
│
└── data/
    ├── puzzle.txt
    └── regions.txt
```

## Requirements

- C++20 compatible compiler
- CMake 3.20 or later
- Git
- CLI11 v2.5.0

### Dependencies

[CLI11](https://github.com/CLIUtils/CLI11) is used for command-line argument parsing.

The dependency is automatically downloaded and configured through CMake FetchContent.

## Build Instructions

Clone the repository:

```bash
git clone https://github.com/Rafael-l-m/Sudoku-CSP-Solver.git
```

Configure the project:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
```

Build:

```bash
cmake --build build -j
```

The executable will be generated in the build directory.

Linux / macOS:

```bash
./build/sudoku
```

Windows:

```powershell
.\build\Release\sudoku.exe
```

## Input File Format

The solver requires two input files:

1. Sudoku Puzzle
2. Region Map

Both files must contain exactly 81 digits representing a 9×9 grid.

Whitespace and line breaks are ignored.

### Sudoku Puzzle

The puzzle uses the following notation:

- `1`–`9`: Pre-filled cells.
- `0`: Empty cells.

Example (`puzzle.txt`):

```text
530070000
600195000
098000060
800060003
400803001
700020006
060000280
000419005
000080079
```

### Region Map

The Region Map defines the region membership of each cell.

Each digit represents a region ID from `1` to `9`.

Every region must:

- Contain exactly 9 cells.
- Be orthogonally connected.

Example (`regions.txt`) for standard Sudoku:

```text
111222333
111222333
111222333
444555666
444555666
444555666
777888999
777888999
777888999
```

For irregular Sudoku, replace the Region Map with a valid custom configuration.

The solver does not require any changes to its internal algorithms when the region layout changes.

## Usage

### Basic Execution

```bash
./build/sudoku \
    --input data/puzzle.txt \
    --regions data/regions.txt
```

### Display Solver Statistics

```bash
./build/sudoku \
    --input data/puzzle.txt \
    --regions data/regions.txt \
    --stats
```

### Save the Solution

```bash
./build/sudoku \
    --input data/puzzle.txt \
    --regions data/regions.txt \
    --output solution.txt
```

### Save the Solution and Display Statistics

```bash
./build/sudoku \
    --input data/puzzle.txt \
    --regions data/regions.txt \
    --output solution.txt \
    --stats
```

### Display Help

```bash
./build/sudoku --help
```

## Command-Line Arguments

| Argument        | Description                      | Required |
|---              |---                               |---       |
| `-i, --input`   | Path to the Sudoku puzzle file   | Yes      |
| `-r, --regions` | Path to the Region Map file      | Yes      |
| `-o, --output`  | Path to the solution output file | No       |
| `-s, --stats`   | Display solver statistics        | No       |
| `-h, --help`    | Display command-line help        | No       |

## Output Example

A successfully solved puzzle is displayed as a formatted 9×9 grid.

```text
+-------+-------+-------+
| 5 3 4 | 6 7 8 | 9 1 2 |
| 6 7 2 | 1 9 5 | 3 4 8 |
| 1 9 8 | 3 4 2 | 5 6 7 |
+-------+-------+-------+
| 8 5 9 | 7 6 1 | 4 2 3 |
| 4 2 6 | 8 5 3 | 7 9 1 |
| 7 1 3 | 9 2 4 | 8 5 6 |
+-------+-------+-------+
| 9 6 1 | 5 3 7 | 2 8 4 |
| 2 8 7 | 4 1 9 | 6 3 5 |
| 3 4 5 | 2 8 6 | 1 7 9 |
+-------+-------+-------+
```

### Solver Statistics

When `--stats` is enabled, the solver reports:

| Metric | Description |
|---|---|
| Search Nodes | Number of recursive search nodes visited |
| Backtracks | Number of failed search branches |
| AC-3 Revisions | Number of arc revision operations |
| Values Removed | Total number of domain values eliminated |

These metrics can be used to evaluate the effectiveness of different constraint propagation and heuristic strategies.

## Exit Codes

The program uses the following exit codes:

| Code | Meaning                    | Description                                                       |
|---   |---                         |---                                                                |
| `0`  | Success                    | The Sudoku was solved successfully.                               |
| `1`  | No Solution                | No valid Sudoku solution exists.                                  |
| `2`  | CLI Argument Error         | Invalid or missing command-line arguments.                        |
| `3`  | File I/O Error             | Failed to read an input file or write the output file.            |
| `4`  | Invalid Sudoku Data        | Invalid puzzle values, malformed input, or an invalid Region Map. |
| `5`  | Internal Computation Error | An unexpected internal computation error occurred.                |

## Error Handling

The solver validates input data before starting the search.

The following conditions are checked:

- Puzzle values must be between `0` and `9`.
- Region IDs must be between `1` and `9`.
- Every region must contain exactly 9 cells.
- Every region must be orthogonally connected.
- Input files must contain exactly 81 digits.
- Input and output files must be accessible.

Invalid inputs are rejected before constraint propagation begins.

## Limitations

- Supports 9×9 Sudoku only.
- Supports standard row and column constraints.
- Supports custom connected regions through Region Maps.
- Uses binary inequality constraints for AC-3.
- Returns the first valid solution rather than enumerating all possible solutions.
- Does not currently implement global AllDifferent propagation or uniqueness verification.

## Future Improvements

Potential extensions include:

- Generalized Arc Consistency (GAC) for AllDifferent constraints.
- Régin's Algorithm.
- Maintaining Arc Consistency (MAC).
- Sudoku solution uniqueness detection.
- Support for additional Sudoku variants, such as Diagonal Sudoku and Hyper Sudoku.
- Unit testing and performance benchmarking.
- Incremental constraint propagation.

## License

This project is licensed under the MIT License.

## Acknowledgements

- [CLI11](https://github.com/CLIUtils/CLI11) — Command-line argument parsing library.
- CSP and constraint propagation techniques inspired by classical constraint satisfaction literature.
