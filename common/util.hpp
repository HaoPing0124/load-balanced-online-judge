#pragma once

#include <iostream>
#include <string>
#include <unistd.h>
#include <fstream>
#include <atomic>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>

namespace ns_util
{
    // 时间工具类
    class TimeUtil
    {
    public:
        static std::string GetTimeStamp()
        {
            struct timeval _time;
            gettimeofday(&_time, nullptr);
            return std::to_string(_time.tv_sec);
        }

        // 获得毫秒时间戳
        static std::string GetTimeMs()
        {
            struct timeval _time;
            gettimeofday(&_time, nullptr);

            long long ms = static_cast<long long>(_time.tv_sec) * 1000 + _time.tv_usec / 1000;
            return std::to_string(ms);
        }
    };

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

        static std::string UniqFileName()
        {
            // 通过进程 ID 和原子递增值，降低并发请求中文件名冲突的风险
            static std::atomic_uint id{0};

            std::string ms = TimeUtil::GetTimeMs();
            std::string pid = std::to_string(getpid());
            std::string uniq_id = std::to_string(id.fetch_add(1, std::memory_order_relaxed));

            // 毫秒时间戳 + 进程 ID + 原子递增编号
            return ms + "_" + pid + "_" + uniq_id;
        }

        static bool WriteFile(const std::string &target, const std::string &content)
        {
            std::ofstream out(target);
            if (!out.is_open())
            {
                return false;
            }

            out.write(content.c_str(), content.size());
            out.close();
            return true;
        }

        static bool ReadFile(const std::string &target, std::string *content, bool keep = false /* 是否保留 '\n' */)
        {
            if (content == nullptr)
            {
                return false;
            }

            content->clear();

            std::ifstream in(target);
            if (!in.is_open())
            {
                return false;
            }

            std::string line;
            // getline:不保存行分割符
            // getline内部重载了强制类型转化
            while (getline(in, line))
            {
                (*content) += line;
                (*content) += (keep ? "\n" : "");
            }
            in.close();
            return true;
        }
    };
}