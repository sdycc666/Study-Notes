#include <stdio.h>
#include <stdlib.h>

// 合并两个有序序列
void Merge(int a[], int low, int mid, int high, int tmp[]) {
    int i = low, j = mid + 1, k = low;
    while (i <= mid && j <= high) {
        if (a[i] <= a[j])
            tmp[k++] = a[i++];
        else
            tmp[k++] = a[j++];
    }
    while (i <= mid) tmp[k++] = a[i++];
    while (j <= high) tmp[k++] = a[j++];
    // 复制回原数组
    for (i = low; i <= high; i++)
        a[i] = tmp[i];
}

// 归并排序
void MergeSort(int a[], int low, int high, int tmp[]) {
    if (low < high) {
        int mid = (low + high) / 2;
        MergeSort(a, low, mid, tmp);
        MergeSort(a, mid + 1, high, tmp);
        Merge(a, low, mid, high, tmp);
    }
}

int main() {
    int a[] = {49, 38, 65, 97, 76, 13, 27, 49};
    int n = sizeof(a) / sizeof(a[0]);
    int *tmp = (int *)malloc(n * sizeof(int));
    MergeSort(a, 0, n - 1, tmp);
    free(tmp);
    for (int i = 0; i < n; i++)
        printf("%d ", a[i]);
    return 0;
}