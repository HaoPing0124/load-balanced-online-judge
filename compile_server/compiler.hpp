#pragma once

#include <iostream>
#include <string>
#include <unistd.h>
#include <fstream>
#include <fcntl.h>
#include <cerrno>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>

#include "../common/util.hpp"
#include "../common/log.hpp"

namespace ns_compiler
{
    using namespace ns_util;
    using namespace ns_log;

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
            // 删除可能残留的旧可执行程序，避免本次编译失败却被判断为成功
            unlink(PathUtil::Exe(file_name).c_str());

            pid_t pid = fork();
            if (pid < 0)
            {
                LOG(ERROR) << "内部错误，创建编译子进程失败" << "\n";
                return false;
            }
            else if (pid == 0)
            {
                // 子进程
                umask(0);

                int _stderr = open(PathUtil::CompilerError(file_name).c_str(), O_CREAT | O_WRONLY | O_TRUNC, 0644);
                if (_stderr < 0)
                {
                    _exit(1);
                }

                // 重定向标准错误到 _stderr
                if (dup2(_stderr, 2) < 0)
                {
                    close(_stderr);
                    _exit(2);
                }

                close(_stderr);

                // 程序替换，并不影响进程的文件描述符表
                // 子进程：调用编译器，完成对代码的编译工作
                // g++ -o target src -D COMPILER_ONLINE -std=c++11
                execlp("g++", "g++", "-o", PathUtil::Exe(file_name).c_str(), PathUtil::Src(file_name).c_str(), "-D", "COMPILER_ONLINE", "-std=c++11", nullptr);

                // execlp 执行成功后，不会继续执行下面的代码
                _exit(3);
            }
            else
            {
                // 父进程
                int status = 0;
                pid_t ret = 0;

                do
                {
                    ret = waitpid(pid, &status, 0);
                } while (ret < 0 && errno == EINTR);

                if (ret < 0)
                {
                    LOG(ERROR) << "等待编译子进程失败" << "\n";
                    return false;
                }

                // 编译子进程必须正常退出，并且退出码必须为 0
                if (!WIFEXITED(status) || WEXITSTATUS(status) != 0)
                {
                    LOG(ERROR) << PathUtil::Src(file_name) << " 编译失败" << "\n";
                    return false;
                }

                // 编译是否成功，还要检查是否形成对应的可执行程序
                if (FileUtil::IsFileExists(PathUtil::Exe(file_name)))
                {
                    LOG(INFO) << PathUtil::Src(file_name) << " 编译成功!" << "\n";
                    return true;
                }
            }

            LOG(ERROR) << "编译失败，没有形成可执行程序" << "\n";
            return false;
        }
    };
}
