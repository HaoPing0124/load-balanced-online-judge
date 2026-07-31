#pragma once
// MySQL 版本
#include "../common/util.hpp"
#include "../common/log.hpp"
#include "include/mysql.h"

#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <fstream>
#include <cstdlib>
#include <cassert>

// 根据题目list文件，加载所有的题目信息到内存中
// model: 主要用来和数据进行交互，对外提供访问数据的接口

namespace ns_model2
{
    using namespace std;
    using namespace ns_log;
    using namespace ns_util;

    struct Question
    {
        std::string number; // 题目编号，唯一
        std::string title;  // 题目的标题
        std::string star;   // 难度: 简单 中等 困难
        std::string desc;   // 题目的描述
        std::string header; // 题目预设给用户在线编辑器的代码
        std::string tail;   // 题目的测试代码，需要与 header 拼接形成完整代码
        int cpu_limit;      // 题目的 CPU 时间限制，单位为秒
        int mem_limit;      // 题目的空间限制，单位为 KB
    };

    const std::string questions = "questions";
    const std::string host = "127.0.0.1";
    const std::string user = "oj_client";
    const std::string db = "oj";
    const int port = 3306;

    class Model
    {
    public:
        Model() {}
        ~Model() {}

    public:
        bool QueryMySql(const std::string &sql, vector<Question> *out)
        {
            // 检查输出参数，防止对空指针进行访问
            if (out == nullptr)
            {
                LOG(ERROR) << "QueryMySql 的输出参数 out 为空\n";
                return false;
            }

            // 清空旧数据，防止同一个 vector 多次查询时数据不断累积
            out->clear();

            // 从环境变量中读取数据库密码，避免将密码上传到公开仓库
            const char *passwd = std::getenv("OJ_MYSQL_PASSWD");
            if (passwd == nullptr)
            {
                LOG(ERROR) << "没有设置 OJ_MYSQL_PASSWD 环境变量\n";
                return false;
            }

            // 初始化一个 MySQL 连接句柄
            MYSQL *my = mysql_init(nullptr);
            if (my == nullptr)
            {
                LOG(ERROR) << "初始化 MySQL 连接句柄失败\n";
                return false;
            }

            // 连接数据库
            if (mysql_real_connect(my, host.c_str(), user.c_str(),
                                   passwd, db.c_str(), port, nullptr, 0) == nullptr)
            {
                LOG(ERROR) << "连接数据库失败，错误码: " << mysql_errno(my) << "，错误信息: " << mysql_error(my) << "\n";

                mysql_close(my);
                return false;
            }

            LOG(INFO) << "连接数据库成功!" << "\n";

            // 设置当前连接使用 utf8mb4 编码
            if (mysql_set_character_set(my, "utf8mb4") != 0)
            {
                LOG(ERROR) << "设置数据库字符集失败，错误码: " << mysql_errno(my) << "，错误信息: " << mysql_error(my) << "\n";

                mysql_close(my);
                return false;
            }

            // 执行sql语句
            if (mysql_query(my, sql.c_str()) != 0)
            {
                LOG(ERROR) << "SQL 执行失败，SQL: " << sql << "，错误码: " << mysql_errno(my) << "，错误信息: " << mysql_error(my) << "\n";
                return false;
            }

            // 获取 SELECT 查询产生的结果集
            MYSQL_RES *res = mysql_store_result(my);
            if (res == nullptr)
            {
                LOG(ERROR) << "获取查询结果失败，错误码: " << mysql_errno(my) << "，错误信息: " << mysql_error(my) << "\n";

                mysql_close(my);
                return false;
            }

            // 分析结果
            int rows = mysql_num_rows(res);   // 获得行数量
            int cols = mysql_num_fields(res); // 获得列数量

            if (cols != 8)
            {
                LOG(ERROR) << "查询结果字段数量错误，期望字段数量为 8，实际字段数量为: " << cols << "\n";
                mysql_free_result(res);
                mysql_close(my);
                return false;
            }

            Question q;

            for (int i = 0; i < rows; ++i)
            {
                MYSQL_ROW row = mysql_fetch_row(res);

                // 数据库字段有可能是 NULL，所以读取前进行判断
                q.number = row[0] == nullptr ? "" : row[0];
                q.title = row[1] == nullptr ? "" : row[1];
                q.star = row[2] == nullptr ? "" : row[2];
                q.desc = row[3] == nullptr ? "" : row[3];
                q.header = row[4] == nullptr ? "" : row[4];
                q.tail = row[5] == nullptr ? "" : row[5];

                q.cpu_limit = row[6] == nullptr ? 0 : std::atoi(row[6]);
                q.mem_limit = row[7] == nullptr ? 0 : std::atoi(row[7]);

                out->push_back(q);
            }

            // 释放结果空间
            mysql_free_result(res);

            // 关闭 MySQL 连接
            mysql_close(my);

            return true;
        }

        bool GetAllQuestions(vector<Question> *out)
        {
            std::string sql = "select * from ";
            sql += questions;
            return QueryMySql(sql, out);
        }

        bool GetOneQuestion(const std::string &number, Question *q)
        {
            if (q == nullptr)
            {
                LOG(ERROR) << "GetOneQuestion 的输出参数 q 为空\n";
                return false;
            }

            if (number.empty())
            {
                LOG(ERROR) << "题目编号为空\n";
                return false;
            }

            // number 会被直接拼接到 SQL 中，因此必须保证它只包含数字
            for (auto ch : number)
            {
                if (ch < '0' || ch > '9')
                {
                    LOG(ERROR) << "题目编号不合法: " << number << "\n";
                    return false;
                }
            }
            
            std::string sql = "select * from ";
            sql += questions;
            sql += " where number=";
            sql += number;

            vector<Question> result;
            if (QueryMySql(sql, &result))
            {
                if (result.size() == 1)
                {
                    *q = result[0];
                    return true;
                }
            }
            return false;
        }
    };
} // namespace ns_model
