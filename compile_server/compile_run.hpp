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
        // code < 0：整个过程发生了非运行错误，例如代码为空或编译失败
        // code == 0：整个过程全部完成
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
                desc = "编译运行服务器内部错误";
                break;

            case -3:
                if (!FileUtil::ReadFile(PathUtil::CompilerError(file_name), &desc, true) || desc.empty())
                {
                    desc = "代码编译失败，无法读取编译错误信息";
                }
                break;

            case -4:
                desc = "请求 JSON 格式错误";
                break;

            case -5:
                desc = "运行时错误：程序以非 0 退出码结束";
                break;

            case -6:
                desc = "CPU 或内存限制参数不合法";
                break;

            case SIGILL: // 4
                desc = "运行时错误：程序执行了非法指令（SIGILL，信号 4）";
                break;

            case SIGABRT: // 6
                desc = "运行时错误：程序异常终止，可能由于内存申请失败";
                break;

            case SIGXCPU: // 24
                desc = "运行超时：超过 CPU 时间限制";
                break;

            case SIGFPE: // 8
                desc = "运行时错误：程序发生算术异常，例如除以 0";
                break;

            case SIGSEGV: // 11
                desc = "运行时错误：程序发生非法内存访问";
                break;

            case SIGXFSZ: // 25
                desc = "运行时错误：程序输出内容超过大小限制";
                break;

            case SIGKILL: // 9
                desc = "运行时错误：程序被强制终止";
                break;

            case SIGBUS: // 7
                desc = "运行时错误：程序发生总线错误";
                break;

            default:
                desc = "运行时错误：收到信号 " + std::to_string(code);
                break;
            }

            return desc;
        }

        /***************************************
         * 输入:
         *  code：用户提交的代码
         *  input：用户给自己提交的代码对应的输入，不做处理
         *  cpu_limit：时间要求
         *  mem_limit：空间要求
         *
         * 输出:
         *  必填:
         *      status：状态码
         *      reason：请求结果
         *  选填：
         *      stdout：程序运行完成后的标准输出
         *      stderr：程序运行完成后的标准错误
         *
         * 参数：
         *  in_json: {"code":"#include...","input":"","cpu_limit":1,"mem_limit":10240}
         *  out_json: {"status":0,"reason":"","stdout":"","stderr":""}
         * ************************************/
        static void Start(const std::string &in_json, std::string *out_json)
        {
            if (out_json == nullptr)
            {
                LOG(ERROR) << "输出 JSON 参数为空" << "\n";
                return;
            }

            out_json->clear();

            // 即使 temp 目录在服务运行过程中被删除，也尝试重新创建
            if (!FileUtil::EnsureDirectory(temp_path))
            {
                Json::Value out_value;
                out_value["status"] = -2;
                out_value["reason"] = "创建临时工作目录失败";

                Json::StyledWriter writer;
                *out_json = writer.write(out_value);
                return;
            }

            Json::Value in_value;
            Json::Reader reader;

            if (!reader.parse(in_json, in_value) || !in_value.isObject() || !in_value.isMember("code") || !in_value.isMember("input") || !in_value.isMember("cpu_limit") || !in_value.isMember("mem_limit"))
            {
                Json::Value out_value;
                out_value["status"] = -4;
                out_value["reason"] = CodeToDesc(-4, "");

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
                status_code = -1;
                goto END;
            }

            if (cpu_limit <= 0 || mem_limit <= 0)
            {
                status_code = -6;
                goto END;
            }

            // 形成的文件名具有唯一性，没有目录也没有后缀
            file_name = FileUtil::UniqFileName();

            // 将用户代码写入待编译源文件
            if (!FileUtil::WriteFile(PathUtil::Src(file_name), code))
            {
                LOG(ERROR) << "写入用户源代码文件失败" << "\n";
                status_code = -2;
                goto END;
            }

            // 将用户输入写入标准输入文件
            if (!FileUtil::WriteFile(PathUtil::Stdin(file_name), input))
            {
                LOG(ERROR) << "写入用户输入文件失败" << "\n";
                status_code = -2;
                goto END;
            }

            // 编译文件
            if (!Compiler::Compile(file_name))
            {
                status_code = -3;
                goto END;
            }

            // 运行文件
            run_result = Runner::Run(file_name, cpu_limit, mem_limit);

            if (run_result == -5)
            {
                status_code = -5;
            }
            else if (run_result < 0)
            {
                status_code = -2;
            }
            else
            {
                // run_result == 0 表示运行成功
                // run_result > 0 表示用户程序收到了对应的信号
                status_code = run_result;
            }

        END:
            out_value["status"] = status_code;
            out_value["reason"] = CodeToDesc(status_code, file_name);

            if (status_code == 0)
            {
                std::string _stdout;
                if (!FileUtil::ReadFile(PathUtil::Stdout(file_name), &_stdout, true))
                {
                    _stdout.clear();
                }
                out_value["stdout"] = _stdout;

                std::string _stderr;
                if (!FileUtil::ReadFile(PathUtil::Stderr(file_name), &_stderr, true))
                {
                    _stderr.clear();
                }
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