#pragma once

// 文件版本
#include "../common/util.hpp"
#include "../common/log.hpp"

#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <fstream>

// 根据题目list文件，加载所有的题目信息到内存中
// model: 主要用来和数据进行交互，对外提供访问数据的接口

namespace ns_model
{
    using namespace std;
    using namespace ns_log;
    using namespace ns_util;

    struct Question
    {
        std::string number; // 题目编号，唯一
        std::string title;  // 题目的标题
        std::string star;   // 难度: 简单 中等 困难
        int cpu_limit;      // 题目的 CPU 时间限制，单位为秒
        int mem_limit;      // 题目的空间限制，单位为 KB
        std::string desc;   // 题目的描述
        std::string header; // 题目预设给用户在线编辑器的代码
        std::string tail;   // 题目的测试代码，需要与 header 拼接形成完整代码
    };

    const std::string questions_list = "./questions/questions.list";
    const std::string questions_path = "./questions/";

    class Model
    {
    public:
        Model()
        {
            if (!LoadQuestionList(questions_list))
            {
                LOG(FATAL) << "加载题库失败" << "\n";
            }
        }

        bool LoadQuestionList(const string &question_list)
        {
            // 加载配置文件: questions/questions.list + 题目编号文件
            ifstream in(question_list);
            if (!in.is_open())
            {
                LOG(FATAL) << " 加载题库失败,请检查是否存在题库文件" << "\n";
                return false;
            }

            string line;
            while (getline(in, line))
            {
                vector<string> tokens;
                if (!StringUtil::SplitString(line, &tokens, " "))
                {
                    LOG(ERROR) << "切分题目配置失败" << "\n";
                    continue;
                }

                // 1 判断回文数 简单 1 30000
                if (tokens.size() != 5)
                {
                    LOG(WARNING) << "加载部分题目失败, 请检查文件格式" << "\n";
                    continue;
                }

                Question q;
                q.number = tokens[0];
                q.title = tokens[1];
                q.star = tokens[2];
                q.cpu_limit = atoi(tokens[3].c_str());
                q.mem_limit = atoi(tokens[4].c_str());

                string path = questions_path;
                path += q.number;
                path += "/";

                bool desc_ok = FileUtil::ReadFile(path + "desc.txt", &(q.desc), true);
                bool header_ok = FileUtil::ReadFile(path + "header.cpp", &(q.header), true);
                bool tail_ok = FileUtil::ReadFile(path + "tail.cpp", &(q.tail), true);

                if (!desc_ok || !header_ok || !tail_ok)
                {
                    LOG(WARNING) << "加载题目文件失败，题目编号：" << q.number << "\n";
                    continue;
                }

                auto result = _questions.insert(std::make_pair(q.number, q));
                if (!result.second)
                {
                    LOG(WARNING) << "题目编号重复：" << q.number << "\n";
                }
            }

            LOG(INFO) << "加载题库...成功!" << "\n";
            in.close();

            return true;
        }

        // 获取完整题目列表
        bool GetAllQuestions(vector<Question> *out)
        {
            if (_questions.size() == 0)
            {
                LOG(ERROR) << "用户获取题库失败" << "\n";
                return false;
            }

            if (out == nullptr)
            {
                LOG(ERROR) << "题目列表输出参数为空" << "\n";
                return false;
            }

            out->clear();
            for (const auto &q : _questions)
            {
                out->push_back(q.second); // first: key, second: value
            }

            return true;
        }

        // 获取指定题目
        bool GetOneQuestion(const std::string &number, Question *q)
        {
            const auto &iter = _questions.find(number);
            if (iter == _questions.end())
            {
                LOG(ERROR) << "用户获取题目失败, 题目编号: " << number << "\n";
                return false;
            }

            if (q == nullptr)
            {
                LOG(ERROR) << "题目输出参数为空" << "\n";
                return false;
            }

            (*q) = iter->second;
            return true;
        }

        ~Model() {}

    private:
        unordered_map<string, Question> _questions;
    };

} // namespace ns_model