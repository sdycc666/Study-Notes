#include <stdio.h>

// 冒泡排序
void BubbleSort(int a[], int n) {
    int i, j, temp;
    for (i = 0; i < n - 1; i++) {
        int flag = 0; // 标记是否发生交换
        for (j = 0; j < n - 1 - i; j++) {
            if (a[j] > a[j + 1]) {
                temp = a[j];
                a[j] = a[j + 1];
                a[j + 1] = temp;
                flag = 1;
            }
        }
        if (flag == 0) break; // 无交换说明已有序，提前结束
    }
}

int main() {
    int a[] = {49, 38, 65, 97, 76, 13, 27, 49};
    int n = sizeof(a) / sizeof(a[0]);
    BubbleSort(a, n);
    for (int i = 0; i < n; i++)
        printf("%d ", a[i]);
    return 0;
}