#include "compile_run.hpp"
#include "../common/httplib.h"

using namespace ns_compile_and_run;
using namespace httplib;

void Usage(std::string proc)
{
    std::cerr << "Usage: " << "\n\t" << proc << " port" << std::endl;
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        Usage(argv[0]);
        return 1;
    }

    Server svr;

    // svr.Get("/hello",
    // [](const Request &req, Response &resp)
    // {
    //     resp.set_content(
    //         "hello httplib!\n",
    //         "text/plain"
    //     );
    // });

    svr.Post("/compile_and_run", [](const Request &req, Response &resp)
             {
        // 用户请求的服务正文是我们想要的 json string
        std::string in_json = req.body;
        std::string out_json;
        if(!in_json.empty()){
            CompileAndRun::Start(in_json, &out_json);
            resp.set_content(out_json, "application/json;charset=utf-8");
        } });

    svr.listen("0.0.0.0", atoi(argv[1]));

    // in_json: {"code": "#include...", "input": "","cpu_limit":1, "mem_limit":10240}
    // out_json: {"status":0, "reason":"", "stdout":"", "stderr":""}
    // 通过 http 让 client 上传一个 json string
    // 下面的工作，充当客户端请求的 json 串
    // std::string in_json;
    // Json::Value in_value;
    // // R"()", raw string
    // in_value["code"] = R"(#include<iostream>
    // int main(){
    //     std::cout << "Hello Compile" << std::endl;
    //     return 0;
    // })";

    // in_value["input"] = "";
    // in_value["cpu_limit"] = 1;
    // in_value["mem_limit"] = 10240 * 3;

    // Json::FastWriter writer;
    // in_json = writer.write(in_value);
    // std::cout << in_json << std::endl;

    // // 将来给客户端返回的 json 串
    // std::string out_json;
    // CompileAndRun::Start(in_json, &out_json);

    // std::cout << out_json << std::endl;
    return 0;
}