#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>

// 设置日志文件路径及保留天数
#define DAY 30                                                 // 日志文件保留天数
#define LOG_PATH "/usr/local/bin/netfileorstring-server/logs" // 日志文件路径
/**
 * @brief 读取文件内容并返回字符串
 */
char* read_file(const char* filename);

/**
 * @brief 写入文件内容
 */
int write_file(const char* filename, const char* content);

/**
 * @brief 获取格式化后系统当前时间
 *
 * @param flag 时间格式标志：0 返回值格式为：yyyymmddhhMMss共十四字节。flag=1 ：返回值格式为：yyyy/mm/dd hh:MM:ss共十九字节。 flag=2 :返回 yyyy-mm-dd 共计8字节
 * @param result 接收返回结果的缓冲。
 * @return char* result 返回结果14字节或19字节时间
 */
int ERR_LOG(const char *format, ...);

/**
 * @brief 删除指定路径下超过指定天数的日志文件
 *
 * @param log_path 日志文件所在目录
 * @param day 超过多少天的日志文件将被删除
 * @return int 返回0表示成功，其他表示失败
 */
char delete_oldlog(char *log_path, int day);

/**
 * @brief 将标准时间字符串转换为时间戳
 *
 * @param str_time 标准时间字符串，格式为 "YYYY-MM-DD HH:MM:SS"
 * @return int 返回对应的时间戳
 */
char *sysUsecTime(int flag, char *result);

/**
 * @brief 将标准时间字符串转换为时间戳
 *
 * @param str_time 标准时间字符串，格式为 "YYYY-MM-DD" 或 "YYYY-MM-DD HH:MM:SS"
 * @return int 返回对应的时间戳
 */
int standard_to_stamp(char *str_time);



#endif // UTILS_H