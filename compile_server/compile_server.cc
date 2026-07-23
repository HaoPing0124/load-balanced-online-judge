#include "compile_run.hpp"
#include "runner.hpp"

using namespace ns_compile_and_run;
int main()
{
    // in_json: {"code": "#include...", "input": "","cpu_limit":1, "mem_limit":10240}
    // out_json: {"status":0, "reason":"", "stdout":"", "stderr":""}
    // 通过http 让client 给我们 上传一个json string
    // 下面的工作，充当客户端请求的json串
    std::string in_json;
    Json::Value in_value;
    // R"()", raw string
    in_value["code"] = R"(#include<iostream>
    int main(){
        std::cout << "Hello Compile" << std::endl;
        return 0;
    })";

    in_value["input"] = "";
    in_value["cpu_limit"] = 1;
    in_value["mem_limit"] = 10240 * 3;

    Json::FastWriter writer;
    in_json = writer.write(in_value);
    std::cout << in_json << std::endl;

    // 将来给客户端返回的json串
    std::string out_json;
    CompileAndRun::Start(in_json, &out_json);

    std::cout << out_json << std::endl;
    return 0;
}