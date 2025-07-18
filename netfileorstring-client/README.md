# 文本或文件上传工具

本项目包含一个客户端程序，用于将文本内容或文件通过网络发送到服务器端。

## 目录结构

```
netfileorstring-client
├── src
│   ├── main.c         # 程序入口
│   ├── client.c       # 客户端核心功能实现
│   ├── client.h       # 客户端函数声明
│   ├── utils.c        # 工具函数实现
│   └── utils.h        # 工具函数声明
├── Makefile           # 构建脚本
└── README.md          # 项目说明
```

## 编译方法

在项目根目录下执行：

```
make
```

编译完成后会生成可执行文件 `client`。

## 使用方法

```
./client <ip> <port> <file|text> <内容>
```

- 发送文件示例：
  ```
  ./client 127.0.0.1 8888 file ./test.txt
  ```
- 发送文本示例：
  ```
  ./client 127.0.0.1 8888 text "你好，世界"
  ```

## 功能说明

- 支持将本地文件内容发送到服务器端指定目录。
- 支持将文本内容发送到服务器端指定文本文件。
- 自动处理网络连接和数据传输。

## 依赖环境

- GCC 或兼容的 C 编译器
- 支持 POSIX Socket 的操作系统（如 Linux、macOS）

## 许可证

本项目采用 MIT 许可证。详情请参见 LICENSE 文件。