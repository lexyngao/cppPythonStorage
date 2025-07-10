#include <iostream>
#include <vector>
#include "factorWriter.h"
#include <random>  // 新增头文件
#include "factorWriterGZ.h"

// 生成随机数据
std::vector<float> generate_random_floats(size_t count, float min_val, float max_val) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(min_val, max_val);

    std::vector<float> data(count);
    for (size_t i = 0; i < count; ++i) {
        data[i] = dis(gen);
    }
    return data;
}

std::vector<int32_t> generate_random_ints(size_t count, int32_t min_val, int32_t max_val) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(min_val, max_val);

    std::vector<int32_t> data(count);
    for (size_t i = 0; i < count; ++i) {
        data[i] = dis(gen);
    }
    return data;
}

std::vector<int64_t> generate_timestamps(size_t count, int64_t start_time) {
    std::vector<int64_t> data(count);
    for (size_t i = 0; i < count; ++i) {
        data[i] = start_time + i;
    }
    return data;
}

int main(int argc, char** argv) {
    try {
        // 解析命令行参数（默认1000万行）
        size_t num_rows = 10000000;
        bool compress = false;

        for (int i = 1; i < argc; ++i) {
            std::string arg = argv[i];
            if (arg == "--rows" && i + 1 < argc) {
                num_rows = std::stoul(argv[++i]);
            } else if (arg == "--compress") {
                compress = true;
            }
        }
        // 1. 创建写入器（保存路径为当前目录的data文件夹，文件名factors_raw.bin，不启用压缩）
        factorWriter writer("./data", "factors_raw.bin", compress);

        // 2. 定义列（先定义列的元数据）
        writer.add_column("price", true, true);    // 列名price，浮点型，32位（float）
        writer.add_column("volume", false, true);  // 列名volume，整型，32位（int32_t）
        writer.add_column("timestamp", false, false); // 列名timestamp，整型，64位（int64_t）

        // 3. 准备测试数据（3列）
        // 生成大规模测试数据（例如：1000万行）
        std::vector<float> prices = generate_random_floats(num_rows, 10.0f, 1000.0f);
        std::vector<int32_t> volumes = generate_random_ints(num_rows, 100, 1000000);
        std::vector<int64_t> timestamps = generate_timestamps(num_rows, 1630000000);

        // 4. 添加数据（注意：添加顺序必须与定义列的顺序一致）
        writer.add_data(prices);
        writer.add_data(volumes);
        writer.add_data(timestamps);

        // 5. 写入文件
        std::cout << "正在写入raw文件..." << std::endl;
        auto start_time = std::chrono::high_resolution_clock::now();
        if (writer.write()) {
            std::cout << "数据写入成功！文件路径：./data/factors_raw.bin" << std::endl;
        } else {
            std::cerr << "数据写入失败！" << std::endl;
        }
        auto end_time = std::chrono::high_resolution_clock::now();
        std::cout << "raw文件写入完成，耗时: "
                  << std::chrono::duration_cast<std::chrono::seconds>(end_time - start_time).count()
                  << " 秒" << std::endl;



        //(可选)
        // 1.创建gz写入器
        factorWriterGZ writerGz("./data", "factors_gz.bin.gz");
        // 2. 定义列（先定义列的元数据）
        writerGz.add_column("price", true, true);    // 列名price，浮点型，32位（float）
        writerGz.add_column("volume", false, true);  // 列名volume，整型，32位（int32_t）
        writerGz.add_column("timestamp", false, false); // 列名timestamp，整型，64位（int64_t）
        // 3. 添加数据（注意：添加顺序必须与定义列的顺序一致）
        writerGz.add_data(prices);
        writerGz.add_data(volumes);
        writerGz.add_data(timestamps);
        // 4. 写入文件
        std::cout << "正在写入gz文件..." << std::endl;
        start_time = std::chrono::high_resolution_clock::now();
        if (writerGz.write()) {
            std::cout << "数据写入成功！文件路径：./data/factors_gz.bin" << std::endl;
        } else {
            std::cerr << "数据写入失败！" << std::endl;
        }
        end_time = std::chrono::high_resolution_clock::now();
        std::cout << "gz文件写入完成，耗时: "
                  << std::chrono::duration_cast<std::chrono::seconds>(end_time - start_time).count()
                  << " 秒" << std::endl;


    } catch (const std::exception& e) {
        // 捕获并打印错误信息
        std::cerr << "错误：" << e.what() << std::endl;
        return 1; // 异常退出
    }

    return 0;
}