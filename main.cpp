#include <CLI/CLI.hpp>
#include <nlohmann/json.hpp>
#include <Sudoku.hpp>
#include <array>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>


using json = nlohmann::json;


// ------------------------------------------------------
// File loading
// ------------------------------------------------------

template <std::size_t Size>
static std::array<int, Size> readGrid(const std::string& path) {
    std::ifstream file(path);

    if (!file) {
        throw std::runtime_error(
            "Cannot open file: " + path
        );
    }

    std::array<int, Size> result{};
    char ch;
    std::size_t position = 0;

    while (file.get(ch)) {
        if (ch >= '0' && ch <= '9') {
            if (position >= Size) {
                throw std::runtime_error(
                    "Too many values in file: " + path
                );
            }

            result[position++] = ch - '0';
        }

        else if (ch == '\n' || ch == '\r' || ch == '\t' || ch == ' ') {
            continue;
        }

        else {
            throw std::runtime_error(
                "Invalid character in file: " + path
            );
        }
    }

    if (position != Size) {
        throw std::runtime_error(
            "Expected exactly " + std::to_string(Size) +
            " digits in file: " + path
        );
    }

    return result;
}

static json createResult(const SudokuCSP& solver, const bool showStats) {
    json result;

    result["success"] = true;
    result["solution"] = json::array();

    const auto& solution = solver.solution();

    for (int row = 0; row < SudokuCSP::N; ++row) {
        json currentRow = json::array();

        for (int col = 0; col < SudokuCSP::N; ++col) {
            currentRow.push_back(
                solution[row * SudokuCSP::N + col]
            );
        }

        result["solution"].push_back(currentRow);
    }

    // Statistics
    if (showStats) {
        const auto&[nodes, backtracks,
                    revisions, values_removed] = solver.statistics();

        result["statistics"] = {
            {"nodes", nodes},
            {"backtracks", backtracks},
            {"revisions", revisions},
            {"values_removed", values_removed}
        };
    }

    return result;
}


// ------------------------------------------------------
// Main / CLI
// ------------------------------------------------------

int main(int argc, char** argv) {
    CLI::App app{
        "A Command-Line Sudoku CSP Solver"
    };

    std::string inputPath;
    std::string regionPath;
    std::string outputPath;

    bool showStats = false;
    bool jsonOutput = false;

    app.add_option(
        "-i,--input",
        inputPath,
        "Path to the Sudoku puzzle file"
    )->required()->check(CLI::ExistingFile);

    app.add_option(
        "-r,--regions",
        regionPath,
        "Path to the Region Map file"
    )->required()->check(CLI::ExistingFile);

    app.add_option(
        "-o,--output",
        outputPath,
        "Optional output file"
    );

    app.add_flag(
        "-s,--stats",
        showStats,
        "Display solver statistics"
    );

    app.add_flag(
        "--json",
        jsonOutput,
        "Output result in JSON format"
    );

    try {
        CLI11_PARSE(app, argc, argv);

        auto puzzle = readGrid<81>(inputPath);
        auto regions = readGrid<81>(regionPath);

        SudokuCSP solver(puzzle, regions);

        if (!solver.solve()) {
            if (jsonOutput) {
                json result = {
                    {"success", false},
                    {"error", {
                        {"code", "NO_SOLUTION"},
                        {"message", "No solution exists."}
                    }}
                };

                std::cout << result.dump(4) << '\n';
            }

            else {
                std::cerr << "Error: No solution exists.\n";
            }

            return 1;
        }

        // Save solution if output path is provided.
        if (!outputPath.empty()) {
            solver.writeSolution(outputPath);
        }

        if (jsonOutput) {
            json result = createResult(solver, showStats);

            if (!outputPath.empty()) {
                result["output_file"] = outputPath;
            }

            std::cout << result.dump(4) << '\n';
        }

        else {
            solver.printSolution();

            if (!outputPath.empty()) {
                std::cout << "Solution saved to: "
                          << outputPath << '\n';
            }

            if (showStats) {
                const auto&[nodes, backtracks,
                            revisions, values_removed] = solver.statistics();

                std::cout << "\nSolver statistics:\n";
                std::cout << "\tSearch nodes: "
                          << nodes << '\n';
                std::cout << "\tBacktracks: "
                          << backtracks << '\n';
                std::cout << "\tAC-3 revisions: "
                          << revisions << '\n';
                std::cout << "\tValues removed: "
                          << values_removed << '\n';
            }
        }

        return 0;
    }

    catch (const CLI::ParseError& e) {
        return app.exit(e);
    }

    catch (const std::exception& e) {
        if (jsonOutput) {
            json result = {
                {"success", false},
                {"error", {
                    {"code", "EXECUTION_ERROR"},
                    {"message", e.what()}
                }}
            };

            std::cout << result.dump(4) << '\n';
        }

        else {
            std::cerr << "Error: " << e.what() << '\n';
        }

        return 3;
    }
}
