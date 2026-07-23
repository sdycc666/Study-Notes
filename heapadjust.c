#include <stdio.h>

// 调整堆（大顶堆）
void HeapAdjust(int a[], int s, int n) {
    int temp = a[s];
    for (int j = 2 * s + 1; j < n; j = 2 * j + 1) {
        if (j + 1 < n && a[j] < a[j + 1]) // 找较大孩子
            j++;
        if (temp >= a[j]) break;          // 已满足堆性质
        a[s] = a[j];
        s = j;
    }
    a[s] = temp;
}

// 堆排序
void HeapSort(int a[], int n) {
    int i, temp;
    // 建初始堆
    for (i = n / 2 - 1; i >= 0; i--)
        HeapAdjust(a, i, n);
    // 堆排序
    for (i = n - 1; i > 0; i--) {
        temp = a[0];
        a[0] = a[i];
        a[i] = temp;
        HeapAdjust(a, 0, i);
    }
}

int main() {
    int a[] = {49, 38, 65, 97, 76, 13, 27, 49};
    int n = sizeof(a) / sizeof(a[0]);
    HeapSort(a, n);
    for (int i = 0; i < n; i++)
        printf("%d ", a[i]);
    return 0;
}