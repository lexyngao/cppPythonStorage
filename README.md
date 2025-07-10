# 高效C++ Python文件交互插件

## 项目简介

本项目旨在为因子计算与机器学习流程提供高效的 C++ 与 Python 文件交互插件。通过优化数据存储与读取方式，实现 C++ 端高效写入、Python 端高效读取，满足大规模数据在多语言环境下的高性能流转需求。

---


## 功能说明

### C++ 部分

- **输入参数：**
  1. 列名（如字符串数组）
  2. 每一列的 vector，所有 vector 长度一致
  3. 数字类型是否以 32 位存储（如 float32/int32）
  4. save_path（文件保存路径）
  5. save_name（文件保存名称）
- **输出：**
  - 数据以高效格式持久化存储至硬盘

### Python 部分

- **输入参数：**
  1. save_path（文件保存路径）
  2. save_name（文件保存名称）
- **输出：**
  1. 列名（所有列名）
  2. Numpy ndarray（数据内容）

---

## 性能要求

- 以 `.gz` 文件实现的读写性能为基准，**读写速度提升 30% 以上**。

---

## 额外需求

- Python 端读取时，**支持以内存映射（memory mapping）方式读取**，进一步提升大文件的读取效率。

---

## 详细目录结构与文件说明

```text
cppPythonStorage/
├── src/                         # C++ 源码目录
│   ├── main.cpp                 # C++ 测试整体流程，包含生成模拟数据
│   ├── factorWriter.h           # C++ 写入接口头文件，采用不压缩二进制
│   ├── factorWriter.cpp         # C++ 写入接口实现，采用不压缩二进制
│   ├── factorWriterGZ.h         # C++ 使用GZ压缩写入头文件
│   ├── factorWriterGZ.cpp       # C++ 使用GZ压缩写入实现，实现以.gz格式压缩存储
│
├── python/                      # Python 相关代码
│   ├── factorReader.py          # Python 端内存映射读取模块
│   ├── factorReaderGZ.py        # Python 端.gz格式读取模块，压缩文件读取
│   ├── performanceComparison.py # 性能对比脚本，测试不同读写方式性能
│
├── data/                        # 测试数据与样例目录（当前为空，运行src/main.cpp生成的数据文件将存放此处）
│
├── CMakeLists.txt               # C++ 构建配置文件，定义编译规则
├── README.md                    # 项目说明文档（本文件）

```



## 使用说明

### C++ 端

运行src/main.cpp。可替换其中测试数据为所需数据。

### Python 端

memory mapping 方式：运行python/factorReader.py
对照组gz方式：运行python/factorReaderGZ.py

或可直接运行performanceComparison.py脚本查看阅读性能对比。

---

## 实验结果

=== 测试GZ压缩读取 ===
GZ读取耗时：0.909秒
GZ文件大小：78.70MB

=== 测试原始+内存映射读取 ===
内存映射读取耗时：0.234秒
原始文件大小：152.59MB

性能提升：74.23%（内存映射 vs GZ）

---