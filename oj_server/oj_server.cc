#include <iostream>
#include <signal.h>

#include "../common/httplib.h"
#include "oj_control.hpp"

using namespace httplib;
using namespace ns_control;

int main()
{
    // 用户请求的服务路由功能
    Server svr;
    Control ctrl;

    // 获取所有的题目列表
    svr.Get("/all_questions", [&ctrl](const Request &req, Response &resp)
            { 
                //返回一张包含有所有题目的html网页
                std::string html;
                ctrl.AllQuestions(&html);

                //用户看到的是 网页数据 + 拼上了题目相关的数据
                resp.set_content(html, "text/html; charset=utf-8"); });

    // 用户要根据题目编号，获取题目的内容
    // /question/100 -> 正则匹配
    // R"()", 原始字符串raw string, 保持字符串内容的原貌，不用做相关的转义
    svr.Get(R"(/question/(\d+))", [&ctrl](const Request &req, Response &resp)
            {
                std::string number = req.matches[1];
                std::string html;
                ctrl.Question(number, &html);
                resp.set_content(html, "text/html; charset=utf-8"); });

    // 用户提交代码，使用判题功能(1. 每道题的测试用例 2. compile_and_run)
    svr.Get(R"(/judge/(\d+))", [](const Request &req, Response &resp)
            {
        std::string number = req.matches[1];
        resp.set_content("第" + number + "题的判题", "text/plain; charset=utf-8"); });

    svr.set_base_dir("./wwwroot");
    svr.listen("0.0.0.0", 8085);
    return 0;
}