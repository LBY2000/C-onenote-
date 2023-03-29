#include <libpmemobj.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define POOL_SIZE (1024 * 1024 * 16)
#define ARRAY_SIZE 1000

POBJ_LAYOUT_BEGIN(quick_sort);
POBJ_LAYOUT_ROOT(quick_sort, struct my_root);
POBJ_LAYOUT_END(quick_sort);

struct my_root {
    int array[ARRAY_SIZE];
};

void swap(int *a, int *b) {
    int temp = *a;
    *a = *b;
    *b = temp;
}

int partition(PMEMobjpool *pop, PMEMoid root_oid, int low, int high) {
    struct my_root *root = (struct my_root *)pmemobj_direct(root_oid);
    int pivot = root->array[high];
    int i = low - 1;

    for (int j = low; j <= high - 1; j++) {
        if (root->array[j] <= pivot) {
            i++;
            swap(&root->array[i], &root->array[j]);
        }
    }
    swap(&root->array[i + 1], &root->array[high]);
    pmemobj_persist(pop, &root->array[0], sizeof(root->array));
    return (i + 1);
}

void quickSort(PMEMobjpool *pop, PMEMoid root_oid, int low, int high) {
    if (low < high) {
        int pi = partition(pop, root_oid, low, high);

        quickSort(pop, root_oid, low, pi - 1);
        quickSort(pop, root_oid, pi + 1, high);
    }
}

int main() {
    PMEMobjpool *pop = pmemobj_create("/mnt/pmem0/pool", POBJ_LAYOUT_NAME(quick_sort), POOL_SIZE, 0666);
    if (pop == NULL) {
        perror("pmemobj_create");
        return 1;
    }

    PMEMoid root_oid = pmemobj_root(pop, sizeof(struct my_root));
    struct my_root *root = (struct my_root *)pmemobj_direct(root_oid);

    srand(time(NULL));
    for (int i = 0; i < ARRAY_SIZE; i++) {
        root->array[i] = rand() % ARRAY_SIZE;
    }

    pmemobj_persist(pop, &root->array[0], sizeof(root->array));

    quickSort(pop, root_oid, 0, ARRAY_SIZE - 1);

    for (int i = 0; i < ARRAY_SIZE; i++) {
        printf("%d ", root->array[i]);
    }
    printf("\n");

    pmemobj_close(pop);

    return 0;
}