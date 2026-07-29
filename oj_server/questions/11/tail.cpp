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
    vector<int> a = {1, 3, -1, -3, 5, 3, 6, 7};
    CheckCase(1, Solution().maxSlidingWindow(a, 3), vector<int>({3, 3, 5, 5, 6, 7}));

    vector<int> b = {1};
    CheckCase(2, Solution().maxSlidingWindow(b, 1), vector<int>({1}));

    vector<int> c = {9, 8, 7, 6};
    CheckCase(3, Solution().maxSlidingWindow(c, 2), vector<int>({9, 8, 7}));
    return 0;
}
