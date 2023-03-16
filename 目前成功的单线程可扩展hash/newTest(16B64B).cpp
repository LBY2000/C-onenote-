#include <iostream>
#include <string>
#include <cstdlib>
#include<chrono>
#include <ctime>
#include <random>

using namespace std;
using namespace chrono;
#define N 203000
#define INITIAL 4096
#define DEPTH 12
#define BUC 1024
#define MOD 1023


struct KV{
    size_t key;
    size_t value;
};
struct bucket{
    struct KV kv[4];
};
struct segment{
    struct bucket b[BUC];
    int local_depth;
};
struct ex_hash{
    struct segment **directory;
    int global_depth;
};


bool insert(struct ex_hash *myhash,size_t key,size_t value){
    bool empty=false;
    std::hash<string> trans;
    size_t hash_value =trans(std::to_string(key));
    int loc1=hash_value>>(64-myhash->global_depth);
    int loc2=hash_value&MOD;
    int i,j,k; 
    for(i=loc2;i<(loc2+5)&&i<BUC;i++){    //线性探测
        for(j=0;j<4;j++){
           if(myhash->directory[loc1]->b[i].kv[j].key==0){
               empty=true;
               myhash->directory[loc1]->b[i].kv[j].key=key;
               myhash->directory[loc1]->b[i].kv[j].value=value;
               myhash->directory[loc1]->b[i].kv[j].key=key;
               myhash->directory[loc1]->b[i].kv[j].value=value;
               break;
           }
        }
        if(empty){
            break;
        }
    }
    return empty;
}
bool search(ex_hash *myhash,size_t key,size_t &value){
    bool find=false;
    std::hash<string> trans;
    size_t hash_value =trans(std::to_string(key));
    int loc1=hash_value>>(64-myhash->global_depth);
    int loc2=hash_value&MOD;
    int i,j;
    for(i=loc2;i<(loc2+5)&&i<BUC;i++){
        for(j=0;j<4;j++){
            if(myhash->directory[loc1]->b[i].kv[j].key==key){
                find=true;
                value=myhash->directory[loc1]->b[i].kv[j].value;
                value=myhash->directory[loc1]->b[i].kv[j].value;
            }
        }
    }
}




int main(int argc,char **argv){
    struct ex_hash *myhash=new struct ex_hash;
    struct segment **temp=new struct segment*[INITIAL];
    struct segment *single_segment;
    int i,j,k,t;
    typedef std::chrono::steady_clock Clock;
    auto t3 = Clock::now();//计时开始
    for(i=0;i<(INITIAL);i++){
        temp[i]=new struct segment;
        temp[i]->local_depth=DEPTH;
        for(j=0;j<BUC;j++){
            for(k=0;k<4;k++){
                temp[i]->b[j].kv[k].key=0;
            }
        }
    }
    auto t4 = Clock::now();//计时开始
    myhash->global_depth=DEPTH;
    myhash->directory=temp;
    insert(myhash,1111321,123);
    insert(myhash,1111432,1234);
    
    size_t value,value1;
    if(search(myhash,1111432,value)){
        cout<<"value of 1111432(1234) is:"<<value<<endl;
    }
    size_t ss[N];
    std::hash<string> trans22;
    size_t hash_value2222 =trans22(std::to_string(12345));
    cout<<"hash_value"<<hash_value2222<<endl;
    auto t1 = Clock::now();//计时开始
    for(size_t time=0;time<N;time++){
       size_t rs=(time*2+1);
       ss[time]=rs;
       std::hash<string> trans;
       size_t hash_value =trans(std::to_string(rs));
       while(insert(myhash,rs,time+1)==false){
            if(myhash->global_depth==myhash->directory[hash_value>>(64-myhash->global_depth)]->local_depth){
                //插入失败由于没有空位导致，这里判断，是进行目录倍增还是进行段分裂
                int newsize=1<<(myhash->global_depth+1);
                int loc1=hash_value>>(64-myhash->global_depth),loc2=loc1<<1,loc3=loc2+1;
                temp = new struct segment*[newsize];
                std::hash<string> trans2;
                size_t inner_hash_value;
                myhash->global_depth++;
                for(i=0;i<newsize;i++){
                     if(i==loc2){
                        temp[i]=new struct segment;
                        temp[i]->local_depth=myhash->directory[loc1]->local_depth+1;
                        for(j=0;j<BUC;j++){
                            for(k=0;k<4;k++){
                                temp[i]->b[j].kv[k].key=0;
                            }
                        }
                     }else if(i==loc3){
                        temp[i]=new struct segment;
                        temp[i]->local_depth=myhash->directory[loc1]->local_depth+1;
                        for(j=0;j<BUC;j++){
                            for(k=0;k<4;k++){
                                temp[i]->b[j].kv[k].key=0;
                            }
                        }

                     }else{
                         temp[i]=myhash->directory[i/2];
                     }  //先将旧目录倍增以及增加新段
                }
                
                for(i=0;i<BUC;i++){
                    for(j=0;j<4;j++){
                        if(myhash->directory[loc1]->b[i].kv[j].key!=0){
                            inner_hash_value=trans2(std::to_string(myhash->directory[loc1]->b[i].kv[j].key));
                            if(inner_hash_value>>(64-myhash->global_depth)==loc2){
                                  for(t=inner_hash_value&BUC;(t<inner_hash_value&BUC+5)&&t<BUC;t++){
                                    bool inner_empty=false;
                                    for(k=0;k<4;k++){
                                        if(temp[loc2]->b[t].kv[k].key==0){
                                            temp[loc2]->b[t].kv[k].key=myhash->directory[loc1]->b[i].kv[j].key;
                                            temp[loc2]->b[t].kv[k].value=myhash->directory[loc1]->b[i].kv[j].value;
                                            inner_empty=true;
                                            break;
                                        }
                                    }
                                    if(inner_empty){
                                        break;
                                    }
                                  }
                            }else{
                                  for(t=inner_hash_value&BUC;(t<inner_hash_value&BUC+5)&&t<BUC;t++){
                                    bool inner_empty=false;
                                    for(k=0;k<4;k++){
                                        if(temp[loc3]->b[t].kv[k].key==0){
                                            temp[loc3]->b[t].kv[k].key=myhash->directory[loc1]->b[i].kv[j].key;
                                            temp[loc3]->b[t].kv[k].value=myhash->directory[loc1]->b[i].kv[j].value;
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
                struct segment *p=myhash->directory[loc1];
                struct segment **q=myhash->directory;
                myhash->directory=temp;
                delete p;
                p=NULL;
                delete q;
                q=NULL;
                

            }else{
                //这里表示是段分裂的情况
                 single_segment=new struct segment;
                 for(i=0;i<BUC;i++){
                    for(j=0;j<4;j++){
                        single_segment->b[i].kv[j].key=0;
                    }
                 }
                 int loc1=hash_value>>(64-myhash->directory[hash_value>>(64-myhash->global_depth)]->local_depth);
                 loc1=loc1<<(myhash->global_depth-myhash->directory[hash_value>>(64-myhash->global_depth)]->local_depth);
                 int loc2=loc1+(1<<(myhash->global_depth-myhash->directory[loc1]->local_depth));
                 myhash->directory[loc1]->local_depth++;
                 single_segment->local_depth=myhash->directory[loc1]->local_depth;
                 for(i=0;i<BUC;i++){
                    for(j=0;j<4;j++){
                        if(myhash->directory[loc1]->b[i].kv[j].key!=0){
                            std::hash<string> temptrans;
                            size_t temp_hash_value =temptrans(std::to_string(myhash->directory[loc1]->b[i].kv[j].key));
                            if((temp_hash_value>>(64-myhash->directory[loc1]->local_depth))>=loc2){
                                  for(t=temp_hash_value&BUC;t<(temp_hash_value&BUC+5)&&t<BUC;t++){
                                    bool empty=false;
                                    for(k=0;k<4;k++){
                                        if (single_segment->b[t].kv[k].key!=0)
                                        {
                                            single_segment->b[t].kv[k].key=myhash->directory[loc1]->b[i].kv[j].key;
                                            single_segment->b[t].kv[k].value=myhash->directory[loc1]->b[i].kv[j].value;
                                            myhash->directory[loc1]->b[i].kv[j].key=0;
                                            empty=true;
                                            break;
                                        }
                                        
                                    }
                                    if(empty){
                                        break;
                                    }
                                  }

                            }else{

                            }
                        }
                    }
                 }
            int direct=2*loc2-loc1;
            for(i=loc2;i<direct;i++){
                myhash->directory[i]=single_segment;
            }
            }


       }


    }
    auto t2 = Clock::now();//计时结束
    cout<<"global_depth is:"<<myhash->global_depth<<endl;
     bool R=true;
    
    for(int z=0;z<N-1;z++){
        
        search(myhash,ss[z],value);
        search(myhash,ss[z+1],value1);
        if(value==value1){
            R=false;
            cout<<"wrong"<<endl;         
            break;
        }else if(value!=(z+1)&&value1!=(z+2)){
            cout<<"wrong"<<endl;
            R=false;
            break;
        }
        
    }
    bool find=false;
    std::hash<string> trans2;
    size_t hash_value2;
    int loc11;
    int loc21;
   // size_t hash_table[N];
 /*
    for(int p=0;p<N;p++){
        hash_table[p]=trans2(ss[p]);
    }*/
    auto t5 = Clock::now();//计时开始
    float m=100,n=124,x=234,y=321;
    for(int z=0;z<N;z++){
  //  auto t11 = Clock::now();
       hash_value2 =trans2(std::to_string(ss[z]));
       loc11=hash_value2>>(64-myhash->global_depth);
       loc21=hash_value2&MOD;
       for(i=loc21&MOD;i<(loc21+5)&&i<BUC;i++){
          for(j=0;j<4;j++){
             if(myhash->directory[loc11]->b[i].kv[j].key==ss[z]){
                  find=true;
                  value=myhash->directory[loc11]->b[i].kv[j].value;
             }
        }
    }
     find=false;
  //   auto t12 = Clock::now();
   //  std::chrono::duration_cast<std::chrono::nanoseconds>(t12 - t11).count();
   //  std::chrono::duration_cast<std::chrono::nanoseconds>(t12 - t11).count();
  //   std::chrono::duration_cast<std::chrono::nanoseconds>(t12 - t11).count();
 //    std::chrono::duration_cast<std::chrono::nanoseconds>(t12 - t11).count();
//     std::chrono::duration_cast<std::chrono::nanoseconds>(t12 - t11).count();
    }

    auto t6 = Clock::now();//计时开始
    if(R){
        cout<<"right"<<endl;
    }
    std::cout<<"203000次操作延迟:" <<std::chrono::duration_cast<std::chrono::nanoseconds>(t2 - t1).count()<< '\n';
    std::cout<<"初始化延迟:" <<std::chrono::duration_cast<std::chrono::nanoseconds>(t4 - t3).count()<< '\n';
    std::cout<<"读取延迟:" <<std::chrono::duration_cast<std::chrono::nanoseconds>(t6 - t5).count()<< '\n';

    return 0;
}