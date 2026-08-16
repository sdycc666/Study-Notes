#define _POSIX_C_SOURCE 199309L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <mysql/mysql.h>
#include <stdbool.h>

#define HOST "127.0.0.1"
#define USER "tom"
#define PASSWD "123456"
#define DB "hr"
#define PORT 3306

#define RING_SIZE 1024          // 缓冲区容量
#define BATCH_SIZE 20           // 批量写入最大条数
#define WAIT_TIMEOUT_MS 1000
#define SAMPLE_INTERVAL_SEC 5   // 两次 /proc/stat 采样的间隔

typedef struct
{
    int sensor_id;
    double value;   // 存放 CPU 使用率 (%)
    time_t ts;
} SensorRecord;

// ---------- 环形缓冲区 ----------
typedef struct
{
    SensorRecord data[RING_SIZE];
    int head;
    int count;
} RingBuffer;

void ring_init(RingBuffer *rb)
{
    rb->head = 0;
    rb->count = 0;
}

int ring_is_full(RingBuffer *rb)
{
    return rb->count == RING_SIZE;
}

int ring_push(RingBuffer *rb, SensorRecord rec)
{
    if (ring_is_full(rb))
        return -1;
    int tail = (rb->head + rb->count) % RING_SIZE;
    rb->data[tail] = rec;
    rb->count++;
    return 0;
}

int ring_pop_batch(RingBuffer *rb, SensorRecord *out, int max)
{
    int n = (rb->count < max) ? rb->count : max;
    for (int i = 0; i < n; i++)
    {
        int idx = (rb->head + i) % RING_SIZE;
        out[i] = rb->data[idx];
    }
    rb->head = (rb->head + n) % RING_SIZE;
    rb->count -= n;
    return n;
}

// ---------- /proc/stat CPU 采样 ----------
// 只取前 7 个字段: user nice system idle iowait irq softirq
typedef struct
{
    long long user;
    long long nice;
    long long system;
    long long idle;
    long long iowait;
    long long irq;
    long long softirq;
} CpuStats;

// 读取 /proc/stat 第一行（cpu 聚合行）的 jiffies
int read_cpu_stats(CpuStats *s)
{
    FILE *fp = fopen("/proc/stat", "r");
    if (!fp)
    {
        perror("open /proc/stat failed");
        return -1;
    }

    char line[512];
    if (!fgets(line, sizeof(line), fp))
    {
        fclose(fp);
        fprintf(stderr, "read /proc/stat empty\n");
        return -1;
    }
    fclose(fp);

    // 第一行是 "cpu  user nice system idle iowait irq softirq ..."
    if (sscanf(line, "cpu %lld %lld %lld %lld %lld %lld %lld",
               &s->user, &s->nice, &s->system, &s->idle,
               &s->iowait, &s->irq, &s->softirq) != 7)
    {
        fprintf(stderr, "parse /proc/stat failed: %s", line);
        return -1;
    }
    return 0;
}

// 基于两次采样的增量计算 CPU 使用率(%)
// 总时间 = user+nice+system+idle+iowait+irq+softirq
// 工作时间 = 总时间 - idle
// 使用率 = (工作时间增量 / 总时间增量) * 100
double calc_cpu_usage(const CpuStats *prev, const CpuStats *cur)
{
    long long total_prev = prev->user + prev->nice + prev->system +
                           prev->idle + prev->iowait + prev->irq + prev->softirq;
    long long total_cur  = cur->user + cur->nice + cur->system +
                           cur->idle + cur->iowait + cur->irq + cur->softirq;

    long long work_prev = total_prev - prev->idle;
    long long work_cur  = total_cur  - cur->idle;

    long long total_delta = total_cur - total_prev;
    long long work_delta  = work_cur  - work_prev;

    if (total_delta <= 0)
        return 0.0;

    return (double)work_delta / (double)total_delta * 100.0;
}

// ---------- MySQL 批量写入 ----------
void build_insert_sql(int n, char *sql)
{
    strcpy(sql, "INSERT INTO sensor_data(sensor_id, ts, value) VALUES ");
    for (int i = 0; i < n; i++)
    {
        if (i > 0)
            strcat(sql, ",");
        strcat(sql, "(?,?,?)");
    }
}

void time_to_mysql_time(time_t t, MYSQL_TIME *mt)
{
    struct tm tm_buf;
    localtime_r(&t, &tm_buf);
    memset(mt, 0, sizeof(*mt));
    mt->year   = tm_buf.tm_year + 1900;
    mt->month  = tm_buf.tm_mon + 1;
    mt->day    = tm_buf.tm_mday;
    mt->hour   = tm_buf.tm_hour;
    mt->minute = tm_buf.tm_min;
    mt->second = tm_buf.tm_sec;
}

int batch_insert(MYSQL *conn, SensorRecord *batch, int n)
{
    if (n <= 0)
        return 0;

    char sql[4096];
    build_insert_sql(n, sql);

    MYSQL_STMT *stmt = mysql_stmt_init(conn);
    if (!stmt)
    {
        fprintf(stderr, "mysql_stmt_init failed\n");
        return -1;
    }

    if (mysql_stmt_prepare(stmt, sql, strlen(sql)) != 0)
    {
        fprintf(stderr, "prepare failed: %s\n", mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        return -1;
    }

    int sensor_ids[BATCH_SIZE];
    MYSQL_TIME times[BATCH_SIZE];
    double values[BATCH_SIZE];

    for (int i = 0; i < n; i++)
    {
        sensor_ids[i] = batch[i].sensor_id;
        time_to_mysql_time(batch[i].ts, &times[i]);
        values[i] = batch[i].value;
    }

    MYSQL_BIND bind[3 * BATCH_SIZE];
    memset(bind, 0, sizeof(bind));

    bool is_null[3 * BATCH_SIZE];
    memset(is_null, 0, sizeof(is_null));

    for (int i = 0; i < n; i++)
    {
        bind[i * 3].buffer_type = MYSQL_TYPE_LONG;
        bind[i * 3].buffer = &sensor_ids[i];
        bind[i * 3].is_null = &is_null[i * 3];

        bind[i * 3 + 1].buffer_type = MYSQL_TYPE_DATETIME;
        bind[i * 3 + 1].buffer = &times[i];
        bind[i * 3 + 1].is_null = &is_null[i * 3 + 1];

        bind[i * 3 + 2].buffer_type = MYSQL_TYPE_DOUBLE;
        bind[i * 3 + 2].buffer = &values[i];
        bind[i * 3 + 2].is_null = &is_null[i * 3 + 2];
    }

    if (mysql_stmt_bind_param(stmt, bind) != 0)
    {
        fprintf(stderr, "bind_param failed: %s\n", mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        return -1;
    }

    mysql_autocommit(conn, 0);  // 关闭自动提交，开启事务

    if (mysql_stmt_execute(stmt) != 0)
    {
        fprintf(stderr, "execute failed: %s\n", mysql_stmt_error(stmt));
        mysql_rollback(conn);       // 执行失败，回滚
        mysql_autocommit(conn, 1);  // 恢复自动提交
        mysql_stmt_close(stmt);     // 关闭语句句柄
        return -1;                  // 失败必须返回，不再继续 commit
    }

    mysql_commit(conn);
    mysql_autocommit(conn, 1);
    mysql_stmt_close(stmt);
    return 0;
}

int main(int argc, char const *argv[])
{
    MYSQL *conn = mysql_init(NULL);
    if (!conn)
    {
        fprintf(stderr, "mysql_init failed\n");
        return 1;
    }

    if (!mysql_real_connect(conn, HOST, USER, PASSWD, DB, PORT, NULL, 0))
    {
        fprintf(stderr, "connect failed: %s\n", mysql_error(conn));
        mysql_close(conn);
        return 1;
    }

    RingBuffer rb;
    ring_init(&rb);

    SensorRecord rec;
    SensorRecord batch[BATCH_SIZE];

    struct timespec last_write;
    clock_gettime(CLOCK_MONOTONIC, &last_write);

    // 第一次采样作为基准（单次读取无法计算使用率，必须有前值）
    CpuStats prev_stats, cur_stats;
    if (read_cpu_stats(&prev_stats) != 0)
    {
        mysql_close(conn);
        return 1;
    }

    int total_samples = 50;

    for (int i = 0; i < total_samples; i++)
    {
        // 间隔采样：等待后再读第二次，才能算出该时段内的 CPU 使用率
        sleep(SAMPLE_INTERVAL_SEC);

        if (read_cpu_stats(&cur_stats) != 0)
            continue;

        double usage = calc_cpu_usage(&prev_stats, &cur_stats);

        rec.sensor_id = 1;          // 1 代表整机 CPU 聚合
        rec.value = usage;          // 写入 CPU 使用率(%)
        rec.ts = time(NULL);

        // 环形缓冲区满则直接丢弃，不再写本地兜底文件
        if (ring_push(&rb, rec) != 0)
        {
            fprintf(stderr, "ring buffer full, drop sample #%d\n", i);
        }

        struct timespec now;
        clock_gettime(CLOCK_MONOTONIC, &now);
        long elapsed_ms = (now.tv_sec - last_write.tv_sec) * 1000 +
                          (now.tv_nsec - last_write.tv_nsec) / 1000000;

        if (rb.count >= BATCH_SIZE ||
            (rb.count > 0 && elapsed_ms >= WAIT_TIMEOUT_MS))
        {
            int n = ring_pop_batch(&rb, batch, BATCH_SIZE);
            if (n > 0)
            {
                if (batch_insert(conn, batch, n) != 0)
                {
                    fprintf(stderr, "batch insert failed\n");
                }
                clock_gettime(CLOCK_MONOTONIC, &last_write);
            }
        }

        // 当前采样作为下一次的前值
        prev_stats = cur_stats;
    }

    // 循环结束后冲刷剩余数据
    if (rb.count > 0)
    {
        int n = ring_pop_batch(&rb, batch, BATCH_SIZE);
        if (n > 0)
        {
            if (batch_insert(conn, batch, n) != 0)
                fprintf(stderr, "final batch insert failed\n");
        }
    }

    mysql_close(conn);
    return 0;
}



// gcc -o cpu_sensor cpu_sensor.c -lmysqlclient