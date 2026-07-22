#pragma once

#include <iostream>
#include <string>
#include <unistd.h>
#include <fstream>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include "../common/util.hpp"

namespace ns_compiler
{
    using namespace ns_util;

    class Compiler
    {
    public:
        Compiler() {}
        ~Compiler() {}

        // 返回值：编译成功：true，否则：false
        // 输入参数：编译的文件名
        // file_name: 1234
        // 1234 -> ./temp/1234.cpp
        // 1234 -> ./temp/1234.exe
        // 1234 -> ./temp/1234.compile_error
        static bool Compile(const std::string &file_name)
        {
            pid_t pid = fork();
            if (pid < 0)
            {
                return false;
            }
            else if (pid == 0)
            {
                // 子进程
                umask(0);
                int _stderr = open(PathUtil::CompilerError(file_name).c_str(), O_CREAT | O_WRONLY | O_TRUNC, 0644);
                if (_stderr < 0)
                {
                    exit(1);
                }

                // 重定向标准错误到_stderr
                dup2(_stderr, 2);

                // 程序替换，并不影响进程的文件描述符表
                // 子进程: 调用编译器，完成对代码的编译工作
                // g++ -o target src -std=c++11
                execlp("g++", "g++", "-o", PathUtil::Exe(file_name).c_str(),
                       PathUtil::Src(file_name).c_str(), "-std=c++11", nullptr);

                exit(2);
            }
            else
            {
                // 父进程
                waitpid(pid, nullptr, 0);

                // 编译是否成功,就看有没有形成对应的可执行程序
                if (FileUtil::IsFileExists(PathUtil::Exe(file_name)))
                {
                    return true;
                }
            }

            return false;
        }
    };
}