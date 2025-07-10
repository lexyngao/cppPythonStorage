import os
import numpy as np
import time


class FactorReader:
    def __init__(self, save_path, save_name):
        """初始化读取器，指定文件路径和名称"""
        self.file_path = os.path.join(save_path, save_name)
        self.columns = []  # 存储列元数据（名称、数据类型等）
        self.data = None   # 存储读取的Numpy数组


    def read(self, memmap=True):
        """
        读取文件数据
        :param memmap: 是否使用内存映射（仅支持非压缩文件）
        :return: 列名列表 + Numpy数组
        """
        # 1. 解析文件头（元数据）
        with open(self.file_path, 'rb') as f:
            # 验证文件标识（魔法数）
            magic = f.read(4)
            if magic != b'FACT':
                raise ValueError("无效文件格式，不是因子数据文件！")

            # 读取版本号和标志位
            version = int.from_bytes(f.read(2), byteorder='little')
            flags = int.from_bytes(f.read(2), byteorder='little')
            compress = (flags & 0x0001) != 0  # 判断是否压缩

            # 读取列数和行数
            n_cols = int.from_bytes(f.read(4), byteorder='little')
            n_rows = int.from_bytes(f.read(4), byteorder='little')

            # 读取每列的元数据（名称和类型）
            for _ in range(n_cols):
                # 读取列名
                name_len = int.from_bytes(f.read(2), byteorder='little')
                name = f.read(name_len).decode('utf-8')

                # 读取类型标识（1字节：bit0=是否32位，bit1=是否浮点）
                type_flag = int.from_bytes(f.read(1), byteorder='little')
                is_32bit = (type_flag & 0x01) != 0
                is_float = (type_flag & 0x02) != 0

                # 映射到Numpy数据类型（确保dtype是实例）
                dtype = np.dtype(np.float32 if (is_float and is_32bit) else
                                 np.float64 if (is_float and not is_32bit) else
                                 np.int32 if (not is_float and is_32bit) else np.int64)

                self.columns.append({
                    'name': name,
                    'dtype': dtype,
                    'n_rows': n_rows,
                    'item_size': dtype.itemsize  # 提前计算并存储每个元素的字节数
                })

            # 记录数据区在文件中的偏移量（用于后续读取）
            data_offset = f.tell()

        # 2. 读取数据（支持内存映射）
        if memmap:
            # 内存映射仅支持非压缩文件
            if compress:
                raise ValueError("内存映射不支持压缩文件，请关闭压缩或使用常规读取")

            # 使用numpy.memmap直接映射文件数据区
            mm = np.memmap(
                self.file_path,
                dtype=np.uint8,  # 先按字节映射
                mode='r',        # 只读模式
                offset=data_offset  # 从数据区开始映射
            )

            # 按列解析数据
            arrays = []
            offset = 0
            for col in self.columns:
                dtype = col['dtype']
                item_size = dtype.itemsize  # 每个元素的字节数
                col_size = col['n_rows'] * item_size  # 整列的总字节数

                # 从内存映射中截取并转换为对应数据类型
                col_data = mm[offset:offset + col_size].view(dtype).reshape(-1)
                arrays.append(col_data)
                offset += col_size

            # 合并为二维数组（行数 x 列数）
            self.data = np.column_stack(arrays)

        else:
            # 常规读取（支持压缩文件）
            with open(self.file_path, 'rb') as f:
                f.seek(data_offset)  # 跳转到数据区

                if compress:
                    # TODO读取压缩数据并解压
                    # 直接读取原始数据
                    raw_data = f.read()
                else:
                    # 直接读取原始数据
                    raw_data = f.read()

            # 解析原始数据为Numpy数组
            arrays = []
            offset = 0
            for col in self.columns:
                dtype = col['dtype']
                item_size =  col['item_size']  # 使用预计算的字节数，确保是整数
                col_size = col['n_rows'] * item_size

                # 从字节流中截取并转换为对应数据类型
                col_data = np.frombuffer(
                    raw_data,
                    dtype=dtype,
                    count=col['n_rows'],
                    offset=offset
                )
                arrays.append(col_data)
                offset += col_size

            # 合并为二维数组
            self.data = np.column_stack(arrays)

        # 3. 返回列名和数据
        column_names = [col['name'] for col in self.columns]
        return column_names, self.data


# 测试代码
if __name__ == "__main__":
    # 读取C++生成的文件（路径与C++保持一致）
    reader = FactorReader("../data", "factors_raw.bin")

    try:
        # # 常规读取（支持压缩文件）
        # print("=== 常规读取 ===")
        # cols, data = reader.read(memmap=True)
        # print(f"列名：{cols}")
        # print(f"数据形状：{data.shape}")
        # print("数据内容：")
        # print(data)

        # 内存映射读取（仅支持非压缩文件，若C++写入时启用了压缩，需先关闭）
        # 如需测试，先修改C++代码中FactorWriter的compress参数为false，重新生成文件
        print("\n=== 内存映射读取 ===")
        start_time = time.time()
        cols_mmap, data_mmap = reader.read(memmap=True)
        print(f"读取耗时: {time.time() - start_time:.3f}秒")
        print(f"列名：{cols_mmap}")
        print(f"数据形状：{data_mmap.shape}")
        print(f"前5行数据: {data_mmap[:5]}")

    except Exception as e:
        print(f"读取失败：{str(e)}")