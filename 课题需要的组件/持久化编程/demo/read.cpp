#include <libpmemobj.h>
#include <iostream>

#define LAYOUT_NAME "my_pool"
const size_t ARRAY_SIZE = 10;
using namespace std;

int main() {
    PMEMobjpool *pop = pmemobj_open("/mnt/pmem0/test", LAYOUT_NAME);

    if (pop == nullptr) {
        cerr << "Failed to open pool" << endl;
        return 1;
    }

    PMEMoid root_oid = pmemobj_root(pop, sizeof(int)*ARRAY_SIZE);
    int *r = static_cast<int *>(pmemobj_direct(root_oid));

    for (int i = 0; i < 10; i++) {
        cout << r[i] << " ";
    }
    cout << endl;

    pmemobj_close(pop);

    return 0;
}