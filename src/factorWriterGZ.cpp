//
// Created by lexyn on 25-7-10.
//

#include "factorWriterGZ.h"
#include <stdexcept>
#include <cstring>

factorWriterGZ::factorWriterGZ(const std::string &save_path, const std::string &save_name) {
    // 拼接完整路径
    file_path = save_path + "/" + save_name;
    // 打开GZ文件（"wb9"表示最高压缩级别）
    gz_file = gzopen(file_path.c_str(), "wb9");
    if (!gz_file) {
        throw std::runtime_error("无法打开GZ文件：" + file_path);
    }
}

factorWriterGZ::~factorWriterGZ() {
    if (gz_file) {
        gzclose(gz_file);
    }
}

void factorWriterGZ::add_column(const std::string &name, bool is_float, bool is_32bit) {
    columns.push_back({name, is_float, is_32bit, 0});  // 行数暂定为0，添加数据时更新
}


bool factorWriterGZ::write() {
    if (columns.empty() || column_data.empty()) {
        throw std::runtime_error("未添加列或数据");
    }

    // 1. 写入文件头（魔法数 + 版本）
    const char magic[4] = {'F', 'A', 'C', 'G'};  // 魔法数：FACT + G
    gzwrite(gz_file, magic, 4);
    uint16_t version = 1;  // 版本号
    gzwrite(gz_file, &version, sizeof(version));

    // 2. 写入列数和行数（所有列行数相同）
    uint32_t n_cols = columns.size();
    uint32_t n_rows = columns[0].n_rows;
    gzwrite(gz_file, &n_cols, sizeof(n_cols));
    gzwrite(gz_file, &n_rows, sizeof(n_rows));

    // 3. 写入每列元数据
    for (const auto& col : columns) {
        // 列名长度 + 列名
        uint16_t name_len = col.name.size();
        gzwrite(gz_file, &name_len, sizeof(name_len));
        gzwrite(gz_file, col.name.c_str(), name_len);

        // 类型标志（1字节：bit0=32位，bit1=浮点）
        uint8_t type_flag = 0;
        if (col.is_32bit) type_flag |= 0x01;
        if (col.is_float) type_flag |= 0x02;
        gzwrite(gz_file, &type_flag, sizeof(type_flag));
    }

    // 4. 写入数据（按列存储）
    for (const auto& data : column_data) {
        gzwrite(gz_file, data.data(), data.size());
    }

    // 关闭文件
    gzclose(gz_file);
    gz_file = nullptr;

    return true;
}