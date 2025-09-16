#include "../../libs/Catch2/catch_amalgamated.hpp"
#include <iostream>
#include <sstream>
#include <string>

// This global stringstream remains the same.
std::stringstream g_testResults;

// A helper function to trim the useless part of the file paths from the output.
void FormatTestOutput(std::stringstream& stream) {
    std::string full_output = stream.str();
    stream.str(""); // Clear the original stream
    stream.clear();

    // Define the common path prefixes we want to remove.
    // This makes the output clean regardless of where the project is saved.
    const std::string src_prefix = "src\\Tests\\";
    const std::string another_prefix = "src/Tests/"; // Handle both slash types

    std::stringstream input_stream(full_output);
    std::string line;

    while (std::getline(input_stream, line)) {
        size_t pos = line.find(src_prefix);
        if (pos != std::string::npos) {
            // If we found the prefix, only keep the part after it.
            stream << line.substr(pos + src_prefix.length()) << std::endl;
        }
        else {
            pos = line.find(another_prefix);
            if (pos != std::string::npos) {
                // Handle the other slash type as well.
                stream << line.substr(pos + another_prefix.length()) << std::endl;
            }
            else {
                // If it's a summary line or something else, print it as is.
                stream << line << std::endl;
            }
        }
    }
}

void RunAllTests() {
    g_testResults.str("");
    g_testResults.clear();

    auto original_streambuf = Catch::cout().rdbuf();
    Catch::cout().rdbuf(g_testResults.rdbuf());

    // By declaring the session as 'static', it is created only ONCE.
    // This avoids all constructor/destructor related crashes.
    static Catch::Session session;

    char* argv[] = {
        (char*)"kx-vision-tests",
        (char*)"-r", (char*)"compact",
        (char*)"-s",
        (char*)"--colour-mode", (char*)"none"
    };
    int argc = sizeof(argv) / sizeof(argv[0]);

    // We must wrap the run() call in a try/catch block to gracefully
    // handle test failures (e.g., a failed REQUIRE) without crashing the game.
    try {
        // Note: This will only run the tests the first time the button is clicked
        // because the session object is stateful. This is the desired, safe behavior.
        session.run(argc, argv);
    }
    catch (const std::exception& e) {
        g_testResults << "\nFATAL ERROR: A C++ exception was caught during the test run:\n"
            << e.what() << std::endl;
    }
    catch (...) {
        // This catches the TestFailureException from a failed REQUIRE.
        g_testResults << "\nTest run concluded with one or more failures." << std::endl;
    }

    Catch::cout().rdbuf(original_streambuf);

    FormatTestOutput(g_testResults);
}