#include <iostream>
#include <string>
#include <cstdlib>
#include<chrono>
#include <ctime>
#include <random>

using namespace std;
using namespace chrono;
#define N 63000
#define INITIAL 4096
#define DEPTH 12
#define BUC 1024
#define MOD 1023
string rand_str(const int len)  /*参数为字符串的长度*/
{

    /*初始化*/
    string str;                 /*声明用来保存随机字符串的str*/
    char c;                     /*声明字符c，用来保存随机生成的字符*/
    int idx;                    /*用来循环的变量*/
    /*循环向字符串中添加随机生成的字符*/
    for(idx = 0;idx < len;idx ++)
    {
        /*rand()%26是取余，余数为0~25加上'a',就是字母a~z,详见asc码表*/
        c = 'a' + rand()%26;
        str.push_back(c);       /*push_back()是string类尾插函数。这里插入随机字符c*/
    }
    return str;                 /*返回生成的随机字符串*/
}


struct KV{
    string key;
    int value;
};
struct bucket{
    struct KV kv[8];
};
struct segment{
    struct bucket b[BUC];
    int local_depth;
};
struct ex_hash{
    struct segment **directory;
    int global_depth;
};


bool insert(struct ex_hash *myhash,string key,int value){
    bool empty=false;
    std::hash<string> trans;
    size_t hash_value =trans(key);
    int loc1=hash_value>>(64-myhash->global_depth);
    int loc2=hash_value&MOD;
    int i,j,k; 
    for(i=loc2;i<(loc2+5)&&i<BUC;i++){    //线性探测
        for(j=0;j<8;j++){
           if(myhash->directory[loc1]->b[i].kv[j].key=="NULL"){
               empty=true;
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
bool search(ex_hash *myhash,string key,int &value){
    bool find=false;
    std::hash<string> trans;
    size_t hash_value =trans(key);
    int loc1=hash_value>>(64-myhash->global_depth);
    int loc2=hash_value&MOD;
    int i,j;
    for(i=loc2;i<(loc2+5)&&i<BUC;i++){
        for(j=0;j<8;j++){
            if(myhash->directory[loc1]->b[i].kv[j].key==key){
                find=true;
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
            for(k=0;k<8;k++){
                temp[i]->b[j].kv[k].key="NULL";
            }
        }
    }
    auto t4 = Clock::now();//计时开始
    myhash->global_depth=DEPTH;
    myhash->directory=temp;
    insert(myhash,"abcde",123);
    insert(myhash,"abcdefg",1234);
    
    int value,value1;
    if(search(myhash,"abcdefg",value)){
        cout<<"value of abcdefg(1234) is:"<<value<<endl;
    }
    string ss[N];
    
    auto t1 = Clock::now();//计时开始
    for(int time=0;time<N;time++){
       string rs=rand_str(14);
       ss[time]=rs;
       std::hash<string> trans;
       size_t hash_value =trans(rs);
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
                            for(k=0;k<8;k++){
                                temp[i]->b[j].kv[k].key="NULL";
                            }
                        }
                     }else if(i==loc3){
                        temp[i]=new struct segment;
                        temp[i]->local_depth=myhash->directory[loc1]->local_depth+1;
                        for(j=0;j<BUC;j++){
                            for(k=0;k<8;k++){
                                temp[i]->b[j].kv[k].key="NULL";
                            }
                        }

                     }else{
                         temp[i]=myhash->directory[i/2];
                     }  //先将旧目录倍增以及增加新段
                }
                
                for(i=0;i<BUC;i++){
                    for(j=0;j<8;j++){
                        if(myhash->directory[loc1]->b[i].kv[j].key!="NULL"){
                            inner_hash_value=trans2(myhash->directory[loc1]->b[i].kv[j].key);
                            if(inner_hash_value>>(64-myhash->global_depth)==loc2){
                                  for(t=inner_hash_value&BUC;(t<inner_hash_value&BUC+5)&&t<BUC;t++){
                                    bool inner_empty=false;
                                    for(k=0;k<8;k++){
                                        if(temp[loc2]->b[t].kv[k].key=="NULL"){
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
                                    for(k=0;k<8;k++){
                                        if(temp[loc3]->b[t].kv[k].key=="NULL"){
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
                    for(j=0;j<8;j++){
                        single_segment->b[i].kv[j].key="NULL";
                    }
                 }
                 int loc1=hash_value>>(64-myhash->directory[hash_value>>(64-myhash->global_depth)]->local_depth);
                 loc1=loc1<<(myhash->global_depth-myhash->directory[hash_value>>(64-myhash->global_depth)]->local_depth);
                 int loc2=loc1+(1<<(myhash->global_depth-myhash->directory[loc1]->local_depth));
                 myhash->directory[loc1]->local_depth++;
                 single_segment->local_depth=myhash->directory[loc1]->local_depth;
                 for(i=0;i<BUC;i++){
                    for(j=0;j<8;j++){
                        if(myhash->directory[loc1]->b[i].kv[j].key!="NULL"){
                            std::hash<string> temptrans;
                            size_t temp_hash_value =temptrans(myhash->directory[loc1]->b[i].kv[j].key);
                            if((temp_hash_value>>(64-myhash->directory[loc1]->local_depth))>=loc2){
                                  for(t=temp_hash_value&BUC;t<(temp_hash_value&BUC+5)&&t<BUC;t++){
                                    bool empty=false;
                                    for(k=0;k<8;k++){
                                        if (single_segment->b[t].kv[k].key!="NULL")
                                        {
                                            single_segment->b[t].kv[k].key=myhash->directory[loc1]->b[i].kv[j].key;
                                            single_segment->b[t].kv[k].value=myhash->directory[loc1]->b[i].kv[j].value;
                                            myhash->directory[loc1]->b[i].kv[j].key="NULL";
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
 //   size_t hash_table[N];
 /*
    for(int p=0;p<N;p++){
        hash_table[p]=trans2(ss[p]);
    }*/
    auto t5 = Clock::now();//计时开始

    for(int z=0;z<N;z++){
       hash_value2 =trans2(ss[z]);
       loc11=hash_value2>>(64-myhash->global_depth);
       loc21=hash_value2&MOD;
       for(i=loc21&MOD;i<(loc21+5)&&i<BUC;i++){
          for(j=0;j<8;j++){
             if(myhash->directory[loc11]->b[i].kv[j].key==ss[z]){
                  find=true;
                  value=myhash->directory[loc11]->b[i].kv[j].value;
             }
        }
    }
     find=false;
    }

    auto t6 = Clock::now();//计时开始
    if(R){
        cout<<"right"<<endl;
    }
    std::cout<<"53000次操作延迟:" <<std::chrono::duration_cast<std::chrono::nanoseconds>(t2 - t1).count()<< '\n';
    std::cout<<"初始化延迟:" <<std::chrono::duration_cast<std::chrono::nanoseconds>(t4 - t3).count()<< '\n';
    std::cout<<"读取延迟:" <<std::chrono::duration_cast<std::chrono::nanoseconds>(t6 - t5).count()<< '\n';

    return 0;
}