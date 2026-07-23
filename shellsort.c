#include <stdio.h>

// 希尔排序（缩小增量排序）
void ShellSort(int a[], int n) {
    int i, j, dk, temp;
    // 初始增量为n/2，每次减半
    for (dk = n / 2; dk > 0; dk /= 2) {
        // 对每个子序列做直接插入排序
        for (i = dk; i < n; i++) {
            temp = a[i];
            for (j = i - dk; j >= 0 && a[j] > temp; j -= dk)
                a[j + dk] = a[j];
            a[j + dk] = temp;
        }
    }
}

int main() {
    int a[] = {49, 38, 65, 97, 76, 13, 27, 49};
    int n = sizeof(a) / sizeof(a[0]);
    ShellSort(a, n);
    for (int i = 0; i < n; i++)
        printf("%d ", a[i]);
    return 0;
}