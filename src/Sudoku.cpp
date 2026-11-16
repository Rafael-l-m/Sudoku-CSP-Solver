#include <Sudoku.hpp>
#include <fstream>
#include <iostream>
#include <queue>
#include <utility>


// Public

SudokuCSP::SudokuCSP(
    const std::array<int, CELL_COUNT>& puzzle,
    const std::array<int, CELL_COUNT>& regions
) : puzzle_(puzzle), regions_(regions) {

    validatePuzzle();
    validateRegions();
    buildConstraintGraph();
}

bool SudokuCSP::solve() {
    stats_ = {};

    // Every cell starts with domain {1,...,9}.
    Domains domains;

    for (int i = 0; i < CELL_COUNT; ++i) {
        if (puzzle_[i] == 0) {
            domains[i] = FULL_DOMAIN;
        }

        else {
            domains[i] = bit(puzzle_[i]);
        }
    }

    // Initial constraint propagation.
    if (!ac3(domains)) {
        return false;
    }

    return backtrack(domains);
}

void SudokuCSP::printSolution() const {
    std::cout << "\nSolution:\n";

    for (int r = 0; r < N; ++r) {
        if (r % 3 == 0) {
            std::cout << "+-------+-------+-------+\n";
        }

        for (int c = 0; c < N; ++c) {
            if (c % 3 == 0) {
                std::cout << "| ";
            }

            std::cout << solution_[index(r, c)] << ' ';
        }

        std::cout << "|\n";
    }

    std::cout << "+-------+-------+-------+\n";
}

void SudokuCSP::writeSolution(const std::string& path) const {
    std::ofstream out(path);

    if (!out) {
        throw std::runtime_error(
            "Cannot open output file: " + path
        );
    }

    for (int r = 0; r < N; ++r) {
        for (int c = 0; c < N; ++c) {
            out << solution_[index(r, c)];
            if (c != N - 1) out << ' ';
        }
        out << '\n';
    }
}

const SudokuCSP::Statistics& SudokuCSP::statistics() const {
    return stats_;
}

const std::array<int, SudokuCSP::CELL_COUNT>& SudokuCSP::solution() const {
    return solution_;
}


// Private

void SudokuCSP::validatePuzzle() const {
    for (const auto value : puzzle_) {
        if (value < 0 || value > 9) {
            throw std::runtime_error(
                "Puzzle values must be between 0 and 9."
            );
        }
    }
}

void SudokuCSP::validateRegions() const {
    std::array<int, 10> count{};

    for (const auto region : regions_) {
        if (region < 1 || region > 9) {
            throw std::runtime_error(
                "Region IDs must be between 1 and 9."
            );
        }

        ++count[region];
    }

    for (int region = 1; region <= 9; ++region) {
        if (count[region] != 9) {
            throw std::runtime_error(
                "Every region must contain exactly 9 cells."
            );
        }
    }

    // Each region must also be orthogonally connected.
    for (int region = 1; region <= 9; ++region) {
        std::array<bool, CELL_COUNT> visited{};
        std::queue<int> q;

        int start = -1;

        for (int i = 0; i < CELL_COUNT; ++i) {
            if (regions_[i] == region) {
                start = i;
                break;
            }
        }

        q.push(start);
        visited[start] = true;

        int reached = 0;

        while (!q.empty()) {
            int current = q.front();
            q.pop();

            ++reached;

            const int r = current / N;
            const int c = current % N;

            for (int k = 0; k < 4; ++k) {
                constexpr int dc[] = {0, 0, -1, 1};
                constexpr int dr[] = {-1, 1, 0, 0};

                const int nr = r + dr[k];
                const int nc = c + dc[k];

                if (nr < 0 || nr >= N ||
                    nc < 0 || nc >= N) {
                    continue;
                }

                if (int next = index(nr, nc);
                    !visited[next] && regions_[next] == region) {
                    visited[next] = true;
                    q.push(next);
                }
            }
        }

        if (reached != 9) {
            throw std::runtime_error(
                "Region " + std::to_string(region) +
                " is not connected."
            );
        }
    }
}

void SudokuCSP::buildConstraintGraph() {
    for (int i = 0; i < CELL_COUNT; ++i) {
        const int r1 = i / N;
        const int c1 = i % N;

        for (int j = 0; j < CELL_COUNT; ++j) {
            if (i == j) continue;

            const int r2 = j / N;
            const int c2 = j % N;

            const bool sameRow = r1 == r2;
            const bool sameColumn = c1 == c2;
            const bool sameRegion = regions_[i] == regions_[j];

            if (sameRow || sameColumn || sameRegion) {
                neighbors_[i].push_back(j);
            }
        }
    }
}

bool SudokuCSP::revise(Domains& domains, const int xi, const int xj) {
    ++stats_.revisions;

    const Domain oldDomain = domains[xi];
    Domain newDomain = oldDomain;

    // Constraint: Xi != Xj.
    //
    // A value can only be removed when Xj has
    // a singleton domain containing that value.

    if (domainSize(domains[xj]) == 1) {
        if (const Domain neighborValue = domains[xj]; (newDomain & neighborValue) != 0) {
            newDomain &= ~neighborValue;
        }
    }

    domains[xi] = newDomain;

    stats_.values_removed +=
        domainSize(oldDomain) - domainSize(newDomain);

    return oldDomain != newDomain;
}

bool SudokuCSP::ac3(Domains& domains) {
    using Arc = std::pair<int, int>;

    std::deque<Arc> queue;

    // Initially add every directed arc.
    for (int xi = 0; xi < CELL_COUNT; ++xi) {
        for (int xj : neighbors_[xi]) {
            queue.emplace_back(xi, xj);
        }
    }

    while (!queue.empty()) {
        auto [xi, xj] = queue.front();
        queue.pop_front();

        if (revise(domains, xi, xj)) {

            if (domains[xi] == 0) {
                return false;
            }

            // If Xi changed, its other neighbors
            // must be reconsidered.
            for (int xk : neighbors_[xi]) {
                if (xk != xj) {
                    queue.emplace_back(xk, xi);
                }
            }
        }
    }

    return true;
}

bool SudokuCSP::forwardChecking(Domains& domains, const int variable, const int value) {
    const Domain assignedValue = bit(value);

    for (const auto neighbor : neighbors_[variable]) {

        if (domains[neighbor] & assignedValue) {

            domains[neighbor] &= ~assignedValue;

            ++stats_.values_removed;

            if (domains[neighbor] == 0) {
                return false;
            }
        }
    }

    return true;
}

int SudokuCSP::selectVariable(const Domains& domains) const {
    int bestVariable = -1;
    int bestDomainSize = 10;
    int bestDegree = -1;

    for (int variable = 0; variable < CELL_COUNT; ++variable) {

        const auto currentSize = domainSize(domains[variable]);

        // Already assigned.
        if (currentSize <= 1) {
            continue;
        }

        // Degree heuristic:
        // Count unassigned neighboring variables.
        int degree = 0;

        for (const auto neighbor : neighbors_[variable]) {
            if (domainSize(domains[neighbor]) > 1) {
                ++degree;
            }
        }

        // MRV first, Degree as tie-breaker.
        if (currentSize < bestDomainSize ||
            (currentSize == bestDomainSize &&
            degree > bestDegree))
        {

            bestVariable = variable;
            bestDomainSize = currentSize;
            bestDegree = degree;
        }
    }

    return bestVariable;
}

std::vector<int> SudokuCSP::orderValues(const Domains& domains, const int variable) const {

    auto values = valuesOf(domains[variable]);

    // Least Constraining Value:
    // Prefer values that eliminate the fewest
    // possibilities from neighboring domains.

    auto impact = [&](int value) {
        int eliminated = 0;

        for (const auto neighbor : neighbors_[variable]) {
            if (contains(domains[neighbor], value)) {
                ++eliminated;
            }
        }

        return eliminated;
    };

    std::ranges::stable_sort(values,[&](const int a, const int b) {
        return impact(a) < impact(b);
    });

    return values;
}

bool SudokuCSP::backtrack(const Domains& domains) {
    ++stats_.nodes;

    // All domains are singleton:
    // the assignment is complete.
    const auto variable = selectVariable(domains);

    if (variable == -1) {
        for (int i = 0; i < CELL_COUNT; ++i) {
            if (domainSize(domains[i]) != 1) {
                return false;
            }

            solution_[i] = std::countr_zero(
            static_cast<unsigned int>(domains[i])
            ) + 1;
        }

        return true;
    }

    // MRV + Degree selects the variable.
    // LCV determines the value ordering.

    for (const auto values = orderValues(domains, variable); const auto value : values) {

        // Each branch gets its own copy.
        // This automatically restores domains
        // when backtracking.
        Domains nextDomains = domains;

        // Assign X = value.
        nextDomains[variable] = bit(value);

        // Forward Checking.
        if (!forwardChecking(
            nextDomains,
            variable,
            value))
        {
            ++stats_.backtracks;
            continue;
        }

        // AC-3 propagation.
        if (!ac3(nextDomains)) {
            ++stats_.backtracks;
            continue;
        }

        // Recursive search.
        if (backtrack(nextDomains)) {
            return true;
        }

        ++stats_.backtracks;
    }

    return false;
}
