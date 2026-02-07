/**
 * @file      main.c
 * @brief     程序入口文件
 * @author    huenrong (sgyhy1028@outlook.com)
 * @date      2026-02-01 15:40:26
 *
 * @copyright Copyright (c) 2026 huenrong
 *
 */

#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>

#include "hs_ring_buffer.h"

// 'b': 阻塞读; 'n': 非阻塞读
static char s_read_mode = 'b';
static hs_ring_buffer_t *s_test_ring_buffer = NULL;
static uint8_t s_write_data[] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A};

/**
 * @brief 获取当前 UTC 时间戳(毫秒)
 *
 * @param[out] timestamp_ms: 自 1970-01-01 00:00:00 UTC 起的毫秒数
 *
 * @return 0 : 成功
 * @return <0: 失败
 */
int hs_time_get_current_timestamp_ms(uint64_t *timestamp_ms)
{
    if (timestamp_ms == NULL)
    {
        return -1;
    }

    struct timespec time_spec = {0};
    if (clock_gettime(CLOCK_REALTIME, &time_spec) == -1)
    {
        return -2;
    }

    *timestamp_ms = time_spec.tv_sec * 1000 + time_spec.tv_nsec / 1000000;

    return 0;
}

/**
 * @brief 消费者线程
 *
 * @param[in] arg: 线程参数
 *
 * @return NULL
 */
static void *consumer_thread(void *arg)
{
    (void)arg;

    uint64_t current_timestamp = 0;
    hs_time_get_current_timestamp_ms(&current_timestamp);
    printf("[%ld] this is %s, waiting for read data from ring buffer, read mode: %c.\n\n", current_timestamp,
           __FUNCTION__, s_read_mode);

    while (true)
    {
        uint8_t read_data[10];
        memset(read_data, 0, sizeof(read_data));

        if (s_read_mode == 'b')
        {
            ssize_t ret = hs_ring_buffer_read(s_test_ring_buffer, read_data, sizeof(read_data));
            if (ret > 0)
            {
                current_timestamp = 0;
                hs_time_get_current_timestamp_ms(&current_timestamp);
                printf("[%ld] read data from ring buffer. data[len: %ld]: ", current_timestamp, ret);
                for (ssize_t i = 0; i < ret; i++)
                {
                    printf("0x%02X ", read_data[i]);
                }
                printf("\n\n");
            }
            else
            {
                printf("read data from ring buffer failed. ret: %ld\n", ret);
            }
        }
        else
        {
            ssize_t ret = hs_ring_buffer_read_with_timeout(s_test_ring_buffer, read_data, sizeof(read_data), 600);
            if (ret > 0)
            {
                current_timestamp = 0;
                hs_time_get_current_timestamp_ms(&current_timestamp);
                printf("[%ld] read data from ring buffer success. data[len: %ld]: ", current_timestamp, ret);
                for (ssize_t i = 0; i < ret; i++)
                {
                    printf("0x%02X ", read_data[i]);
                }
                printf("\n\n");
            }
            else if (ret == 0)
            {
                current_timestamp = 0;
                hs_time_get_current_timestamp_ms(&current_timestamp);
                printf("[%ld] read data from ring buffer timeout\n\n", current_timestamp);
            }
            else
            {
                current_timestamp = 0;
                hs_time_get_current_timestamp_ms(&current_timestamp);
                printf("[%ld] read data from ring buffer failed. ret: %ld\n", current_timestamp, ret);
            }
        }
    }

    return NULL;
}

/**
 * @brief 程序入口
 *
 * @param[in] argc: 参数个数
 * @param[in] argv: 参数列表
 *
 * @return 成功: 0
 * @return 失败: 其它
 */
int main(int argc, char *argv[])
{
    // 传参 'b' 或 'n' 指定读模式，分别为阻塞读和非阻塞读，默认为阻塞读
    if (argc > 1)
    {
        s_read_mode = argv[1][0];
        if ((s_read_mode != 'b') && (s_read_mode != 'n'))
        {
            printf("invalid read mode. use 'b' or 'n'\n");

            return -1;
        }
    }

    s_test_ring_buffer = hs_ring_buffer_create();
    if (s_test_ring_buffer == NULL)
    {
        printf("create ring buffer failed\n");

        return -1;
    }
    printf("create ring buffer success\n");

    int ret = hs_ring_buffer_init(s_test_ring_buffer, 1024);
    if (ret != 0)
    {
        printf("init ring buffer failed. ret: %d\n", ret);

        return -1;
    }
    printf("init ring buffer success\n");

    // 创建消费者线程
    pthread_t consumer_thread_id;
    ret = pthread_create(&consumer_thread_id, NULL, consumer_thread, NULL);
    if (ret != 0)
    {
        printf("create consumer thread failed. ret: %d\n", ret);

        return -1;
    }
    pthread_detach(consumer_thread_id);

    sleep(1);

    while (true)
    {
        size_t write_len = sizeof(s_write_data);

        uint64_t current_timestamp = 0;
        hs_time_get_current_timestamp_ms(&current_timestamp);
        printf("[%ld] write data to ring buffer. data[len: %ld]: ", current_timestamp, write_len);
        for (size_t i = 0; i < write_len; i++)
        {
            printf("0x%02X ", s_write_data[i]);
        }
        printf("\n");

        ssize_t ret = hs_ring_buffer_write(s_test_ring_buffer, s_write_data, write_len);
        if (ret != write_len)
        {
            printf("write data to ring buffer failed. ret: %ld\n", ret);
        }

        sleep(1);
    }

    return 0;
}
