#include <thread>
#include <iostream>
#include <string>
#include <cstdlib>
#include<chrono>
#include <ctime>
#include <cmath>
using namespace std;
#define BUC 2048
#define MOD 2047
#define LINER 50
#define INITIAL 32768 //初始的段数
#define DEPTH 15     //初始深度
#define N 10000000   //测试规模
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
    
  //  std::chrono::duration_cast<std::chrono::nanoseconds>(t2 - t1).count;
    
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
    
 //   std::chrono::duration_cast<std::chrono::nanoseconds>(t2 - t1).count;
    
    return find;
}
int Extendible_Hash::get_global_depth(){
    return this->global_depth;
}


void Extendible_Hash::multi_thread_insert(size_t key[N],size_t value[N],int left,int right){
           int i;
      //     typedef std::chrono::steady_clock Clock;
           for(i=left;i<=right;i++){
      //        auto t1 = Clock::now();//计时开始
              this->insert(key[i],value[i]);
       //       auto t2 = Clock::now();//计时结束
      //        auto delay = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();
           }
           return ;
}
void Extendible_Hash::multi_thread_search(size_t key[N],size_t value[N],int left,int right){
           int i;
       //    typedef std::chrono::steady_clock Clock;
           for(i=left;i<=right;i++){
         //     auto t1 = Clock::now();//计时开始
              this->search(key[i],value[i]);
         //     auto t2 = Clock::now();//计时开始
         //     auto delay = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();
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
    auto t1 = Clock::now();//计时开始
    thread th1(&Extendible_Hash::multi_thread_insert,&myhash,key_array,value_array,0,(N/M*1-1));
    thread th2(&Extendible_Hash::multi_thread_insert,&myhash,key_array,value_array,(N/M*1),(N/M*2-1));
    thread th3(&Extendible_Hash::multi_thread_insert,&myhash,key_array,value_array,(N/M*2),(N/M*3-1));
    thread th4(&Extendible_Hash::multi_thread_insert,&myhash,key_array,value_array,(N/M*3),(N/M*4-1));
    thread th5(&Extendible_Hash::multi_thread_insert,&myhash,key_array,value_array,(N/M*4),(N/M*5-1));
    thread th6(&Extendible_Hash::multi_thread_insert,&myhash,key_array,value_array,(N/M*5),(N/M*6-1));
    thread th7(&Extendible_Hash::multi_thread_insert,&myhash,key_array,value_array,(N/M*6),(N/M*7-1));
    thread th8(&Extendible_Hash::multi_thread_insert,&myhash,key_array,value_array,(N/M*7),(N/M*8-1));
    thread th9(&Extendible_Hash::multi_thread_insert,&myhash,key_array,value_array,(N/M*8),(N/M*9-1));
    thread th10(&Extendible_Hash::multi_thread_insert,&myhash,key_array,value_array,(N/M*9),(N/M*10-1));
    thread th11(&Extendible_Hash::multi_thread_insert,&myhash,key_array,value_array,(N/M*10),(N/M*11-1));
    thread th12(&Extendible_Hash::multi_thread_insert,&myhash,key_array,value_array,(N/M*11),(N/M*12-1));
    thread th13(&Extendible_Hash::multi_thread_insert,&myhash,key_array,value_array,(N/M*12),(N/M*13-1));
    thread th14(&Extendible_Hash::multi_thread_insert,&myhash,key_array,value_array,(N/M*13),(N/M*14-1));
    thread th15(&Extendible_Hash::multi_thread_insert,&myhash,key_array,value_array,(N/M*14),(N/M*15-1));
    thread th16(&Extendible_Hash::multi_thread_insert,&myhash,key_array,value_array,(N/M*15),(N/M*16-1));
    thread th17(&Extendible_Hash::multi_thread_insert,&myhash,key_array,value_array,(N/M*16),(N/M*17-1));
    thread th18(&Extendible_Hash::multi_thread_insert,&myhash,key_array,value_array,(N/M*17),(N/M*18-1));
    thread th19(&Extendible_Hash::multi_thread_insert,&myhash,key_array,value_array,(N/M*18),(N/M*19-1));
    thread th20(&Extendible_Hash::multi_thread_insert,&myhash,key_array,value_array,(N/M*19),(N/M*20-1));
    thread th21(&Extendible_Hash::multi_thread_insert,&myhash,key_array,value_array,(N/M*20),(N/M*21-1));
    thread th22(&Extendible_Hash::multi_thread_insert,&myhash,key_array,value_array,(N/M*21),(N/M*22-1));
    thread th23(&Extendible_Hash::multi_thread_insert,&myhash,key_array,value_array,(N/M*22),(N/M*23-1));
    thread th24(&Extendible_Hash::multi_thread_insert,&myhash,key_array,value_array,(N/M*23),(N/M*24-1));
    thread th25(&Extendible_Hash::multi_thread_insert,&myhash,key_array,value_array,(N/M*24),(N/M*25-1));
    thread th26(&Extendible_Hash::multi_thread_insert,&myhash,key_array,value_array,(N/M*25),(N/M*26-1));
    thread th27(&Extendible_Hash::multi_thread_insert,&myhash,key_array,value_array,(N/M*26),(N/M*27-1));
    thread th28(&Extendible_Hash::multi_thread_insert,&myhash,key_array,value_array,(N/M*27),(N/M*28-1));
    thread th29(&Extendible_Hash::multi_thread_insert,&myhash,key_array,value_array,(N/M*28),(N/M*29-1));
    thread th30(&Extendible_Hash::multi_thread_insert,&myhash,key_array,value_array,(N/M*29),(N/M*30-1));
    thread th31(&Extendible_Hash::multi_thread_insert,&myhash,key_array,value_array,(N/M*30),(N/M*31-1));
    thread th32(&Extendible_Hash::multi_thread_insert,&myhash,key_array,value_array,(N/M*31),(N/M*32-1));
    thread th33(&Extendible_Hash::multi_thread_insert,&myhash,key_array,value_array,(N/M*32),(N/M*33-1));
    thread th34(&Extendible_Hash::multi_thread_insert,&myhash,key_array,value_array,(N/M*33),(N/M*34-1));
    thread th35(&Extendible_Hash::multi_thread_insert,&myhash,key_array,value_array,(N/M*34),(N/M*35-1));
    thread th36(&Extendible_Hash::multi_thread_insert,&myhash,key_array,value_array,(N/M*35),(N/M*36-1));
    thread th37(&Extendible_Hash::multi_thread_insert,&myhash,key_array,value_array,(N/M*36),(N/M*37-1));
    thread th38(&Extendible_Hash::multi_thread_insert,&myhash,key_array,value_array,(N/M*37),(N/M*38-1));
    thread th39(&Extendible_Hash::multi_thread_insert,&myhash,key_array,value_array,(N/M*38),(N/M*39-1));
    thread th40(&Extendible_Hash::multi_thread_insert,&myhash,key_array,value_array,(N/M*39),(N/M*40-1));
    thread th41(&Extendible_Hash::multi_thread_insert,&myhash,key_array,value_array,(N/M*40),(N/M*41-1));
    thread th42(&Extendible_Hash::multi_thread_insert,&myhash,key_array,value_array,(N/M*41),(N/M*42-1));
    thread th43(&Extendible_Hash::multi_thread_insert,&myhash,key_array,value_array,(N/M*42),(N/M*43-1));
    thread th44(&Extendible_Hash::multi_thread_insert,&myhash,key_array,value_array,(N/M*43),(N/M*44-1));
    thread th45(&Extendible_Hash::multi_thread_insert,&myhash,key_array,value_array,(N/M*44),(N/M*45-1));
    thread th46(&Extendible_Hash::multi_thread_insert,&myhash,key_array,value_array,(N/M*45),(N/M*46-1));
    thread th47(&Extendible_Hash::multi_thread_insert,&myhash,key_array,value_array,(N/M*46),(N/M*47-1));
    thread th48(&Extendible_Hash::multi_thread_insert,&myhash,key_array,value_array,(N/M*47),(N/M*48-1));
    thread th49(&Extendible_Hash::multi_thread_insert,&myhash,key_array,value_array,(N/M*48),(N/M*49-1));
    thread th50(&Extendible_Hash::multi_thread_insert,&myhash,key_array,value_array,(N/M*49),(N/M*50-1));
   // thread th20(&Extendible_Hash::multi_thread_insert,&myhash,key_array,value_array,2000000,2499999)

    th1.join();
    th2.join();
    th3.join();
    th4.join();
    th5.join();
    th6.join();
    th7.join();
    th8.join();
    th9.join();
    th10.join();
    th11.join();
    th12.join();
    th13.join();
    th14.join();
    th15.join();
    th16.join();
    th17.join();
    th18.join();
    th19.join();
    th20.join();
    th21.join();
    th22.join();
    th23.join();
    th24.join();
    th25.join();
    th26.join();
    th27.join();
    th28.join();
    th29.join();
    th30.join();
    th31.join();
    th32.join();
    th33.join();
    th34.join();
    th35.join();
    th36.join();
    th37.join();
    th38.join();
    th39.join();
    th40.join();
    th41.join();
    th42.join();
    th43.join();
    th44.join();
    th45.join();
    th46.join();
    th47.join();
    th48.join();
    th49.join();
    th50.join();
    auto t2 = Clock::now();//计时开始
    std::cout<<M<<"个线程的"<<N<<"次写操作延迟:" <<float(std::chrono::duration_cast<std::chrono::nanoseconds>(t2 - t1).count())/1000000<<"ms"<<'\n';

    auto t3 = Clock::now();//计时开始
    thread th_1(&Extendible_Hash::multi_thread_search,&myhash,key_array,identity_value_array,0,(N/M*1-1));
    thread th_2(&Extendible_Hash::multi_thread_search,&myhash,key_array,identity_value_array,(N/M*1),(N/M*2-1));
    thread th_3(&Extendible_Hash::multi_thread_search,&myhash,key_array,identity_value_array,(N/M*2),(N/M*3-1));
    thread th_4(&Extendible_Hash::multi_thread_search,&myhash,key_array,identity_value_array,(N/M*3),(N/M*4-1));
    thread th_5(&Extendible_Hash::multi_thread_search,&myhash,key_array,identity_value_array,(N/M*4),(N/M*5-1));
    thread th_6(&Extendible_Hash::multi_thread_search,&myhash,key_array,identity_value_array,(N/M*5),(N/M*6-1));
    thread th_7(&Extendible_Hash::multi_thread_search,&myhash,key_array,identity_value_array,(N/M*6),(N/M*7-1));
    thread th_8(&Extendible_Hash::multi_thread_search,&myhash,key_array,identity_value_array,(N/M*7),(N/M*8-1));
    thread th_9(&Extendible_Hash::multi_thread_search,&myhash,key_array,identity_value_array,(N/M*8),(N/M*9-1));
    thread th_10(&Extendible_Hash::multi_thread_search,&myhash,key_array,identity_value_array,(N/M*9),(N/M*10-1));
    thread th_11(&Extendible_Hash::multi_thread_search,&myhash,key_array,identity_value_array,(N/M*10),(N/M*11-1));
    thread th_12(&Extendible_Hash::multi_thread_search,&myhash,key_array,identity_value_array,(N/M*11),(N/M*12-1));
    thread th_13(&Extendible_Hash::multi_thread_search,&myhash,key_array,identity_value_array,(N/M*12),(N/M*13-1));
    thread th_14(&Extendible_Hash::multi_thread_search,&myhash,key_array,identity_value_array,(N/M*13),(N/M*14-1));
    thread th_15(&Extendible_Hash::multi_thread_search,&myhash,key_array,identity_value_array,(N/M*14),(N/M*15-1));
    thread th_16(&Extendible_Hash::multi_thread_search,&myhash,key_array,identity_value_array,(N/M*15),(N/M*16-1));
    thread th_17(&Extendible_Hash::multi_thread_search,&myhash,key_array,identity_value_array,(N/M*16),(N/M*17-1));
    thread th_18(&Extendible_Hash::multi_thread_search,&myhash,key_array,identity_value_array,(N/M*17),(N/M*18-1));
    thread th_19(&Extendible_Hash::multi_thread_search,&myhash,key_array,identity_value_array,(N/M*18),(N/M*19-1));
    thread th_20(&Extendible_Hash::multi_thread_search,&myhash,key_array,identity_value_array,(N/M*19),(N/M*20-1));
    thread th_21(&Extendible_Hash::multi_thread_search,&myhash,key_array,identity_value_array,(N/M*20),(N/M*21-1));
    thread th_22(&Extendible_Hash::multi_thread_search,&myhash,key_array,identity_value_array,(N/M*21),(N/M*22-1));
    thread th_23(&Extendible_Hash::multi_thread_search,&myhash,key_array,identity_value_array,(N/M*22),(N/M*23-1));
    thread th_24(&Extendible_Hash::multi_thread_search,&myhash,key_array,identity_value_array,(N/M*23),(N/M*24-1));
    thread th_25(&Extendible_Hash::multi_thread_search,&myhash,key_array,identity_value_array,(N/M*24),(N/M*25-1));
    thread th_26(&Extendible_Hash::multi_thread_search,&myhash,key_array,identity_value_array,(N/M*25),(N/M*26-1));
    thread th_27(&Extendible_Hash::multi_thread_search,&myhash,key_array,identity_value_array,(N/M*26),(N/M*27-1));
    thread th_28(&Extendible_Hash::multi_thread_search,&myhash,key_array,identity_value_array,(N/M*27),(N/M*28-1));
    thread th_29(&Extendible_Hash::multi_thread_search,&myhash,key_array,identity_value_array,(N/M*28),(N/M*29-1));
    thread th_30(&Extendible_Hash::multi_thread_search,&myhash,key_array,identity_value_array,(N/M*29),(N/M*30-1));
    thread th_31(&Extendible_Hash::multi_thread_search,&myhash,key_array,identity_value_array,(N/M*30),(N/M*31-1));
    thread th_32(&Extendible_Hash::multi_thread_search,&myhash,key_array,identity_value_array,(N/M*31),(N/M*32-1));
    thread th_33(&Extendible_Hash::multi_thread_search,&myhash,key_array,identity_value_array,(N/M*32),(N/M*33-1));
    thread th_34(&Extendible_Hash::multi_thread_search,&myhash,key_array,identity_value_array,(N/M*33),(N/M*34-1));
    thread th_35(&Extendible_Hash::multi_thread_search,&myhash,key_array,identity_value_array,(N/M*34),(N/M*35-1));
    thread th_36(&Extendible_Hash::multi_thread_search,&myhash,key_array,identity_value_array,(N/M*35),(N/M*36-1));
    thread th_37(&Extendible_Hash::multi_thread_search,&myhash,key_array,identity_value_array,(N/M*36),(N/M*37-1));
    thread th_38(&Extendible_Hash::multi_thread_search,&myhash,key_array,identity_value_array,(N/M*37),(N/M*38-1));
    thread th_39(&Extendible_Hash::multi_thread_search,&myhash,key_array,identity_value_array,(N/M*38),(N/M*39-1));
    thread th_40(&Extendible_Hash::multi_thread_search,&myhash,key_array,identity_value_array,(N/M*39),(N/M*40-1));
    thread th_41(&Extendible_Hash::multi_thread_search,&myhash,key_array,identity_value_array,(N/M*40),(N/M*41-1));
    thread th_42(&Extendible_Hash::multi_thread_search,&myhash,key_array,identity_value_array,(N/M*41),(N/M*42-1));
    thread th_43(&Extendible_Hash::multi_thread_search,&myhash,key_array,identity_value_array,(N/M*42),(N/M*43-1));
    thread th_44(&Extendible_Hash::multi_thread_search,&myhash,key_array,identity_value_array,(N/M*43),(N/M*44-1));
    thread th_45(&Extendible_Hash::multi_thread_search,&myhash,key_array,identity_value_array,(N/M*44),(N/M*45-1));
    thread th_46(&Extendible_Hash::multi_thread_search,&myhash,key_array,identity_value_array,(N/M*45),(N/M*46-1));
    thread th_47(&Extendible_Hash::multi_thread_search,&myhash,key_array,identity_value_array,(N/M*46),(N/M*47-1));
    thread th_48(&Extendible_Hash::multi_thread_search,&myhash,key_array,identity_value_array,(N/M*47),(N/M*48-1));
    thread th_49(&Extendible_Hash::multi_thread_search,&myhash,key_array,identity_value_array,(N/M*48),(N/M*49-1));
    thread th_50(&Extendible_Hash::multi_thread_search,&myhash,key_array,identity_value_array,(N/M*49),(N/M*50-1));
   // thread th20(&Extendible_Hash::multi_thread_insert,&myhash,key_array,value_array,2000000,2499999)

    th_1.join();
    th_2.join();
    th_3.join();
    th_4.join();
    th_5.join();
    th_6.join();
    th_7.join();
    th_8.join();
    th_9.join();
    th_10.join();
    th_11.join();
    th_12.join();
    th_13.join();
    th_14.join();
    th_15.join();
    th_16.join();
    th_17.join();
    th_18.join();
    th_19.join();
    th_20.join();
    th_21.join();
    th_22.join();
    th_23.join();
    th_24.join();
    th_25.join();
    th_26.join();
    th_27.join();
    th_28.join();
    th_29.join();
    th_30.join();
    th_31.join();
    th_32.join();
    th_33.join();
    th_34.join();
    th_35.join();
    th_36.join();
    th_37.join();
    th_38.join();
    th_39.join();
    th_40.join();
    th_41.join();
    th_42.join();
    th_43.join();
    th_44.join();
    th_45.join();
    th_46.join();
    th_47.join();
    th_48.join();
    th_49.join();
    th_50.join();
    auto t4 = Clock::now();//计时开始
    std::cout<<M<<"个线程的"<<N<<"次读操作延迟:" <<float(std::chrono::duration_cast<std::chrono::nanoseconds>(t4 - t3).count())/1000000<<"ms"<<'\n';

    












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
