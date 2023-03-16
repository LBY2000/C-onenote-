#include <iostream>
#include <string>
#include <cstdlib>
using namespace std;
#define N 20293
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
     int local_depth;
     struct bucket b[1024];
};

 struct ex_hash{
    int global_depth;
    struct segment **directory;
};
struct ex_hash *myhash;
struct segment **temp;
struct segment **newtemp;
struct segment *single_segment;
bool insert(struct ex_hash *myhash,string key,int value){
    std::hash<string> trans;
    size_t hash_value =trans(key);
    int i,j,k;
    bool empty;
    for(i=0;i<8;i++){
        if(myhash->directory[hash_value>>(64-myhash->global_depth)]->b[hash_value&1023].kv[i].key=="NULL"){
           myhash->directory[hash_value>>(64-myhash->global_depth)]->b[hash_value&1023].kv[i].key=key;
           myhash->directory[hash_value>>(64-myhash->global_depth)]->b[hash_value&1023].kv[i].value=value;
           empty=true;
           break;
        }else{
            empty=false;
        }
    }
    if(empty){
        return true;
    }else{
        return false;
   
}
}

bool search(struct ex_hash *myhash,string key,int &value){
    std::hash<string> trans;
    size_t hash_value =trans(key);
    int i;
    bool find=false;
    for(i=0;i<8;i++){
        if(myhash->directory[hash_value>>(64-myhash->global_depth)]->b[hash_value&1023].kv[i].key==key){
           value = myhash->directory[hash_value>>(64-myhash->global_depth)]->b[hash_value&1023].kv[i].value;
           find=true;
           break;
        }
    }
    if(find){
        return true;
    }else{
        return false;
    }
}

int main(int argc,char **argv){
    
    temp=new struct segment*[4];
    temp[0]=new struct segment;
    temp[1]=new struct segment;
    temp[2]=new struct segment;
    temp[3]=new struct segment;
    for(int i=0;i<4;i++){
        for(int j=0;j<1024;j++){
            for(int k=0;k<8;k++){
                temp[i]->b[j].kv[k].key="NULL";
            }
        }
    }
    myhash=new ex_hash;
    myhash->directory=temp;
    myhash->global_depth=2;
    myhash->directory[0]->local_depth=2;
    myhash->directory[1]->local_depth=2;
    myhash->directory[2]->local_depth=2;
    myhash->directory[3]->local_depth=2; 
    insert(myhash,"testfor",666);
    insert(myhash,"wadaasd",1111);
    insert(myhash,"adwwa",333);
    insert(myhash,"wad111d",444);
    insert(myhash,"adwasdasdwa",555);
    int z;
    string ss[N];
    for(z=0;z<N;z++){
       string rs =rand_str(14);
       ss[z]=rs;

    while(insert(myhash,rs,z+1)==false){

       std::hash<string> trans;
       size_t hash_value =trans(rs);
       if(myhash->global_depth==myhash->directory[hash_value>>(64-myhash->global_depth)]->local_depth){
            int i,j,k;
            int loc1=hash_value>>(64-myhash->global_depth);
            myhash->global_depth=myhash->global_depth+1;
            int newsize=1<<(myhash->global_depth);
            newtemp=new struct segment*[newsize];
            int loc2=loc1<<1,loc3=loc2+1;
            for(i=0;i<newsize;i++){
                if(i==loc2){
                    newtemp[i]=new struct segment;
                    newtemp[i]->local_depth=myhash->directory[loc1]->local_depth+1;
                    for(j=0;j<1024;j++){
                        for(k=0;k<8;k++){
                            newtemp[i]->b[j].kv[k].key="NULL";
                        }
                    }
                        
                    }else if(i==loc3){
                        newtemp[i]=new struct segment;
                        newtemp[i]->local_depth=myhash->directory[loc1]->local_depth+1;
                        for(j=0;j<1024;j++){
                            for(k=0;k<8;k++){
                                newtemp[i]->b[j].kv[k].key="NULL";
                            }
                        }
                    }else{
                        newtemp[i]=myhash->directory[i/2];
                    }
                    
                } //更新新目录的各个指向 
                
                for(i=0;i<1024;i++){
                for(j=0;j<8;j++){
                    if(myhash->directory[loc1]->b[i].kv[j].key!="NULL"){
                            std::hash<string> temptrans;
                            size_t temp_hash_value =temptrans(myhash->directory[loc1]->b[i].kv[j].key); 
                            if(temp_hash_value>>(64-myhash->global_depth)==loc2){
                                for(k=0;k<8;k++){
                                   if(newtemp[loc2]->b[i].kv[k].key=="NULL"){
                                        newtemp[loc2]->b[i].kv[k].key=myhash->directory[loc1]->b[i].kv[j].key;
                                        newtemp[loc2]->b[i].kv[k].value=myhash->directory[loc1]->b[i].kv[j].value;            
                                        break;
                                        }
                                                }
                            }else{
                                for(k=0;k<8;k++){
                                if(newtemp[loc3]->b[i].kv[k].key=="NULL"){
                                        newtemp[loc3]->b[i].kv[k].key=myhash->directory[loc1]->b[i].kv[j].key;
                                        newtemp[loc3]->b[i].kv[k].value=myhash->directory[loc1]->b[i].kv[j].value;  
                                        break;
                                        }
                            }
                    }
                }
                }
                
            }
            struct segment *p=myhash->directory[loc1];
            struct segment **q=myhash->directory;
            myhash->directory=newtemp;
            
            delete p;
            p=NULL;
            delete q;
            q=NULL;

               //目录和段深度一致，需要目录倍增
       }else{   //目录和段深度不一致，不需要目录倍增




                   if(z==20293){
                    std::hash<string> trans2;
       size_t hash_value2 =trans(ss[z]);
       cout<<endl<<"MSB:"<<(hash_value2>>(64-myhash->global_depth))<<endl;
       cout<<"LSB:"<<(hash_value2&1023)<<endl;
       cout<<"local_depth:"<<myhash->directory[16]->local_depth<<endl;
        int value,value1;
    for(int t=0;t<z-1;t++){
        search(myhash,ss[t],value);
        search(myhash,ss[t+1],value1);
        if(value==value1){
            cout<<"now is wrong"<<endl<<endl;         
            break;
        }
    }
    if(value!=value1){
        cout<<"now is right"<<endl<<endl;
    }

       }







            single_segment = new struct segment;
            for(int p=0;p<1024;p++)
             for(int q=0;q<8;q++){
                single_segment->b[p].kv[q].key="NULL";
            }
            int i,j,k;
            int loc1,loc2;
            loc1=(hash_value>>(64-myhash->directory[hash_value>>(64-myhash->global_depth)]->local_depth));
            loc1=loc1<<(myhash->global_depth-myhash->directory[hash_value>>(64-myhash->global_depth)]->local_depth);

            myhash->directory[loc1]->local_depth++;
            loc2=loc1+(1<<(myhash->global_depth-myhash->directory[loc1]->local_depth));

            single_segment->local_depth=myhash->directory[loc1]->local_depth;
            for(i=0;i<1024;i++){
                for(j=0;j<8;j++){
                    if(myhash->directory[loc1]->b[i].kv[j].key!="NULL"){
                            std::hash<string> temptrans;
                            size_t temp_hash_value =temptrans(myhash->directory[loc1]->b[i].kv[j].key);
                            if((temp_hash_value>>(64-myhash->directory[loc1]->local_depth))>=loc2){
                                for(k=0;k<8;k++){
                                if(single_segment->b[i].kv[k].key=="NULL"){
                                        single_segment->b[i].kv[k].key=myhash->directory[loc1]->b[i].kv[j].key;
                                        single_segment->b[i].kv[k].value=myhash->directory[loc1]->b[i].kv[j].value;
                                        myhash->directory[loc1]->b[i].kv[j].key="NULL";
                                        break;
                                        }
                                                }
                            }else{
                                ;
                    }
                }
                }
                
            }
            int direct=2*loc2-loc1;
            for(i=loc2;i<direct;i++){
                myhash->directory[i]=single_segment;
            }



                               if(z==20293){
                    std::hash<string> trans2;
       size_t hash_value2 =trans(ss[z]);
       cout<<endl<<"MSB:"<<(hash_value2>>(64-myhash->global_depth))<<endl;
       cout<<"LSB:"<<(hash_value2&1023)<<endl;
       cout<<"local_depth:"<<myhash->directory[16]->local_depth<<endl;
        int value,value1;
        /*
    int o[32];
    for(int r=0;r<32;r++){
        o[r]=0;
    }
    for(int t=0;t<z-1;t++){
        search(myhash,ss[t],value);
        search(myhash,ss[t+1],value1);
        
        if(value==value1){
         //   cout<<"now is wrong"<<endl<<endl;  
          //  cout<<"t="<<t<<endl;   
          //  cout<<"value="<<value<<endl;    
            //break;
            hash_value2 =trans(ss[t]);
         o[(hash_value2>>(64-myhash->global_depth))]=1;
        }
    }
    for(int r=0;r<32;r++){
        if(o[r]==1){
            cout<<"r="<<r<<endl;
        }
    }*/

    if(value!=value1){
        cout<<"now is right"<<endl<<endl;
    }

       }



       }         //目录和段深度不一致，仅仅需要扩展段即可


    }
}









    
    int value1;
    search(myhash,"wadaasd",value1);
    cout<<"the value(1111) is:"<<value1<<endl;


    int value;
    search(myhash,"adwwa",value);
    cout<<"the value(333) is:"<<value<<endl;
    search(myhash,"adwasdasdwa",value);
    cout<<"the value(555) is:"<<value<<endl;
    search(myhash,"testfor",value);
    cout<<"the value(666) is:"<<value<<endl;
    cout<<"global:"<<myhash->global_depth<<endl;
    for(z=0;z<N-1;z++){
        search(myhash,ss[z],value);
        search(myhash,ss[z+1],value1);
        if(value==value1){
            cout<<"wrong"<<endl;         
            break;
        }
    }
    if(value!=value1){
        cout<<"right"<<endl;
    }
    
    return 0;
}


    /*   hash函数计算与8字节hash值转换
    std::hash<string> trans;
    size_t hash_value =trans("100asdasdasdasdasdsadasdadasdas");
    cout<<hash_value<<endl;
    
    cout<<sizeof(hash_value)<<endl;*/



















