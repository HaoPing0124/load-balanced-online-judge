#pragma once

#include <iostream>
#include <string>
#include <unistd.h>
#include <fstream>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>

namespace ns_util
{
    // 路径工具类
    const std::string temp_path = "./temp/";
    class PathUtil
    {
    public:
        static std::string AddSuffix(const std::string &file_name, const std::string &suffix)
        {
            std::string path_name = temp_path;
            path_name += file_name;
            path_name += suffix;
            return path_name;
        }

        // 编译时需要有的临时文件
        // 构建源文件路径+后缀的完整文件名
        // 1234 -> ./temp/1234.cpp
        static std::string Src(const std::string &file_name)
        {
            return AddSuffix(file_name, ".cpp");
        }

        static std::string Exe(const std::string &file_name)
        {
            return AddSuffix(file_name, ".exe");
        }

        static std::string CompilerError(const std::string &file_name)
        {
            return AddSuffix(file_name, ".compile_error");
        }

        // 运行时需要的临时文件
        static std::string Stdin(const std::string &file_name)
        {
            return AddSuffix(file_name, ".stdin");
        }

        static std::string Stdout(const std::string &file_name)
        {
            return AddSuffix(file_name, ".stdout");
        }
        
        // 构建该程序对应的标准错误完整的路径+后缀名
        static std::string Stderr(const std::string &file_name)
        {
            return AddSuffix(file_name, ".stderr");
        }
    };

    // 文件工具类
    class FileUtil
    {
    public:
        static bool IsFileExists(const std::string &path_name)
        {
            struct stat st;
            if (stat(path_name.c_str(), &st) == 0)
            {
                // 获取属性成功，文件已经存在
                return true;
            }

            return false;
        }
    };

    class TimeUtil
    {
    public:
        static std::string GetTimeStamp()
        {
            struct timeval _time;
            gettimeofday(&_time, nullptr);
            return std::to_string(_time.tv_sec);
        }
    };
}