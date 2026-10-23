#include "utils.hpp"
#include <algorithm>
#include <cctype>
#include <iostream>

#ifdef _WIN32
#include <windows.h>
#include <shellapi.h>

#ifdef _MSC_VER
#pragma comment(lib, "shell32.lib")
#endif

static std::string toUtf8(const std::wstring& text)
{
    if (text.empty())
    {
        return {};
    }

    int size = WideCharToMultiByte(
        CP_UTF8, 0, text.data(), static_cast<int>(text.size()), nullptr, 0, nullptr, nullptr);

    std::string result(size, '\0');

    WideCharToMultiByte(
        CP_UTF8, 0, text.data(), static_cast<int>(text.size()), result.data(), size, nullptr, nullptr);

    return result;
}
#endif

std::string toLower(std::string str) {
    std::transform(
        str.begin(),
        str.end(),
        str.begin(),
        [](unsigned char c)
        {
            return std::tolower(c);
        }
    );

    return str;
}

std::vector<std::string> getArguments(int argc, char* argv[])
{
    std::vector<std::string> arguments;

#ifdef _WIN32
    int count = 0;
    LPWSTR* wideArguments = CommandLineToArgvW(GetCommandLineW(), &count);

    if (wideArguments != nullptr)
    {
        for (int i = 1; i < count; i++)
        {
            arguments.push_back(toUtf8(wideArguments[i]));
        }

        LocalFree(wideArguments);
        return arguments;
    }
#endif

    for (int i = 1; i < argc; i++)
    {
        arguments.push_back(argv[i]);
    }

    return arguments;
}

std::string readLine()
{
#ifdef _WIN32
    HANDLE input = GetStdHandle(STD_INPUT_HANDLE);
    DWORD mode;

    // Read the console in UTF-16, the only reliable way to get
    // non-ASCII characters typed by the user
    if (GetConsoleMode(input, &mode))
    {
        std::wstring line;
        wchar_t buffer[256];
        DWORD read = 0;

        while (ReadConsoleW(input, buffer, 256, &read, nullptr) && read > 0)
        {
            line.append(buffer, read);

            if (line.back() == L'\n')
            {
                break;
            }
        }

        while (!line.empty() && (line.back() == L'\n' || line.back() == L'\r'))
        {
            line.pop_back();
        }

        return toUtf8(line);
    }
#endif

    std::string line;
    std::getline(std::cin, line);

    if (!line.empty() && line.back() == '\r')
    {
        line.pop_back();
    }

    return line;
}

Utf8Console::Utf8Console()
{
#ifdef _WIN32
    previousCodePage = GetConsoleOutputCP();
    SetConsoleOutputCP(CP_UTF8);
#endif
}

Utf8Console::~Utf8Console()
{
#ifdef _WIN32
    if (previousCodePage != 0)
    {
        SetConsoleOutputCP(previousCodePage);
    }
#endif
}
