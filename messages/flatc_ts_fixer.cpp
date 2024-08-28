/**
 * @file flatc_ts_fixer.cpp
 *
 * Correct generated flatbuffer typescript files
 *
 * @author Dan Keenan
 * @date 8/25/24
 * @copyright GNU GPLv3
 */

#include <filesystem>
#include <fstream>
#include <iostream>
#include <regex>

int main(int argc, char* argv[])
{
    if (argc != 3) {
        std::cout << "Usage: " << argv[0] << " <fbs ts dir> <out dir>" << std::endl;
        return 1;
    }

    const std::filesystem::path inDir(argv[1]);
    const std::filesystem::path outDir(argv[2]);
    // https://regex101.com/r/zhn2JH/2
    const std::regex filepathRegex(R"((['"])[./]*mobilesacn/message/(.+)\.(?:js|ts)\1)");
    for (const auto& entry : std::filesystem::recursive_directory_iterator(inDir)) {
        if (!entry.is_regular_file() || entry.path().extension() != ".ts") {
            continue;
        }

        // Setup I/O.
        const auto relPath = std::filesystem::relative(entry.path(), inDir);
        std::ifstream in(entry.path(), std::ios_base::in | std::ios::binary);
        if (!in.is_open()) {
            std::cerr << "Failed to open file " << entry.path() << std::endl;
            return 1;
        }
        std::ofstream out(outDir / relPath,
                          std::ios_base::out | std::ios::binary | std::ios::trunc);
        if (!out.is_open()) {
            std::cerr << "Failed to open file " << (outDir / relPath) << std::endl;
            return 1;
        }

        // Read line by line, fixing imports as needed.
        for (std::string line; std::getline(in, line);) {
            std::regex_replace(std::ostreambuf_iterator(out), line.begin(), line.end(),
                               filepathRegex, R"($1./$2.ts$1)");
            out << std::endl;
        }
    }

    return 0;
}
