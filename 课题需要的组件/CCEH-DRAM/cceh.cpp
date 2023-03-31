#include <iostream>
#include <thread>
#include <bitset>
#include <cassert>
#include <unordered_map>
#include <stdio.h>
#include <vector>
#include "cceh.h"
#include "hash.h"
#include "util.h"

#define f_seed 0xc70697UL
#define s_seed 0xc70697UL
//#define f_seed 0xc70f6907UL
//#define s_seed 0xc70f6907UL

using namespace std;

void Segment::execute_path(vector<pair<size_t, size_t>>& path, Key_t& key, Value_t value){
    for(int i=path.size()-1; i>0; --i){
	bucket[path[i].first] = bucket[path[i-1].first];
//	pmemobj_persist(pop, (char*)&bucket[path[i].first], sizeof(Pair));
    }
    bucket[path[0].first].value = value;
 //   mfence();
    bucket[path[0].first].key = key;
//    pmemobj_persist(pop, (char*)&bucket[path[0].first], sizeof(Pair));
}

void Segment::execute_path(vector<pair<size_t, size_t>>& path, Pair _bucket){
    int i = 0;
    int j = (i+1) % 2;

    Pair temp[2];
    temp[0] = _bucket;
    for(auto p: path){
	temp[j] = bucket[p.first];
	bucket[p.first] = temp[i];
	i = (i+1) % 2;
	j = (i+1) % 2;
    }
}
	
vector<pair<size_t, size_t>> Segment::find_path(size_t target, size_t pattern){
    vector<pair<size_t, size_t>> path;
    path.reserve(kCuckooThreshold);
    path.emplace_back(target, bucket[target].key);

    auto cur = target;
    int i = 0;

    do{
	Key_t* key = &bucket[cur].key;
	auto f_hash = hash_funcs[0](key, sizeof(Key_t), f_seed);
	auto s_hash = hash_funcs[2](key, sizeof(Key_t), s_seed);

	if((f_hash >> (8*sizeof(f_hash) - local_depth)) != pattern || *key == INVALID){
	    break;
	}

	for(int j=0; j<kNumPairPerCacheLine*kNumCacheLine; ++j){
	    auto f_idx = (((f_hash & kMask) * kNumPairPerCacheLine) + j) % kNumSlot;
	    auto s_idx = (((s_hash & kMask) * kNumPairPerCacheLine) + j) % kNumSlot;

	    if(f_idx == cur){
		path.emplace_back(s_idx, bucket[s_idx].key);
		cur = s_idx;
		break;
	    }
	    else if(s_idx == cur){
		path.emplace_back(f_idx, bucket[f_idx].key);
		cur = f_idx;
		break;
	    }
	}
	++i;
    }while(i < kCuckooThreshold);

    if(i == kCuckooThreshold){
	path.resize(0);
    }

    return move(path);
}


bool Segment::Insert4split(Key_t& key, Value_t value, size_t loc){
    for(int i=0; i<kNumPairPerCacheLine*kNumCacheLine; ++i){
	auto slot = (loc+i) % kNumSlot;
	if(bucket[slot].key == INVALID){
	    bucket[slot].key = key;
	    bucket[slot].value = value;
	    return 1;
	}
    }
    return 0;
}

struct Segment** Segment::Split(){
	struct Segment **split = new struct Segment*[2];
	split[0]=new struct Segment;
	split[1]=new struct Segment;
	split[0]->initSegment(local_depth+1);
    split[1]->initSegment(local_depth+1);
    auto pattern = ((size_t)1 << (sizeof(Key_t)*8 - local_depth - 1));
    for(int i=0; i<kNumSlot; ++i){
	auto f_hash = h(&bucket[i].key, sizeof(Key_t));
	if(f_hash & pattern){
	    split[1]->Insert4split(bucket[i].key, bucket[i].value, (f_hash & kMask)*kNumPairPerCacheLine);
	}
	else{
	    split[0]->Insert4split(bucket[i].key, bucket[i].value, (f_hash & kMask)*kNumPairPerCacheLine);
	}
    }
    return split;
}


void CCEH::initCCEH(){   //以默认capacity初始化
    crashed = true;  //先把崩溃标记弄上去
	this->dir = new struct Directory;
	this->dir->initDirectory();
	this->dir->segment = new struct Segment*[this->dir->capacity];
    for(int i=0; i<this->dir->capacity; ++i){	
		this->dir->segment[i]=new struct Segment;
		this->dir->segment[i]->initSegment();
    }
}

void CCEH::initCCEH(size_t initCap){   //以指定大小的capacity初始化
    crashed = true;
	this->dir=new struct Directory;
	this->dir->initDirectory(static_cast<size_t>(log2(initCap)));
	this->dir->segment=new struct Segment*[this->dir->capacity];
    
    for(int i=0; i<this->dir->capacity; ++i){
		this->dir->segment[i]=new struct Segment;
		this->dir->segment[i]->initSegment(static_cast<size_t>(log2(initCap)));
    }
}
 
void CCEH::Insert( Key_t& key, Value_t value){

    auto f_hash = hash_funcs[0](&key, sizeof(Key_t), f_seed);
    auto f_idx = (f_hash & kMask) * kNumPairPerCacheLine;

RETRY:
    auto x = (f_hash >> (8*sizeof(f_hash) - this->dir->depth));
    auto target = this->dir->segment[x];
    if(!target){
	std::this_thread::yield();
	goto RETRY;
    }
    
    /* acquire segment exclusive lock */
    if(!target->lock()){
	std::this_thread::yield();
	goto RETRY;
    }

    auto target_check = (f_hash >> (8*sizeof(f_hash) - this->dir->depth));
    if(target != this->dir->segment[target_check]){
	target->unlock();
	std::this_thread::yield();
	goto RETRY;
    }

    auto pattern = (f_hash >> (8*sizeof(f_hash) - target->local_depth));
    for(unsigned i=0; i<kNumPairPerCacheLine * kNumCacheLine; ++i){
	auto loc = (f_idx + i) % Segment::kNumSlot;
	auto _key = target->bucket[loc].key;
	/* validity check for entry keys */
	if((((hash_funcs[0](&target->bucket[loc].key, sizeof(Key_t), f_seed) >> (8*sizeof(f_hash)-target->local_depth)) != pattern) || (target->bucket[loc].key == INVALID)) && (target->bucket[loc].key != SENTINEL)){
	    if(CAS(&target->bucket[loc].key, &_key, SENTINEL)){
		target->bucket[loc].value = value;
	//	mfence();
		target->bucket[loc].key = key;
    //	pmemobj_persist(pop, (char*)&D_RO(target)->bucket[loc], sizeof(Pair));
		/* release segment exclusive lock */
		target->unlock();
		return;
	    }
	}
    }

    auto s_hash = hash_funcs[2](&key, sizeof(Key_t), s_seed);
    auto s_idx = (s_hash & kMask) * kNumPairPerCacheLine;

    for(unsigned i=0; i<kNumPairPerCacheLine * kNumCacheLine; ++i){
	auto loc = (s_idx + i) % Segment::kNumSlot;
	auto _key = target->bucket[loc].key;
	if((((hash_funcs[0](&target->bucket[loc].key, sizeof(Key_t), f_seed) >> (8*sizeof(s_hash)-target->local_depth)) != pattern) || (target->bucket[loc].key == INVALID)) && (target->bucket[loc].key != SENTINEL)){
	    if(CAS(&target->bucket[loc].key, &_key, SENTINEL)){
		    target->bucket[loc].value = value;
		//mfence();
		    target->bucket[loc].key = key;
	//	pmemobj_persist(pop, (char*)&D_RO(target)->bucket[loc], sizeof(Pair));
		    target->unlock();
		return;
	    }
	}
    }

    auto target_local_depth = target->local_depth;
    // COLLISION !!
    /* need to split segment but release the exclusive lock first to avoid deadlock */
    target->unlock();

    if(!target->suspend()){
	std::this_thread::yield();
	goto RETRY;
    }

    /* need to check whether the target segment has been split */
    if(target_local_depth != this->dir->segment[x]->local_depth){
	    target->sema = 0;
	    std::this_thread::yield();
	    goto RETRY;
    }



    struct Segment** s = target->Split();
DIR_RETRY:
    /* need to double the directory */
    if(target->local_depth == dir->depth){
	if(!dir->suspend()){
	    std::this_thread::yield();
	    goto DIR_RETRY;
	}

	x = (f_hash >> (8*sizeof(f_hash) - dir->depth));
	auto dir_old = dir;
	struct Segment **d = this->dir->segment;
	struct Directory* _dir;
	_dir = new struct Directory;
    _dir->segment = new struct Segment*[this->dir->capacity*2];
	_dir->initDirectory(this->dir->depth+1);

	for(int i=0; i<this->dir->capacity; ++i){
	    if(i == x){
		   _dir->segment[2*i] = s[0];
		   _dir->segment[2*i+1] = s[1];
	    }
	    else{
		   _dir->segment[2*i] = d[i];
		   _dir->segment[2*i+1] = d[i];
	    }
	}

	dir = _dir;

    }
    else{ // normal split
	while(!this->dir->lock()){
	    asm("nop");
	}
	x = (f_hash >> (8*sizeof(f_hash) - this->dir->depth));
	if(this->dir->depth == target->local_depth + 1){
	    if(x%2 == 0){
		   this->dir->segment[x+1] = s[1];
		   this->dir->segment[x] = s[0];
	    }
	    else{
		   this->dir->segment[x] = s[1];
		   this->dir->segment[x-1] = s[0];
	    }
	    this->dir->unlock();
	}
	else{
	    int stride = pow(2, this->dir->depth - target_local_depth);
	    auto loc = x - (x%stride);
	    for(int i=0; i<stride/2; ++i){
		   this->dir->segment[loc+stride/2+i] = s[1];
	    }
	    for(int i=0; i<stride/2; ++i){
		   this->dir->segment[loc+i] = s[0];
	    }

	       this->dir->unlock();
	}
    }
    std::this_thread::yield();
    goto RETRY;
}

bool CCEH::Delete(Key_t& key){    //还没实现????
    return false;
}

Value_t CCEH::Get(Key_t& key){
    auto f_hash = hash_funcs[0](&key, sizeof(Key_t), f_seed);
    auto f_idx = (f_hash & kMask) * kNumPairPerCacheLine;

RETRY:
    while(this->dir->sema < 0){
	asm("nop");
    }

    auto x = (f_hash >> (8*sizeof(f_hash) - this->dir->depth));
    auto target = this->dir->segment[x];

    if(!target){
	std::this_thread::yield();
	goto RETRY;
    }

    auto target_check = (f_hash >> (8*sizeof(f_hash) - dir->depth));
    if(target != this->dir->segment[target_check]){
	    target->unlock();
	    std::this_thread::yield();
	goto RETRY;
    }
    
    for(int i=0; i<kNumPairPerCacheLine*kNumCacheLine; ++i){
	auto loc = (f_idx+i) % Segment::kNumSlot;
	if(target->bucket[loc].key == key){
	    Value_t v = target->bucket[loc].value;
	    return v;
	}
    }

    auto s_hash = hash_funcs[2](&key, sizeof(Key_t), s_seed);
    auto s_idx = (s_hash & kMask) * kNumPairPerCacheLine;
    for(int i=0; i<kNumPairPerCacheLine*kNumCacheLine; ++i){
	auto loc = (s_idx+i) % Segment::kNumSlot;
	if(target->bucket[loc].key == key){
	    Value_t v = target->bucket[loc].value;
	    return v;
	}
    }
    return NONE;
}

void CCEH::Recovery(){
    size_t i = 0;
    while(i < this->dir->capacity){
	size_t depth_cur = this->dir->segment[i]->local_depth;
	size_t stride = pow(2, this->dir->depth - depth_cur);
	size_t buddy = i + stride;
	if(buddy == dir->capacity) break;
	for(int j=buddy-1; i<j; j--){
	    if(this->dir->segment[j]->local_depth != depth_cur){
		   this->dir->segment[j] = this->dir->segment[i];
	    }
	}
	i += stride;
    }
}


double CCEH::Utilization(void){
    size_t sum = 0;
    size_t cnt = 0;
    for(int i=0; i<this->dir->capacity; ++cnt){
	auto target = this->dir->segment[i];
	int stride = pow(2, this->dir->depth - target->local_depth);
	auto pattern = (i >> (this->dir->depth - target->local_depth));
	for(unsigned j=0; j<Segment::kNumSlot; ++j){
	    auto f_hash = h(&target->bucket[j].key, sizeof(Key_t));
	    if(((f_hash >> (8*sizeof(f_hash)-target->local_depth)) == pattern) && (target->bucket[j].key != INVALID)){
		sum++;
	    }
	}
	i += stride;
    }
    return ((double)sum) / ((double)cnt * Segment::kNumSlot)*100.0;
}

size_t CCEH::Capacity(void){
    size_t cnt = 0;
    for(int i=0; i<this->dir->capacity; cnt++){
	auto target = this->dir->segment[i];
	int stride = pow(2, this->dir->depth - target->local_depth);
	i += stride;
    }

    return cnt * Segment::kNumSlot;
}

// for debugging
Value_t CCEH::FindAnyway(Key_t& key){
    for(size_t i=0; i<this->dir->capacity; ++i){
	for(size_t j=0; j<Segment::kNumSlot; ++j){
	    if(this->dir->segment[i]->bucket[j].key == key){
		cout << "segment(" << i << ")" << endl;
		cout << "global_depth(" << this->dir->depth << "), local_depth(" << this->dir->segment[i]->local_depth << ")" << endl;
		cout << "pattern: " << bitset<sizeof(int64_t)>(i >> (dir->depth - dir->segment[i]->local_depth)) << endl;
		cout << "Key MSB: " << bitset<sizeof(int64_t)>(h(&key, sizeof(key)) >> (8*sizeof(key) - this->dir->segment[i]->local_depth)) << endl;
		return this->dir->segment[i]->bucket[j].value;
	    }
	}
    }
    return NONE;
}
