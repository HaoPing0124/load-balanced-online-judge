#pragma once

#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <algorithm>
#include <mutex>
#include <cassert>
#include <jsoncpp/json/json.h>

#include "../common/util.hpp"
#include "../common/log.hpp"
#include "../common/httplib.h"
#include "oj_model.hpp"
#include "oj_view.hpp"

namespace ns_control
{
    using namespace std;
    using namespace ns_log;
    using namespace ns_util;
    using namespace ns_model;
    using namespace ns_view;
    using namespace httplib;

    // 提供服务的主机
    class Machine
    {
    public:
        Machine() : ip(""), port(0), load(0), mutex(nullptr)
        {
        }
        ~Machine() {}

    public:
        // 提升主机负载
        void IncLoad()
        {
            if (mutex) mutex->lock();
            ++load;
            if (mutex) mutex->unlock();
        }

        // 减少主机负载
        void DecLoad()
        {
            if (mutex) mutex->lock();
            --load;
            if (mutex) mutex->unlock();
        }

        // 重置主机负载
        void ResetLoad()
        {
            if (mutex) mutex->lock();
            load = 0;
            if (mutex) mutex->unlock();
        }

        // 获取主机负载
        uint64_t Load()
        {
            uint64_t _load = 0;
            if (mutex) mutex->lock();
            _load = load;
            if (mutex) mutex->unlock();

            return _load;
        }

    public:
        std::string ip;    // 编译服务的ip
        int port;          // 编译服务的port
        uint64_t load;     // 编译服务的负载
        std::mutex *mutex; // mutex禁止拷贝的，使用指针
    };

    const std::string service_machine = "./conf/service_machine.conf";
    // 负载均衡模块
    class LoadBalance
    {
    public:
        LoadBalance()
        {
            assert(LoadConf(service_machine));
            LOG(INFO) << "加载 " << service_machine << " 成功" << "\n";
        }

        ~LoadBalance() {}

    public:
        // 加载配置文件
        bool LoadConf(const std::string machine_conf)
        {
            ifstream in(machine_conf);
            if (!in.is_open())
            {
                LOG(FATAL) << " 加载: " << machine_conf << " 失败" << "\n";
                return false;
            }

            std::string line;
            while (getline(in, line))
            {
                std::vector<std::string> tokens;
                StringUtil::SplitString(line, &tokens, ":");
                if (tokens.size() != 2)
                {
                    LOG(WARNING) << " 切分 " << line << " 失败" << "\n";
                    continue;
                }

                Machine m;
                m.ip = tokens[0];
                m.port = atoi(tokens[1].c_str());
                m.load = 0;
                m.mutex = new std::mutex();

                _online.push_back(_machines.size());
                _machines.push_back(m);
            }

            in.close();
            return true;
        }

        // id: 输出型参数
        // m : 输出型参数
        bool SmartChoice(int *id, Machine **m)
        {
            // 1. 使用选择好的主机(更新该主机的负载)
            // 2. 需要可能离线该主机
            // 轮询+hash
            _mutex.lock();

            int online_num = _online.size();
            if (online_num == 0)
            {
                _mutex.unlock();
                LOG(FATAL) << " 所有的后端编译主机已经离线" << "\n";
                return false;
            }

            // 通过遍历的方式，找到所有负载最小的机器
            *id = _online[0];
            *m = &_machines[_online[0]];
            uint64_t min_load = _machines[_online[0]].Load();
            for (int i = 1; i < online_num; ++i)
            {
                uint64_t curr_load = _machines[_online[i]].Load();
                if (min_load > curr_load)
                {
                    min_load = curr_load;
                    *id = _online[i];
                    *m = &_machines[_online[i]];
                }
            }

            _mutex.unlock();
            return true;
        }

        void OfflineMachine(int which)
        {
            _mutex.lock();

            for (auto iter = _online.begin(); iter != _online.end(); ++iter)
            {
                if (*iter == which)
                {
                    _machines[which].ResetLoad();
                    // 已找到要离线的主机
                    _online.erase(iter);
                    _offline.push_back(which);
                    break; // 因为直接使用 break 所以不需要考虑迭代器失效问题
                }
            }

            _mutex.unlock();
        }

        // 上线主机
        void OnlineMachine()
        {
            _mutex.lock();
            _online.insert(_online.end(), _offline.begin(), _offline.end());
            _offline.erase(_offline.begin(), _offline.end());
            _mutex.unlock();

            LOG(INFO) << "所有主机已上线" << "\n";
        }

        // for test
        void ShowMachines()
        {
            _mutex.lock();
            std::cout << "当前在线主机列表: ";
            for (auto &id : _online)
            {
                std::cout << id << " ";
            }
            std::cout << std::endl;
            std::cout << "当前离线主机列表: ";
            for (auto &id : _offline)
            {
                std::cout << id << " ";
            }
            std::cout << std::endl;
            _mutex.unlock();
        }

    private:
        // 提供编译服务的所有的主机
        // 每一台主机都有自己的下标，充当当前主机的id
        std::vector<Machine> _machines;
        std::vector<int> _online;  // 所有在线的主机id
        std::vector<int> _offline; // 所有离线的主机id
        std::mutex _mutex;         // 保证LoadBalance它的数据安全
    };

    // 核心业务逻辑控制器
    class Control
    {
    public:
        Control() {}
        ~Control() {}

    public:
        // 根据题目数据构建网页
        //  html: 输出型参数
        bool AllQuestions(string *html)
        {
            bool ret = true;
            vector<struct Question> all;
            if (_model.GetAllQuestions(&all))
            {
                sort(all.begin(), all.end(), [](const struct Question &q1, const struct Question &q2)
                     { return atoi(q1.number.c_str()) < atoi(q2.number.c_str()); });

                // 获取题目信息成功，将所有的题目数据构建成网页
                _view.AllExpandHtml(all, html);
            }
            else
            {
                *html = "获取题目失败, 形成题目列表失败";
                ret = false;
            }
            return ret;
        }

        bool Question(const string number, string *html)
        {
            bool ret = true;
            struct Question q;
            if (_model.GetOneQuestion(number, &q))
            {
                // 获取指定题目信息成功，将所有的题目数据构建成网页
                _view.OneExpandHtml(q, html);
            }
            else
            {
                *html = "指定题目: " + number + " 不存在!";
                ret = false;
            }
            return ret;
        }

        // code: #include...
        // input: ""
        void Judge(const std::string &number, const std::string in_json, std::string *out_json)
        {
            // 0. 根据题目编号，直接拿到对应的题目细节
            struct Question q;
            _model.GetOneQuestion(number, &q);

            // 1. in_json进行反序列化，得到题目的id，得到用户提交源代码，input
            Json::Reader reader;
            Json::Value in_value;
            reader.parse(in_json, in_value);
            std::string code = in_value["code"].asString();

            // 2. 重新拼接用户代码+测试用例代码，形成新的代码
            Json::Value compile_value;
            compile_value["input"] = in_value["input"].asString();
            compile_value["code"] = code + "\n" + q.tail;
            compile_value["cpu_limit"] = q.cpu_limit;
            compile_value["mem_limit"] = q.mem_limit;

            Json::StyledWriter writer;
            std::string compile_string = writer.write(compile_value);

            // 3. 选择负载最低的主机(差错处理)
            // 规则: 一直选择，直到主机可用，否则，就是全部挂掉
            while (true)
            {
                int id = 0;
                Machine *m = nullptr;
                if (!_loadbalance.SmartChoice(&id, &m))
                {
                    break;
                }

                // 4. 然后发起http请求，得到结果
                httplib::Client cli(m->ip, m->port);
                m->IncLoad();
                LOG(INFO) << " 选择主机成功, 主机id: " << id << " 详情: " << m->ip << ":" << m->port << " 当前主机的负载是: " << m->Load() << "\n";
                if (auto res = cli.Post("/compile_and_run", compile_string, "application/json;charset=utf-8"))
                {
                    // 5. 将结果赋值给out_json
                    if (res->status == 200)
                    {
                        *out_json = res->body;
                        m->DecLoad();
                        LOG(INFO) << "请求编译和运行服务成功..." << "\n";
                        break;
                    }

                    m->DecLoad();
                }
                else
                {
                    // 请求失败
                    LOG(ERROR) << " 当前请求的主机id: " << id << " 详情: " << m->ip << ":" << m->port << " 可能已经离线" << "\n";
                    _loadbalance.OfflineMachine(id);
                    _loadbalance.ShowMachines(); // 调试
                }
            }
        }

    private:
        Model _model;           // 提供后台数据
        View _view;             // 提供html渲染功能
        LoadBalance _loadbalance; // 核心负载均衡器
    };
}
