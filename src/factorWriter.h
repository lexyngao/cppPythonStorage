//
// Created by lexyn on 25-7-10.
//

#ifndef CPPPYTHONSTORAGE_FACTORWRITER_H
#define CPPPYTHONSTORAGE_FACTORWRITER_H

#include <string>
#include <vector>
#include <fstream>  // 用于文件操作

// 列的元数据（记录列名、类型等信息）
struct ColumnMeta {
    std::string name;    // 列名（如"price"）
    bool is_float;       // true=浮点型（float/double），false=整型（int32/int64）
    bool is_32bit;       // true=32位，false=64位
};


class factorWriter {
private:
    std::string file_path;       // 完整文件路径（如"./data/factors.bin"）
    bool compress;               // 是否启用LZ4压缩
    std::vector<ColumnMeta> cols; // 所有列的元数据
    std::vector<const void*> data_ptrs; // 每列数据的指针（避免拷贝）
    size_t row_count = 0;        // 行数（所有列长度必须一致）

public:
    // 构造函数：初始化文件路径和压缩开关
    factorWriter(const std::string& save_path, const std::string& save_name, bool use_compress = false)
            : file_path(save_path + "/" + save_name), compress(use_compress) {}

    // 添加列定义（必须先定义列，再添加数据）
    void add_column(const std::string& name, bool is_float, bool is_32bit) {
        cols.push_back({name, is_float, is_32bit});
    }

    // 添加列数据（模板函数，支持float/double/int32_t/int64_t）
    template<typename T>
    void add_data(const std::vector<T>& data) {
        // 检查列数是否匹配（添加的数据数量必须等于列数）
        if (data_ptrs.size() >= cols.size()) {
            throw std::runtime_error("添加的数据列数超过定义的列数！");
        }
        // 检查行数是否一致（第一列数据决定行数，后续列必须相同）
        if (row_count == 0) {
            row_count = data.size();
        } else if (data.size() != row_count) {
            throw std::runtime_error("列数据长度不一致！");
        }
        // 保存数据指针（不拷贝数据，提高效率）
        data_ptrs.push_back(data.data());
    }

    // 核心：将数据写入文件
    bool write();

};



#endif //CPPPYTHONSTORAGE_FACTORWRITER_H
