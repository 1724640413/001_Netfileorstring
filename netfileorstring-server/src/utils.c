#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <stdarg.h>
#include <dirent.h>
#include <errno.h>
#include "utils.h"

/**
 * @brief 读取文件内容并返回字符串
 *
 * @param filename 文件名
 * @return char* 返回文件内容的字符串，调用者需要释放内存
 */
char* read_file(const char* filename) {
    FILE* file = fopen(filename, "rb");
    if (!file) {
        perror("Failed to open file");
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    long filesize = ftell(file);
    fseek(file, 0, SEEK_SET);

    char* buffer = (char*)malloc(filesize + 1);
    if (!buffer) {
        perror("Failed to allocate memory");
        fclose(file);
        return NULL;
    }

    fread(buffer, 1, filesize, file);
    buffer[filesize] = '\0'; // Null-terminate the string
    fclose(file);
    return buffer;
}

/**
 * @brief 写入字符串到文件
 *
 * @param filename 文件名
 * @param data 要写入的字符串
 * @return int 返回0表示成功，其他表示失败
 */
int write_file(const char* filename, const char* data) {
    FILE* file = fopen(filename, "wb");
    if (!file) {
        perror("Failed to open file");
        return -1;
    }

    fwrite(data, sizeof(char), strlen(data), file);
    fclose(file);
    return 0;
}

/**
 * @brief 记录错误日志信息
 *
 * @param format 格式化字符串
 * @param ... 可变参数
 * @return int 返回0表示成功，其他表示失败
 */
int ERR_LOG(const char *format, ...)
{
    char cmd[1024] = {0};
    char time[20] = {0};
    char filename[28] = {0};
    char error_message[2048] = {0};
    va_list args;
    FILE *fp;

    // 判断路径文件夹是否存在，不存在创建
    if (access(LOG_PATH, F_OK))
    {
        sprintf(cmd, "/bin/mkdir -p %s", LOG_PATH);
        system(cmd);
    }

    sysUsecTime(2, time); // 获取格式化时间给time，2代表返回 YYYY-MM-DD 格式时间；
    time[10] = '\0';
    chdir(LOG_PATH);
    snprintf(filename, 28, "%s_monitor.log", time);

    va_start(args, format);
    vsnprintf(error_message, sizeof(error_message), format, args);
    va_end(args);

    fp = fopen(filename, "a+");
    if (fp != NULL)
    {
        sysUsecTime(1, time);
        fprintf(fp, "[ %s ] %s\n", time, error_message);
        fclose(fp);
        return 0;
    }

    return -1;
}

/**
 * @brief 清理过时日志文件
 *
 * @param log_path 日志文件路径
 * @param day 日志文件保存天数
 * @return int 返回0表示成功，非0表示失败
 * @author Wei.liu
 */
char delete_oldlog(char *log_path, int day)
{
    int num = 0;
    int f_time;
    int n_time;
    int s_time;
    char nowtime[11] = {0};
    DIR *dp;

    struct dirent *dirp;
    s_time = 60 * 60 * 24 * day;
    chdir(log_path);                // 移动工作目录到pilepath中
    memset(&dirp, 0, sizeof(dirp)); // 初始化结构体

    if ((dp = opendir(log_path)) == NULL)
    {
        char message[1024];
        sprintf(message, "opening directory log_path directory error. %d", errno);
        ERR_LOG(message);
        return -1;
    }
    while ((dirp = readdir(dp)) != NULL)
    {
        if (dirp->d_type == DT_REG) // DT_REG ,文件类型，8 ，常规文件
        {
            num++;
            if (strstr(dirp->d_name, "_status.log"))
            {
                dirp->d_name[10] = '\0';
                f_time = standard_to_stamp(dirp->d_name);

                sysUsecTime(2, nowtime);
                n_time = standard_to_stamp(nowtime);
                dirp->d_name[10] = '_';
                if (n_time - f_time >= s_time)
                {
                    remove(dirp->d_name);
                    ERR_LOG("%-3d [ %-20s ] , 是 %d 天内日志文件，已被清理", num, dirp->d_name, day);
                }
                else
                {
                    // ERR_LOG("%-3d [ %-20s ] ，是 %d 天内日志文件", num, dirp->d_name, day);
                }
            }
            else
            {
                ERR_LOG("%-3d [ %-20s ] ，不是statuslog文件,可自行判断是否清理", num, dirp->d_name);
            }
        }
        else
        {
            // sprintf(message, "%-3d [ %-20s ] ，不是常规文件, 文件类型为：%d", num, dirp->d_name, dirp->d_type);
            // ERR_LOG(message);
        }
    }
    closedir(dp);
    return 0;
}

/**
 * @brief 获取格式化后系统当前时间
 *
 * @param flag 时间格式标志：0 返回值格式为：yyyymmddhhMMss共十四字节。flag=1 ：返回值格式为：yyyy/mm/dd hh:MM:ss共十九字节。 flag=2 :返回 yyyy-mm-dd 共计8字节
 * @param result 接收返回结果的缓冲。
 * @return char* result 返回结果14字节或19字节时间
 * @author : Wei.liu
 */
char *sysUsecTime(int flag, char *result)
{
    time_t now_t;
    struct tm *now_tm;

    if (result == NULL)
        return NULL;

    now_t = time(NULL);
    now_tm = localtime(&now_t);
    if (flag == 0)
    {
        strftime(result, 15, "%Y%m%d%H%M%S", now_tm);
    }
    else if (flag == 2)
    {
        strftime(result, 11, "%Y-%m-%d%", now_tm);
    }
    else
    {
        strftime(result, 20, "%Y-%m-%d %H:%M:%S", now_tm);
    }

    return result;
}

/**
 * @brief 标准时间转换为时间戳
 *
 * @param str_time 入参标准时间格式：yyyy-mm-dd hh:MM:ss 或  yyyy-mm-dd ；
 * @return int				根据入参时间，转化出的时间戳；
 * @author      : Wei.liu
 */
int standard_to_stamp(char *str_time)
{
    struct tm stm;
    int iY, iM, iD, iH, iMin, iS;
    memset(&stm, 0, sizeof(stm)); // 初始化 stm 结构体；
    iY = atoi(str_time);          // atoi将字符串转化为整数；
    iM = atoi(str_time + 5);
    iD = atoi(str_time + 8);
    iH = atoi(str_time + 11);
    iMin = atoi(str_time + 14);
    iS = atoi(str_time + 17);
    stm.tm_year = iY - 1900;
    stm.tm_mon = iM - 1;
    stm.tm_mday = iD;
    stm.tm_hour = iH;
    stm.tm_min = iMin;
    stm.tm_sec = iS;
    // printf("%d-%0d-%0d %0d:%0d:%0d\n", iY, iM, iD, iH, iMin, iS);

    return (int)mktime(&stm);
}