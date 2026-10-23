#pragma once

#include <string>
#include <vector>

std::string toLower(std::string str);

// Command line arguments encoded in UTF-8 (on Windows argv uses the
// ANSI code page, which can't represent every file name)
std::vector<std::string> getArguments(int argc, char* argv[]);

// Reads a line from the standard input, encoded in UTF-8
std::string readLine();

// Switches the console to UTF-8 while the object is alive
class Utf8Console
{
public:
    Utf8Console();
    ~Utf8Console();

    Utf8Console(const Utf8Console&) = delete;
    Utf8Console& operator=(const Utf8Console&) = delete;

private:
    unsigned int previousCodePage = 0;
};
