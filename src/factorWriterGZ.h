//
// Created by lexyn on 25-7-10.
//

#ifndef CPPPYTHONSTORAGE_factorWriterGZ_H
#define CPPPYTHONSTORAGE_factorWriterGZ_H

#pragma once
#include <string>
#include <vector>
#include <zlib.h>  // GZ压缩依赖

class factorWriterGZ {
private:
    std::string file_path;
    gzFile gz_file;  // GZ文件句柄

    // 存储列元数据
    struct Column {
        std::string name;
        bool is_float;   // 是否浮点型
        bool is_32bit;   // 是否32位
        size_t n_rows;   // 行数
    };
    std::vector<Column> columns;

    // 存储待写入的数据（按列）
    std::vector<std::vector<char>> column_data;

public:
    // 构造函数：指定输出路径
    factorWriterGZ(const std::string& save_path, const std::string& save_name);

    // 析构函数：关闭文件
    ~factorWriterGZ();

    // 添加列元数据（列名、是否浮点、是否32位）
    void add_column(const std::string& name, bool is_float, bool is_32bit);


    // 添加列数据（模板函数，支持float/double/int32_t/int64_t）
    template<typename T>
    void add_data(const std::vector<T>& data) {
        // 检查是否还有列需要填充数据
        if (columns.empty() || columns.size() <= column_data.size()) {
            throw std::runtime_error("未定义足够的列或数据列数超过定义的列数！");
        }

        // 获取当前列元数据
        Column& current_col = columns[column_data.size()];

        // 检查数据类型是否匹配列定义
        bool is_float = std::is_floating_point_v<T>;
        bool is_32bit = (sizeof(T) == 4);

        if (is_float != current_col.is_float || is_32bit != current_col.is_32bit) {
            throw std::runtime_error("数据类型与列定义不匹配！");
        }

        // 检查行数是否一致
        if (!column_data.empty() && data.size() != columns[0].n_rows) {
            throw std::runtime_error("列数据长度不一致！");
        }

        // 更新列的行数
        current_col.n_rows = data.size();

        // 将数据转换为char数组并存储
        const char* bytes = reinterpret_cast<const char*>(data.data());
        column_data.emplace_back(bytes, bytes + data.size() * sizeof(T));
    }

    // 写入并关闭文件
    bool write();
};


#endif //CPPPYTHONSTORAGE_factorWriterGZ_H
