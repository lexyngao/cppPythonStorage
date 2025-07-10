//
// Created by lexyn on 25-7-10.
//

#include "factorWriter.h"
#include <stdexcept> // 用于抛出异常

// 写入文件的核心实现
bool factorWriter::write() {
    // 检查数据完整性（必须有列定义和对应的数据）
    if (cols.empty() || data_ptrs.empty() || cols.size() != data_ptrs.size()) {
        throw std::runtime_error("列定义与数据不匹配！");
    }

    // 打开文件（二进制模式）
    std::ofstream file(file_path, std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("无法打开文件：" + file_path);
    }

    // -------------------------- 写入文件头 --------------------------
    // 1. 魔法数（4字节，用于识别文件类型，避免误读）
    const char magic[4] = {'F', 'A', 'C', 'T'}; // 自定义标识（Factor Data）
    file.write(magic, 4);

    // 2. 版本号（2字节，便于后续格式升级）
    const uint16_t version = 0x0100; // 版本1.0
    file.write(reinterpret_cast<const char*>(&version), 2);

    // 3. 标志位（2字节，记录压缩状态等信息）
    uint16_t flags = compress ? 0x0001 : 0x0000; // bit0=1表示启用压缩
    file.write(reinterpret_cast<const char*>(&flags), 2);

    // 4. 列数和行数（各4字节）
    uint32_t col_count = cols.size();
    file.write(reinterpret_cast<const char*>(&col_count), 4);
    file.write(reinterpret_cast<const char*>(&row_count), 4);

    // 5. 写入每列的元数据（列名+类型）
    for (const auto& col : cols) {
        // 列名长度（2字节，避免过长列名）
        uint16_t name_len = col.name.size();
        file.write(reinterpret_cast<const char*>(&name_len), 2);
        // 列名字符串
        file.write(col.name.data(), name_len);
        // 类型标识（1字节：bit0=是否32位，bit1=是否浮点）
        uint8_t type_flag = (col.is_32bit ? 0x01 : 0x00) | (col.is_float ? 0x02 : 0x00);
        file.write(reinterpret_cast<const char*>(&type_flag), 1);
    }

    // -------------------------- 写入数据区 --------------------------
    // 计算总数据大小（所有列的字节数之和）
    size_t total_bytes = 0;
    std::vector<size_t> col_bytes; // 每列的字节数
    for (const auto& col : cols) {
        size_t elem_size = col.is_32bit ? 4 : 8; // 32位=4字节，64位=8字节
        size_t bytes = row_count * elem_size;
        col_bytes.push_back(bytes);
        total_bytes += bytes;
    }

    // 准备原始数据（合并所有列的数据到连续内存）
    std::vector<char> raw_data(total_bytes);
    size_t offset = 0;
    for (size_t i = 0; i < cols.size(); ++i) {
        // 拷贝每列数据到raw_data（连续存储）
        memcpy(raw_data.data() + offset, data_ptrs[i], col_bytes[i]);
        offset += col_bytes[i];
    }

    // 压缩并写入，或直接写入原始数据
    if (compress) {
        // TODO:压缩：计算最大压缩后大小，分配缓冲区

    } else {
        // 不压缩：直接写入原始数据
        file.write(raw_data.data(), total_bytes);
    }

    return true;
}