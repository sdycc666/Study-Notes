#include <stdio.h>

// 直接插入排序
void InsertSort(int a[], int n) {
    int i, j, temp;
    for (i = 1; i < n; i++) {
        temp = a[i];         // 当前待插入元素
        j = i - 1;
        // 向前找合适位置
        while (j >= 0 && a[j] > temp) {
            a[j + 1] = a[j]; // 元素后移
            j--;
        }
        a[j + 1] = temp;     // 插入到正确位置
    }
}

int main() {
    int a[] = {49, 38, 65, 97, 76, 13, 27, 49};
    int n = sizeof(a) / sizeof(a[0]);
    InsertSort(a, n);
    for (int i = 0; i < n; i++)
        printf("%d ", a[i]);
    return 0;
}