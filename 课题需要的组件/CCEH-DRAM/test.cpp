#include <cstdio>
#include <ctime>
#include <cstdlib>
#include <unistd.h>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <thread>
#include <vector>
#include <bitset>
#include "cceh.h"
using namespace std;


#define POOL_SIZE (10737418240) // 10GB

void clear_cache() {
    int* dummy = new int[1024*1024*256];
    for (int i=0; i<1024*1024*256; i++) {
	dummy[i] = i;
    }

    for (int i=100;i<1024*1024*256;i++) {
	dummy[i] = dummy[i-rand()%100] + dummy[i+rand()%100];
    }

    delete[] dummy;
}
// 这段代码利用创建并以随机访问方式修改dummy数组的元素，以此破坏程序缓存的局部性



int main (int argc, char* argv[])
{
    if(argc < 3){
	cerr << "Usage: " << argv[0] << "path numData" << endl;
	exit(1);
    }
    const size_t initialSize = 128;   //初始的表目录数量大小
    char path[32];
    strcpy(path, argv[1]);          //path这里是持久内存设备文件的路径，对应到实验室的就是 /mnt/pmem0等
    int numData = atoi(argv[2]);   //将读取到的执行数据大小改为int类型  即调用形式应该为 ./test "...(Path)" num
	
#ifdef MULTITHREAD        //这个在编译时指定了，所以直接用multi版本的可执行文件即可
    int numThreads = atoi(argv[3]);   //多线程下还需要知道第三个，线程数参数
#endif
    struct timespec start, end;
    uint64_t elapsed;     //这两个是用来计算延迟的
    bool exists = false;  //exists表明打开的路径上的持久化文件是否已经存在了，如果存在就是true
    class CCEH *HashTable=new class CCEH;    //声明持久化的CCEH类，实际上就是CCEH HashTable
    HashTable->initCCEH(initialSize);
	//这里注意下崩溃恢复的做法和依据
	/*
    if(access(path, 0) != 0){          //如果path路径上的文件没有，就创建
	pop = pmemobj_create(path, "CCEH", POOL_SIZE, 0666);
	if(!pop){
	    perror("pmemoj_create");
	    exit(1);
	}     //依据path创建持久化内存文件
	HashTable = POBJ_ROOT(pop, CCEH);   //利用root解析HashTable的指针
	D_RW(HashTable)->initCCEH(pop, initialSize);    //D_RW获取HashTable的指针，然后执行自带的initCCEH函数，initialsize指明初始化的目录数大小
    }
    else{    //否则就选择打开
	pop = pmemobj_open(path, "CCEH");
	if(pop == NULL){
	    perror("pmemobj_open");
	    exit(1);
	}
	HashTable = POBJ_ROOT(pop, CCEH);
	if(D_RO(HashTable)->crashed){    //打开时，要考虑是否崩溃了
	    D_RW(HashTable)->Recovery(pop);   //崩溃就恢复
	}
	exists = true;
    }*/

#ifdef MULTITHREAD
    cout << "Params: numData(" << numData << "), numThreads(" << numThreads << ")" << endl;
#else
    cout << "Params: numData(" << numData << ")" << endl;
#endif
    uint64_t* keys = new uint64_t[numData];      

    ifstream ifs;
    string dataset = "/home/byli/CCEH-DRAM/data";
	cout<<"3"<<endl;
    ifs.open(dataset);
    if (!ifs){
		cerr << "No file." << endl;
		exit(1);
    }
    else{
	for(int i=0; i<numData; i++)
	    ifs >> keys[i];
	ifs.close();
		cout << dataset << " is used." << endl;
    }
#ifndef MULTITHREAD // single-threaded的情况，注意这里是ifndef，即如果没指定-DMULTITHREAD
    if(!exists){  //如果原先的文件存在，就不执行插入操作，否则执行插入操作；但是不管怎么样，都得执行搜索操作
	{ // INSERT
	    cout << "Start Insertion" << endl;
	    clear_cache();
	    clock_gettime(CLOCK_MONOTONIC, &start);
	    for(int i=0; i<numData; i++){
		    HashTable->Insert(keys[i], reinterpret_cast<Value_t>(keys[i]));
	    }
	    clock_gettime(CLOCK_MONOTONIC, &end);

	    elapsed = (end.tv_sec - start.tv_sec)*1000000000 + (end.tv_nsec - start.tv_nsec);
	    cout << elapsed/1000 << "\tusec\t" << (uint64_t)(1000000*(numData/(elapsed/1000.0))) << "\tOps/sec\tInsertion" << endl;
	}
    }

    { // SEARCH
	cout << "Start Searching" << endl;
	clear_cache();
	int failedSearch = 0;
	clock_gettime(CLOCK_MONOTONIC, &start);
	for(int i=0; i<numData; i++){
	    auto ret = HashTable->Get(keys[i]);
	    if(ret != reinterpret_cast<Value_t>(keys[i])){
		failedSearch++;
	    }
	}
	clock_gettime(CLOCK_MONOTONIC, &end);
	elapsed = (end.tv_sec - start.tv_sec)*1000000000 + (end.tv_nsec - start.tv_nsec);
	cout << elapsed/1000 << "\tusec\t" << (uint64_t)(1000000*(numData/(elapsed/1000.0))) << "\tOps/sec\tSearch" << endl;
	cout << "Failed Search: " << failedSearch << endl;
    }

#else // multi-threaded
    vector<thread> insertingThreads;
    vector<thread> searchingThreads;
    int chunk_size = numData/numThreads;

    if(!exists){   //依旧是，如果，假如待打开的持久化文件存在，则不执行插入，否则先执行插入操作；然后最终都执行搜索操作
	{ // INSERT
	    auto insert = [&HashTable, &keys](int from, int to){
		for(int i=from; i<to; i++){
		    HashTable->Insert(keys[i], reinterpret_cast<Value_t>(keys[i]));
		}
	    };

	    cout << "Start Insertion" << endl;
	    clear_cache();
	    clock_gettime(CLOCK_MONOTONIC, &start);
	    for(int i=0; i<numThreads; i++){
		if(i != numThreads-1)
		    insertingThreads.emplace_back(thread(insert, chunk_size*i, chunk_size*(i+1)));
		else
		    insertingThreads.emplace_back(thread(insert, chunk_size*i, numData));
	    }

	    for(auto& t: insertingThreads) t.join();
	    clock_gettime(CLOCK_MONOTONIC, &end);

	    elapsed = (end.tv_sec - start.tv_sec)*1000000000 + (end.tv_nsec - start.tv_nsec);
	    cout << elapsed/1000 << "\tusec\t" << (uint64_t)(1000000*(numData/(elapsed/1000.0))) << "\tOps/sec\tInsertion" << endl;
	}
    }

    { // SEARCH
	int failedSearch = 0;
	vector<int> searchFailed(numThreads);

	auto search = [ &HashTable, &keys, &searchFailed](int from, int to, int tid){
	    int fail_cnt = 0;
	    for(int i=from; i<to; i++){
		auto ret = HashTable->Get(keys[i]);
		if(ret != reinterpret_cast<Value_t>(keys[i])){
		    fail_cnt++;
		}
	    }
	    searchFailed[tid] = fail_cnt;
	};

	cout << "Start Search" << endl;
	clear_cache();
	clock_gettime(CLOCK_MONOTONIC, &start);
	for(int i=0; i<numThreads; i++){
	    if(i != numThreads-1)
		searchingThreads.emplace_back(thread(search, chunk_size*i, chunk_size*(i+1), i));
	    else
		searchingThreads.emplace_back(thread(search, chunk_size*i, numData, i));
	}

	for(auto& t: searchingThreads) t.join();
	clock_gettime(CLOCK_MONOTONIC, &end);

	elapsed = (end.tv_sec - start.tv_sec)*1000000000 + (end.tv_nsec - start.tv_nsec);
	cout << elapsed/1000 << "\tusec\t" << (uint64_t)(1000000*(numData/(elapsed/1000.0))) << "\tOps/sec\tSearch" << endl;

	for(auto& v: searchFailed) failedSearch += v;
	cout << "Search Failed: " << failedSearch << endl;
    }
#endif

    auto util = HashTable->Utilization();    //util记录下当前的空间利用率
    cout << "Utilization: " << util << " %" << endl;   //这里是空间利用率测量

    HashTable->crashed = false;  //初始的时候，崩溃是true，一直到末尾才设定新值为false，这样，false才能是正常崩溃的标记
   // pmemobj_persist(pop, (char*)&D_RO(HashTable)->crashed, sizeof(bool));  //持久化标记
  //  pmemobj_close(pop);  //关闭线程池
    return 0;
} 









