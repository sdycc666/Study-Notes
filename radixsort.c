#include <stdio.h>
#include <stdlib.h>

// 找数组中最大数的位数
int MaxBit(int a[], int n) {
    int max = a[0];
    for (int i = 1; i < n; i++)
        if (a[i] > max) max = a[i];
    int d = 1;
    while (max / 10 > 0) {
        max /= 10;
        d++;
    }
    return d;
}

// 基数排序（LSD）
void RadixSort(int a[], int n) {
    int d = MaxBit(a, n);
    int *tmp = (int *)malloc(n * sizeof(int));
    int count[10]; // 桶计数器
    int radix = 1;
    for (int i = 1; i <= d; i++) {
        // 初始化计数器
        for (int j = 0; j < 10; j++) count[j] = 0;
        // 统计每个桶中的元素个数
        for (int j = 0; j < n; j++) {
            int k = (a[j] / radix) % 10;
            count[k]++;
        }
        // 计算桶的结束位置
        for (int j = 1; j < 10; j++)
            count[j] += count[j - 1];
        // 按当前位分配到桶中（倒序处理保证稳定）
        for (int j = n - 1; j >= 0; j--) {
            int k = (a[j] / radix) % 10;
            tmp[--count[k]] = a[j];
        }
        // 复制回原数组
        for (int j = 0; j < n; j++)
            a[j] = tmp[j];
        radix *= 10;
    }
    free(tmp);
}

int main() {
    int a[] = {49, 38, 65, 97, 76, 13, 27, 49};
    int n = sizeof(a) / sizeof(a[0]);
    RadixSort(a, n);
    for (int i = 0; i < n; i++)
        printf("%d ", a[i]);
    return 0;
}