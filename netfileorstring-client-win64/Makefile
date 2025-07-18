CC = gcc
CFLAGS = -Wall -g -D_WIN32_WINNT=0x0600
LDFLAGS = -lws2_32 -L"E:/001 Monitor/005MSYS2/mingw64/lib" -lssl -lcrypto
INCLUDES = -Isrc -I"E:/001 Monitor/005MSYS2/mingw64/include"

# 源文件和目标文件
SRCS = src/main.c src/client.c
OBJS = $(SRCS:.c=.o)
TARGET = client.exe

# 默认目标
all: $(TARGET)

# 链接目标文件
$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET) $(LDFLAGS)

# 编译规则
%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# 清理
clean:
	rm -f src/*.o client.exe