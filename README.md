# load-balanced-online-judge

A load-balanced online judge system implemented in C++.

基于 C++ 实现的负载均衡式在线判题系统，提供题目展示、代码提交、代码编译运行、资源限制、结果返回和后端编译服务负载均衡等功能。

## Project Structure

- `comm`：公共工具、日志、HTTP 通信等公共模块
- `compile_server`：用户代码编译、运行和资源限制模块
- `oj_server`：题目管理、页面渲染、判题请求和负载均衡模块
- `docs`：项目文档和架构设计
- `tests`：测试代码
- `sql`：数据库初始化脚本

## Technology Stack

- C++17
- Linux
- STL
- Boost
- cpp-httplib
- JsonCpp
- ctemplate
- MySQL
- Multi-process and multi-threading

## Development Status

The project is under development.
