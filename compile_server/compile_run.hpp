#pragma once

#include <signal.h>
#include <jsoncpp/json/json.h>

#include "compiler.hpp"
#include "runner.hpp"
#include "../common/util.hpp"
#include "../common/log.hpp"

namespace ns_compile_and_run
{
    using namespace ns_util;
    using namespace ns_log;
    using namespace ns_compiler;
    using namespace ns_runner;

    class CompileAndRun
    {
    public:
        // code > 0：进程收到了信号，导致程序异常崩溃
        // code < 0：编译运行流程出现错误，例如代码为空或编译失败
        // code == 0：整个编译运行流程正常完成
        static std::string CodeToDesc(int code, const std::string &file_name)
        {
            std::string desc;
            switch (code)
            {
            case 0:
                desc = "编译运行成功";
                break;
            case -1:
                desc = "提交的代码是空";
                break;
            case -2:
                desc = "未知错误";
                break;
            case -3:
                // desc = "代码编译的时候发生了错误";
                if (!FileUtil::ReadFile(PathUtil::CompilerError(file_name), &desc, true) || desc.empty())
                {
                    desc = "代码编译失败，无法读取编译错误信息";
                }
                break;
            case -4:
                return "请求 JSON 格式错误";
            case SIGABRT: // 6
                desc = "程序异常终止，可能由于内存超限";
                break;
            case SIGXCPU: // 24
                desc = "CPU使用超时";
                break;
            case SIGFPE: // 8
                desc = "程序发生算术异常，例如除以 0";
                break;
            case SIGSEGV:
                desc = "程序发生非法内存访问";
                break;
            default:
                desc = "未知: " + std::to_string(code);
                break;
            }

            return desc;
        }

        /***************************************
         * 输入:
         *  code： 用户提交的代码
         *  input: 用户给自己提交的代码对应的输入，不做处理
         *  cpu_limit: 时间要求
         *  mem_limit: 空间要求
         *
         * 输出:
         *  必填:
         *      status: 状态码
         *      reason: 请求结果
         *  选填：
         *      stdout: 我的程序运行完的结果
         *      stderr: 我的程序运行完的错误结果
         *
         * 参数：
         *  in_json: {"code": "#include...", "input": "","cpu_limit":1, "mem_limit":10240}
         *  out_json: {"status":0, "reason":"", "stdout":"", "stderr":""}
         * ************************************/
        static void Start(const std::string &in_json, std::string *out_json)
        {
            if (out_json == nullptr)
            {
                LOG(ERROR) << "输出 JSON 参数为空" << "\n";
                return;
            }

            Json::Value in_value;
            Json::Reader reader;
            if (!reader.parse(in_json, in_value))
            {
                Json::Value out_value;
                out_value["status"] = -4;
                out_value["reason"] = "请求 JSON 格式错误";

                Json::StyledWriter writer;
                *out_json = writer.write(out_value);
                return;
            }

            std::string code = in_value["code"].asString();
            std::string input = in_value["input"].asString();
            int cpu_limit = in_value["cpu_limit"].asInt();
            int mem_limit = in_value["mem_limit"].asInt();

            Json::Value out_value;
            int status_code = 0;
            int run_result = 0;
            std::string file_name;

            if (code.size() == 0)
            {
                status_code = -1; // 代码为空
                goto END;
            }

            // 形成的文件名具有唯一性，没有目录就没有后缀
            // 毫秒级时间戳 + 原子性递增唯一值：保证唯一性
            file_name = FileUtil::UniqFileName();

            // 形成临时 src 文件
            // 将用户代码 code 写入到 待编译文件 [file_name].cpp 中
            if (!FileUtil::WriteFile(PathUtil::Src(file_name), code))
            {
                status_code = -2; // 未知错误
                goto END;
            }

            // 将用户输入写入到输入文件中
            if (!FileUtil::WriteFile(PathUtil::Stdin(file_name), input))
            {
                status_code = -2; // 未知错误
                goto END;
            }

            // 编译文件
            if (!Compiler::Compile(file_name))
            {
                status_code = -3; // 代码编译的时候发生了错误
                goto END;
            }

            // 运行文件
            run_result = Runner::Run(file_name, cpu_limit, mem_limit);
            if (run_result < 0)
            {
                // 未知错误
                status_code = -2;
            }
            else if (run_result > 0)
            {
                // 程序运行崩溃
                status_code = run_result;
            }
            else
            {
                // 运行成功
                status_code = 0;
            }

        END:
            out_value["status"] = status_code;
            out_value["reason"] = CodeToDesc(status_code, file_name);

            if (status_code == 0)
            {
                // 整个过程全部成功
                std::string _stdout;
                FileUtil::ReadFile(PathUtil::Stdout(file_name), &_stdout, true);
                out_value["stdout"] = _stdout;

                std::string _stderr;
                FileUtil::ReadFile(PathUtil::Stderr(file_name), &_stderr, true);
                out_value["stderr"] = _stderr;
            }

            Json::StyledWriter writer;
            *out_json = writer.write(out_value);

            if (!file_name.empty())
            {
                RemoveTempFile(file_name);
            }
        }

    private:
        static void RemoveFile(const std::string &path)
        {
            if (!FileUtil::IsFileExists(path))
            {
                return;
            }

            if (unlink(path.c_str()) < 0)
            {
                LOG(WARNING) << "删除临时文件失败: " << path << "\n";
            }
        }

        static void RemoveTempFile(const std::string &file_name)
        {
            RemoveFile(PathUtil::Src(file_name));
            RemoveFile(PathUtil::CompilerError(file_name));
            RemoveFile(PathUtil::Exe(file_name));
            RemoveFile(PathUtil::Stdin(file_name));
            RemoveFile(PathUtil::Stdout(file_name));
            RemoveFile(PathUtil::Stderr(file_name));
        }
    };
}