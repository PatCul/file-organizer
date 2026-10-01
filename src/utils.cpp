#include "utils.hpp"
#include <algorithm>
#include <cctype>

string toLower(string str) {
    transform(
        str.begin(),
        str.end(),
        str.begin(),
        [](unsigned char c)
        {
            return tolower(c);
        }
    );

    return str;
}