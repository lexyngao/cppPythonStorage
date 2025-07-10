import os
import time
import struct
import gzip
import numpy as np

class FactorReaderGZ:
    def __init__(self, save_path, save_name):
        self.file_path = os.path.join(save_path, save_name)
        self.columns = []  # 列元数据
        self.data = None   # 读取的数据

    def read(self):
        """读取GZ压缩文件，返回列名和Numpy数组"""
        with gzip.open(self.file_path, 'rb') as f:
            # 1. 验证文件头
            magic = f.read(4)
            if magic != b'FACG':  # 对应C++的"FACG"魔法数
                raise ValueError("无效的GZ因子文件")

            # 2. 读取版本、列数、行数
            version = struct.unpack('<H', f.read(2))[0]  # <表示小端序
            n_cols = struct.unpack('<I', f.read(4))[0]
            n_rows = struct.unpack('<I', f.read(4))[0]

            # 3. 解析列元数据
            for _ in range(n_cols):
                # 列名
                name_len = struct.unpack('<H', f.read(2))[0]
                name = f.read(name_len).decode('utf-8')

                # 类型标志
                type_flag = struct.unpack('<B', f.read(1))[0]
                is_32bit = (type_flag & 0x01) != 0
                is_float = (type_flag & 0x02) != 0

                # 映射Numpy数据类型
                if is_float:
                    dtype = np.dtype(np.float32) if is_32bit else np.dtype(np.float64)  # 实例化
                else:
                    dtype = np.dtype(np.int32) if is_32bit else np.dtype(np.int64)  # 实例化

                self.columns.append({
                    'name': name,
                    'dtype': dtype,
                    'n_rows': n_rows
                })

            # 4. 读取数据（按列）
            arrays = []
            for col in self.columns:
                dtype = col['dtype']
                item_size = dtype.itemsize
                data_bytes = f.read(col['n_rows'] * item_size)
                # 从二进制解析为Numpy数组
                col_data = np.frombuffer(data_bytes, dtype=dtype)
                arrays.append(col_data)

            # 5. 合并为二维数组
            self.data = np.column_stack(arrays)

        # 返回列名和数据
        column_names = [col['name'] for col in self.columns]
        return column_names, self.data


# 测试GZ读取
if __name__ == "__main__":
    # 读取C++生成的GZ文件
    reader = FactorReaderGZ("../data", "factors_gz.bin.gz")

    try:
        start = time.time()
        cols, data = reader.read()
        print(f"GZ文件读取耗时：{time.time() - start:.3f}秒")
        print(f"列名：{cols}")
        print(f"数据形状：{data.shape}")
        print(f"前5行数据：\n{data[:5]}")
    except Exception as e:
        print(f"读取失败：{str(e)}")