#pragma once

#include <algorithm>
#include <bit>
#include <deque>
#include <vector>

class SudokuCSP final {
public:
    static constexpr int N = 9;
    static constexpr int CELL_COUNT = N * N;

    // 9-bit bitmask: bits 0..8 represent values 1..9.
    using Domain = std::uint16_t;
    using Domains = std::array<Domain, CELL_COUNT>;

    struct Statistics {
        std::uint64_t nodes = 0;
        std::uint64_t backtracks = 0;
        std::uint64_t revisions = 0;
        std::uint64_t values_removed = 0;
    };

    SudokuCSP(
        const std::array<int, CELL_COUNT>& puzzle,
        const std::array<int, CELL_COUNT>& regions
    );

    bool solve();

    void printSolution() const;

    void writeSolution(const std::string& path) const;

    [[nodiscard]] const Statistics& statistics() const;

    [[nodiscard]] const std::array<int, CELL_COUNT>& solution() const;

private:
    static constexpr Domain FULL_DOMAIN = 0x1FF;

    std::array<int, CELL_COUNT> puzzle_;
    std::array<int, CELL_COUNT> regions_;

    // Constraint graph: adjacency list.
    std::array<std::vector<int>, CELL_COUNT> neighbors_;
    std::array<int, CELL_COUNT> solution_{};

    Statistics stats_;


    // --------------------------------------------------
    // Basic helpers
    // --------------------------------------------------

    static constexpr int index(const int row, const int col) { return row * N + col; }

    static constexpr Domain bit(const int value) { return static_cast<Domain>(1u << (value - 1)); }

    static int domainSize(const Domain domain) { return std::popcount(static_cast<unsigned int>(domain)); }

    static bool contains(const Domain domain, const int value) { return (domain & bit(value)) != 0; }

    static std::vector<int> valuesOf(const Domain domain) {
        std::vector<int> values;

        for (int v = 1; v <= 9; ++v) {
            if (contains(domain, v)) {
                values.push_back(v);
            }
        }

        return values;
    }


    // --------------------------------------------------
    // Input validation
    // --------------------------------------------------

    void validatePuzzle() const;

    void validateRegions() const;


    // --------------------------------------------------
    // Constraint graph construction
    // --------------------------------------------------

    void buildConstraintGraph();


    // --------------------------------------------------
    // AC-3
    // --------------------------------------------------

    bool revise(Domains& domains, int xi, int xj);
    bool ac3(Domains& domains);


    // --------------------------------------------------
    // Forward Checking
    // --------------------------------------------------

    bool forwardChecking(Domains& domains, int variable, int value);


    // --------------------------------------------------
    // MRV + Degree Heuristic
    // --------------------------------------------------

    [[nodiscard]] int selectVariable(const Domains& domains) const;


    // --------------------------------------------------
    // LCV
    // --------------------------------------------------

    [[nodiscard]] std::vector<int> orderValues(const Domains& domains, int variable) const;


    // --------------------------------------------------
    // Backtracking
    // --------------------------------------------------

    bool backtrack(const Domains &domains);
};
