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

static vector<vector<int>> Normalize(vector<vector<int>> answer)
{
    sort(answer.begin(), answer.end());
    return answer;
}

int main()
{
    {
        vector<vector<int>> intervals = {{1, 3}, {2, 6}, {8, 10}, {15, 18}};
        vector<vector<int>> actual = Normalize(Solution().merge(intervals));
        vector<vector<int>> expected = {{1, 6}, {8, 10}, {15, 18}};
        CheckCase(1, actual, expected);
    }
    {
        vector<vector<int>> intervals = {{1, 4}, {4, 5}};
        vector<vector<int>> actual = Normalize(Solution().merge(intervals));
        vector<vector<int>> expected = {{1, 5}};
        CheckCase(2, actual, expected);
    }
    {
        vector<vector<int>> intervals = {{1, 4}, {0, 2}, {3, 5}};
        vector<vector<int>> actual = Normalize(Solution().merge(intervals));
        vector<vector<int>> expected = {{0, 5}};
        CheckCase(3, actual, expected);
    }
    return 0;
}
