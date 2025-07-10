import os
import time
import numpy as np
from factorReader import FactorReader #不压缩
from factorReaderGZ import FactorReaderGZ #原始GZ方法

def run_comparison():
    data_path = "../data"
    gz_name = "factors_gz.bin.gz"
    raw_name = "factors_raw.bin"

    # 1. GZ压缩读取测试（含预热）
    print("=== 测试GZ压缩读取 ===")
    # 预热：第一次读取（结果不统计）
    gz_reader_warmup = FactorReaderGZ(data_path, gz_name)
    gz_reader_warmup.read()
    del gz_reader_warmup  # 释放资源

    # 正式测试：第二次读取（统计耗时）
    gz_reader = FactorReaderGZ(data_path, gz_name)
    start = time.time()
    gz_cols, gz_data = gz_reader.read()
    gz_time = time.time() - start
    print(f"GZ读取耗时：{gz_time:.3f}秒")
    print(f"GZ文件大小：{os.path.getsize(gz_reader.file_path)/1024/1024:.2f}MB")

    # 2. 原始+内存映射读取测试（含预热）
    print("\n=== 测试原始+内存映射读取 ===")
    # 预热：第一次读取（结果不统计）
    raw_reader_warmup = FactorReader(data_path, raw_name)
    raw_reader_warmup.read(memmap=True)
    del raw_reader_warmup  # 释放资源

    # 正式测试：第二次读取（统计耗时）
    raw_reader = FactorReader(data_path, raw_name)
    start = time.time()
    raw_cols, raw_data = raw_reader.read(memmap=True)
    raw_time = time.time() - start
    print(f"内存映射读取耗时：{raw_time:.3f}秒")
    print(f"原始文件大小：{os.path.getsize(raw_reader.file_path)/1024/1024:.2f}MB")

    # 3. 计算性能提升
    improvement = (gz_time - raw_time) / gz_time * 100
    print(f"\n性能提升：{improvement:.2f}%（内存映射 vs GZ）")

if __name__ == "__main__":
    run_comparison()