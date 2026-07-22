#pragma once

#include <iostream>
#include <string>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>

#include "../common/util.hpp"
#include "../common/log.hpp"

namespace ns_runner
{
    using namespace ns_util;
    using namespace ns_log;

    class Runner
    {
    public:
        Runner() {}
        ~Runner() {}

    public:
        // 指明文件名即可，不需要代理路径，不需要带后缀
        /*******************************************
         * 返回值 > 0: 运行成功，但程序异常了，退出时收到了信号，返回值就是对应的信号编号
         * 返回值 == 0: 正常运行完毕，结果保存到了对应的临时文件中
         * 返回值 < 0: 运行失败，内部错误
         * *****************************************/
        static int Run(const std::string &file_name)
        {
            /*********************************************
             * 程序运行：
             * 1. 代码跑完，结果正确
             * 2. 代码跑完，结果不正确
             * 3. 代码没跑完，异常了
             * Run需要考虑代码跑完 不需要考虑运行是否正确
             * 结果正确与否：由测试用例决定
             * 只考虑：是否正确运行完毕
             *
             * 一个程序在默认启动的时候
             * 标准输入: 不处理
             * 标准输出: 程序运行完成，输出结果是什么
             * 标准错误: 运行时错误信息
             * *******************************************/
            // temp/./code.exe
            std::string _execute = PathUtil::Exe(file_name);
            std::string _stdin = PathUtil::Stdin(file_name);
            std::string _stdout = PathUtil::Stdout(file_name);
            std::string _stderr = PathUtil::Stderr(file_name);

            umask(0);
            int _stdin_fd = open(_stdin.c_str(), O_CREAT | O_RDONLY, 0644);
            int _stdout_fd = open(_stdout.c_str(), O_CREAT | O_WRONLY | O_TRUNC, 0644);
            int _stderr_fd = open(_stderr.c_str(), O_CREAT | O_WRONLY | O_TRUNC, 0644);

            if (_stdin_fd < 0 || _stdout_fd < 0 || _stderr_fd < 0)
            {
                LOG(ERROR) << "运行时打开标准文件失败" << "\n";
                return -1; // 代表打开文件失败
            }

            pid_t pid = fork();
            if (pid < 0)
            {
                LOG(ERROR) << "运行时创建子进程失败" << "\n";
                close(_stdin_fd);
                close(_stdout_fd);
                close(_stderr_fd);
                return -2;
            }
            else if (pid == 0)
            {
                // 子进程
                // 重定向
                dup2(_stdin_fd, 0);
                dup2(_stdout_fd, 1);
                dup2(_stderr_fd, 2);

                execl(_execute.c_str() /*执行谁*/, _execute.c_str() /*如何执行*/, nullptr);
                exit(1);
            }
            else
            {
                // 父进程
                close(_stdin_fd);
                close(_stdout_fd);
                close(_stderr_fd);

                int status = 0;
                waitpid(pid, &status, 0);

                // 程序运行异常，一定是因为因为收到了信号
                LOG(INFO) << "运行完毕, info: " << (status & 0x7F) << "\n";
                return status & 0x7F;
            }
        }
    };
}