#include <stdio.h>

// 折半插入排序（在有序区间用二分找插入位置）
void BinaryInsertSort(int a[], int n) {
    int i, j, low, high, mid, temp;
    for (i = 1; i < n; i++) {
        temp = a[i];
        low = 0;
        high = i - 1;
        // 二分查找插入位置
        while (low <= high) {
            mid = (low + high) / 2;
            if (a[mid] > temp)
                high = mid - 1;
            else
                low = mid + 1;
        }
        // 元素后移
        for (j = i - 1; j >= high + 1; j--)
            a[j + 1] = a[j];
        a[high + 1] = temp;
    }
}

int main() {
    int a[] = {49, 38, 65, 97, 76, 13, 27, 49};
    int n = sizeof(a) / sizeof(a[0]);
    BinaryInsertSort(a, n);
    for (int i = 0; i < n; i++)
        printf("%d ", a[i]);
    return 0;
}