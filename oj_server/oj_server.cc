#include <iostream>
#include <signal.h>

#include "../common/httplib.h"

using namespace httplib;

int main()
{
    // 用户请求的服务路由功能
    Server svr;

    // 获取所有的题目列表
    svr.Get("/all_questions", [](const Request &req, Response &resp)
            { resp.set_content("题目列表", "text/plain; charset=utf-8"); });

    // 用户要根据题目编号，获取题目的内容
    // /question/100 -> 正则匹配
    // R"()", 原始字符串raw string, 保持字符串内容的原貌，不用做相关的转义
    svr.Get(R"(/question/(\d+))", [](const Request &req, Response &resp){
        std::string number = req.matches[1];
        resp.set_content("第" + number + "题", "text/plain; charset=utf-8");
    });

    // 用户提交代码，使用判题功能(1. 每道题的测试用例 2. compile_and_run)
    svr.Get(R"(/judge/(\d+))", [](const Request &req, Response &resp){
        std::string number = req.matches[1];
        resp.set_content("第" + number + "题的判题", "text/plain; charset=utf-8");
    });

    svr.set_base_dir("./wwwroot");
    svr.listen("0.0.0.0", 8085);
    return 0;
}