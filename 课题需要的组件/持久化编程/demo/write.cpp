#include <iostream>
#include <libpmemobj.h>

const char* PMEM_FILE_PATH = "/mnt/pmem0/test";
const size_t ARRAY_SIZE = 10;


int main()
{
    PMEMobjpool* pool = pmemobj_create(PMEM_FILE_PATH, "my_pool", PMEMOBJ_MIN_POOL, 0666);

    if (pool == NULL) {
        std::cerr << "Failed to create pool: " << pmemobj_errormsg() << std::endl;
        return 1;
    }

    PMEMoid array_oid=pmemobj_root(pool,sizeof(int)*ARRAY_SIZE);
   // pmemobj_zalloc(pool, &array_oid, ARRAY_SIZE * sizeof(int), 0);

    int *array_ptr = static_cast<int*>(pmemobj_direct(array_oid));

    for (size_t i = 0; i < ARRAY_SIZE; i++) {
        array_ptr[i] = i + 1;
    }

    pmemobj_persist(pool, array_ptr, ARRAY_SIZE * sizeof(int));

   // pmemobj_free(&array_oid);
    pmemobj_close(pool);

    return 0;
}