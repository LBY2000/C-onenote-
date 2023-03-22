#include <thread>
#include <iostream>
#include <string>
#include <cstdlib>
#include<chrono>
#include <ctime>
#include <cmath>
#include <thread>
#include <pthread.h>  // Linux-specific API
#include <vector>
using namespace std;
#define BUC 2048
#define MOD 2047
#define LINER 50
#define INITIAL 32768 //初始的段数
#define DEPTH 15      //初始深度
#define N 100000000     //测试规模
#define M 50 //线程数


struct KV{
    size_t key;
    size_t value;
};
struct bucket{
    bool lock=false;
    struct KV kv[4];
};
struct segment{
    struct bucket b[BUC];
    int local_depth;
    bool segment_lock;
};
class Extendible_Hash{
	public:
		Extendible_Hash(int initial_segment_num,int initial_depth);
		void Expand(size_t rs);
		bool insert(size_t key,size_t value);
		bool search(size_t key,size_t &value);
		int get_global_depth();
		struct segment **directory;
		int global_depth;
        bool global_lock;
        void multi_thread_insert(size_t key[N],size_t value[N],int left,int right);  //并发插入key[left]-key[right]的内容
        void multi_thread_search(size_t key[N],size_t value[N],int left,int right);  //并发搜索key[left]-key[right]的内容


        
}; 
Extendible_Hash::Extendible_Hash(int initial_segment_num,int initial_depth){  //初始的段数和全局与局部深度
    struct segment **temp=new struct segment*[initial_segment_num];
    for(int i=0;i<(initial_segment_num);i++){
        temp[i]=new struct segment;
        temp[i]->local_depth=initial_depth;
        for(int j=0;j<BUC;j++){
            for(int k=0;k<4;k++){
                temp[i]->b[j].kv[k].key=0;
            }
        }
}
    this->directory=temp;
    this->global_depth=initial_depth;
}
void Extendible_Hash::Expand(size_t rs){   //因为插入rs而导致失败，从而作为expand的依据
    std::hash<string> trans;
    size_t hash_value =trans(std::to_string(rs));
    struct segment *single_segment;
    if(this->global_depth==this->directory[hash_value>>(64-this->global_depth)]->local_depth){  //需要目录倍增
        int newsize=1<<(this->global_depth+1);
        int loc1=hash_value>>(64-this->global_depth),loc2=loc1<<1,loc3=loc2+1;
        struct segment **temp = new struct segment*[newsize];
        std::hash<string> trans2;
        size_t inner_hash_value;
        this->global_depth++;
        for(int i=0;i<newsize;i++){
            if(i==loc2){
                temp[i]=new struct segment;
                temp[i]->local_depth=this->directory[loc1]->local_depth+1;
                for(int j=0;j<BUC;j++){
                    for(int k=0;k<4;k++){
                        temp[i]->b[j].kv[k].key=0;
                    }
                }
            }else if(i==loc3){
                temp[i]=new struct segment;
                temp[i]->local_depth=this->directory[loc1]->local_depth+1;
                for(int j=0;j<BUC;j++){
                    for(int k=0;k<4;k++){
                        temp[i]->b[j].kv[k].key=0;
                    }
                }

            }else{
                temp[i]=this->directory[i/2];
            }  //先将旧目录倍增以及增加新段
    }      //先将新段和旧段位置对应好，然后下面再迁移
    bool inner_empty;
    for(int i=0;i<BUC;i++){
        for(int j=0;j<4;j++){
            if(this->directory[loc1]->b[i].kv[j].key!=0){

                inner_hash_value=trans2(std::to_string(this->directory[loc1]->b[i].kv[j].key));
                if(inner_hash_value>>(64-this->global_depth)==loc2){
                        for(int t=inner_hash_value&MOD;(t<(inner_hash_value&MOD)+LINER);t++){
                        inner_empty=false;
                        for(int k=0;k<4;k++){
                            if(temp[loc2]->b[t%BUC].kv[k].key==0){
                                temp[loc2]->b[t%BUC].kv[k].key=this->directory[loc1]->b[i].kv[j].key;
                                temp[loc2]->b[t%BUC].kv[k].value=this->directory[loc1]->b[i].kv[j].value;
                                inner_empty=true;
                                break;
                            }
                        }
                        if(inner_empty){
                            break;
                        }
                        }
                }else{
                        for(int t=inner_hash_value&MOD;(t<(inner_hash_value&MOD)+LINER);t++){
                        bool inner_empty=false;
                        for(int k=0;k<4;k++){
                            if(temp[loc3]->b[t%BUC].kv[k].key==0){
                                temp[loc3]->b[t%BUC].kv[k].key=this->directory[loc1]->b[i].kv[j].key;
                                temp[loc3]->b[t%BUC].kv[k].value=this->directory[loc1]->b[i].kv[j].value;
                                inner_empty=true;
                                break;
                            }
                        }
                        if(inner_empty){
                            break;
                        }
                        }                               

                }
        }
        }
    }
        struct segment *p=this->directory[loc1];
        struct segment **q=this->directory;
        this->directory=temp;
        delete p;
        p=NULL;
        delete q;
        q=NULL;
    }else{   //不需要目录倍增
    //这里表示是段分裂的情况
        single_segment=new struct segment;
        int temp_loc1,temp_loc2,temp_loc3;
        for(int i=0;i<BUC;i++){
        for(int j=0;j<4;j++){
            single_segment->b[i].kv[j].key=0;
        }
        }
        int loc1=hash_value>>(64-this->directory[hash_value>>(64-this->global_depth)]->local_depth);
        loc1=loc1<<(this->global_depth-this->directory[hash_value>>(64-this->global_depth)]->local_depth);
        this->directory[loc1]->local_depth++;
        int loc2=loc1+(1<<(this->global_depth-this->directory[loc1]->local_depth));
        std::hash<string> temptrans;
        size_t temp_hash_value;
        single_segment->local_depth=this->directory[loc1]->local_depth;
        for(int i=0;i<BUC;i++){
        for(int j=0;j<4;j++){
            if(this->directory[loc1]->b[i].kv[j].key!=0){
                temp_hash_value =temptrans(std::to_string(this->directory[loc1]->b[i].kv[j].key));
                if((temp_hash_value>>(64-this->directory[loc1]->local_depth))>=loc2){
                        for(int t=temp_hash_value&MOD;t<((temp_hash_value&MOD)+LINER);t++){
                        bool empty=false;
                        for(int k=0;k<4;k++){

                            if (single_segment->b[t%BUC].kv[k].key==0)
                            {
                                single_segment->b[t%BUC].kv[k].key=this->directory[loc1]->b[i].kv[j].key;
                                single_segment->b[t%BUC].kv[k].value=this->directory[loc1]->b[i].kv[j].value;
                                this->directory[loc1]->b[i].kv[j].key=0;
                                empty=true;
                                break;
                            }
                            
                        }
                        if(empty){
                            break;
                        }
                        }

                }

            } //有一个有效的键值对
        }
        }
int direct=2*loc2-loc1;
for(int i=loc2;i<direct;i++){
    this->directory[i]=single_segment;
}                   
    }


}
bool Extendible_Hash::insert(size_t key,size_t value){
    bool empty =false;
    std::hash<string> trans;
    size_t hash_value =trans(std::to_string(key));
    
    int loc1=hash_value>>(64-this->global_depth);
    int loc2=hash_value&MOD;
    int i,j,k; 
    
    for(i=loc2;i<((loc2)+LINER);i++){    //线性探测
        RETRY:
            if(this->directory[loc1]->b[i%BUC].lock==true){
                goto RETRY;
            }else{
                this->directory[loc1]->b[i%BUC].lock==true;
                for(j=0;j<4;j++){
                    if(this->directory[loc1]->b[i%BUC].kv[j].key==0){
                        empty=true;
                        this->directory[loc1]->b[i%BUC].kv[j].key=key;
                        this->directory[loc1]->b[i%BUC].kv[j].value=value;
                        j=5;
                        i=(BUC+LINER+10);}
                }
                this->directory[loc1]->b[i%BUC].lock==false;
            }

    }
    
    
    return empty;               
}
bool Extendible_Hash::search(size_t key,size_t &value){
    bool find = false;
    std::hash<string> trans;
    size_t hash_value =trans(std::to_string(key));
    
    int loc1=hash_value>>(64-this->global_depth);
    int loc2=hash_value&MOD;
    int i,j;
    
    for(i=loc2;i<((loc2)+LINER);i++){
        RETRY:
        if(this->directory[loc1]->b[i%BUC].lock==true){
             goto RETRY;
        }else{
            this->directory[loc1]->b[i%BUC].lock==true;
            for(j=0;j<4;j++){
                if(this->directory[loc1]->b[i%BUC].kv[j].key==key){
                     find=true;
                     value=this->directory[loc1]->b[i%BUC].kv[j].value;
                     j=5;
                     i=(BUC+LINER+10);             
                }

            }
            this->directory[loc1]->b[i%BUC].lock==false;
        }
    }
    
    
    return find;
}
int Extendible_Hash::get_global_depth(){
    return this->global_depth;
}


void Extendible_Hash::multi_thread_insert(size_t key[N],size_t value[N],int left,int right){
           int i;
           typedef std::chrono::steady_clock Clock;
           for(i=left;i<=right;i++){
              auto t1 = Clock::now();//计时开始
              this->insert(key[i],value[i]);
              auto t2 = Clock::now();//计时结束
              auto delay = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();
           }
           return ;
}
void Extendible_Hash::multi_thread_search(size_t key[N],size_t value[N],int left,int right){
           int i;
           typedef std::chrono::steady_clock Clock;
           for(i=left;i<=right;i++){
              auto t1 = Clock::now();//计时开始
              this->search(key[i],value[i]);
              auto t2 = Clock::now();//计时开始
              auto delay = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();
           }
           return ;
}

int main() {
    
    Extendible_Hash myhash(INITIAL,DEPTH);
    size_t *key_array=new size_t[N];
    size_t *value_array=new size_t[N];
    size_t *identity_value_array=new size_t[N];
    typedef std::chrono::steady_clock Clock;
    for(int i=0;i<N;i++){
        key_array[i]=i*2+1;
        value_array[i]=i+1;
    }
    cpu_set_t mask;
    CPU_ZERO(&mask);
    for (int i = 0; i < 20; ++i)
    {
        CPU_SET(i, &mask);
    }
    pthread_t current_thread = pthread_self();
    pthread_setaffinity_np(current_thread, sizeof(cpu_set_t), &mask);
    std::vector<std::thread> threads,threads_2;
    for (int i = 0; i < M; ++i)
    {
        threads.emplace_back(&Extendible_Hash::multi_thread_insert,&myhash,key_array,value_array,(N/M)*i,((N/M)*(i+1)-1));
    }
    auto t1 = Clock::now();//计时开始
    for (auto& thread : threads)
    {
        thread.join();
    }
    auto t2 = Clock::now();//计时结束
    std::cout<<M<<"个线程的"<<N<<"次写操作延迟:" <<float(std::chrono::duration_cast<std::chrono::nanoseconds>(t2 - t1).count())/1000000<<"ms"<<'\n';
    
    for (int i = 0; i < M; ++i)
    {
        threads_2.emplace_back(&Extendible_Hash::multi_thread_search,&myhash,key_array,value_array,(N/M)*i,((N/M)*(i+1)-1));
    }
    auto t3 = Clock::now();//计时开始
    for (auto& thread : threads_2)
    {
        thread.join();
    }
    auto t4 = Clock::now();//计时结束
    
    std::cout<<M<<"个线程的"<<N<<"次读操作延迟:" <<float(std::chrono::duration_cast<std::chrono::nanoseconds>(t4 - t3).count())/1000000<<"ms"<<'\n';
    
    /*
    auto t1 = Clock::now();//计时开始
    thread *th[M];
    for(int num=0;num<M;num++){
       th[num]=new thread(&Extendible_Hash::multi_thread_insert,&myhash,key_array,value_array,(N/M)*num,((N/M)*(num+1)-1));
        
    }
    
    for(int num=0;num<M;num++){
       th[num]->join();
    }
    auto t2 = Clock::now();//计时结束
    std::cout<<M<<"个线程的"<<N<<"次写操作延迟:" <<float(std::chrono::duration_cast<std::chrono::nanoseconds>(t2 - t1).count())/1000000<<"ms"<<'\n';


    auto t3 = Clock::now();//计时开始
    thread *th_search[M];
    for(int num=0;num<M;num++){
       th_search[num]=new thread(&Extendible_Hash::multi_thread_search,&myhash,key_array,identity_value_array,(N/M)*num,((N/M)*(num+1)-1));
    }
    
    for(int num=0;num<M;num++){
       th_search[num]->join();
    }
    auto t4 = Clock::now();//计时结束
    std::cout<<M<<"个线程的"<<N<<"次读操作延迟:" <<float(std::chrono::duration_cast<std::chrono::nanoseconds>(t4 - t3).count())/1000000<<"ms"<<'\n';

    */












    /*
    bool is_right=true;
    size_t value;
    int number=0;
    for(int t=0;t<N;t++){
            myhash.search(key_array[t],value);
            if(value!=(t+1)){
            is_right=false;
            number++;
            }
    }
    if(is_right){
        cout<<"output right."<<endl;
    }else{
        cout<<"there is "<<number<<" of wrong KVs"<<endl;
    }*/
	return 0;
}
