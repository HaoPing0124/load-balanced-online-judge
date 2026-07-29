#ifndef COMPILER_ONLINE
#include "header.cpp"
#endif
#include <algorithm>
#include <cstdlib>
#include <sstream>
#include <string>

static string JsonEscape(const string& text)
{
    string out;
    for (size_t i = 0; i < text.size(); ++i)
    {
        char ch = text[i];
        switch (ch)
        {
        case '\\': out += "\\\\"; break;
        case '"': out += "\\\""; break;
        case '\n': out += "\\n"; break;
        case '\r': out += "\\r"; break;
        case '\t': out += "\\t"; break;
        default: out += ch; break;
        }
    }
    return out;
}

template <class T>
struct TextFormatter
{
    static string Get(const T& value)
    {
        ostringstream out;
        out << value;
        return out.str();
    }
};

template <>
struct TextFormatter<string>
{
    static string Get(const string& value)
    {
        return string("\"") + JsonEscape(value) + "\"";
    }
};

template <class T>
struct TextFormatter<vector<T>>
{
    static string Get(const vector<T>& values)
    {
        string out = "[";
        for (size_t i = 0; i < values.size(); ++i)
        {
            if (i != 0) out += ",";
            out += TextFormatter<T>::Get(values[i]);
        }
        out += "]";
        return out;
    }
};

template <class T>
static void CheckCase(int index, const T& actual, const T& expected)
{
    bool passed = actual == expected;
    cerr << "__OJ_CASE__{\"case\":" << index
         << ",\"passed\":" << (passed ? "true" : "false")
         << ",\"actual\":\"" << JsonEscape(TextFormatter<T>::Get(actual))
         << "\",\"expected\":\"" << JsonEscape(TextFormatter<T>::Get(expected))
         << "\"}" << endl;

    if (!passed)
    {
        abort();
    }
}

int main()
{
    vector<int> a = {1, 8, 6, 2, 5, 4, 8, 3, 7};
    CheckCase(1, Solution().maxArea(a), 49);

    vector<int> b = {1, 1};
    CheckCase(2, Solution().maxArea(b), 1);

    vector<int> c = {4, 3, 2, 1, 4};
    CheckCase(3, Solution().maxArea(c), 16);
    return 0;
}
