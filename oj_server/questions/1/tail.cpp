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
    {
        vector<int> nums = {2, 7, 11, 15};
        vector<int> actual = Solution().twoSum(nums, 9);
        vector<int> expected = {0, 1};
        sort(actual.begin(), actual.end());
        CheckCase(1, actual, expected);
    }
    {
        vector<int> nums = {3, 2, 4};
        vector<int> actual = Solution().twoSum(nums, 6);
        vector<int> expected = {1, 2};
        sort(actual.begin(), actual.end());
        CheckCase(2, actual, expected);
    }
    {
        vector<int> nums = {-3, 4, 3, 90};
        vector<int> actual = Solution().twoSum(nums, 0);
        vector<int> expected = {0, 2};
        sort(actual.begin(), actual.end());
        CheckCase(3, actual, expected);
    }
    return 0;
}
