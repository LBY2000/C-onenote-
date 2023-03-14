#include <iostream>
#include <string>
using namespace std;

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
void expand(ex_hash *myhash,string key){
    std::hash<string> trans;
    size_t hash_value =trans(key);
    int i,j,k;
    int loc1=hash_value>>(64-myhash->global_depth);
    myhash->global_depth=myhash->global_depth+1;
    int newsize=1<<myhash->global_depth;
    //struct segment *temp[newsize];
    temp=new struct segment*[newsize];
    int loc2=loc1<<1,loc3=loc2+1;
    for(i=0;i<newsize;i++){
           temp[i]=myhash->directory[i/2];
        } //更新新目录的各个指向 
        struct segment *p=myhash->directory[loc1];
        temp[loc2]=new struct segment;
        temp[loc2]->local_depth=myhash->directory[loc1]->local_depth+1;
        temp[loc3]=new struct segment;
        temp[loc3]->local_depth=myhash->directory[loc1]->local_depth+1;
        for(i=0;i<1024;i++){
           for(j=0;j<8;j++){
              if(myhash->directory[loc1]->b[i].kv[j].key!="NULL"){
                    std::hash<string> temptrans;
                    size_t temp_hash_value =temptrans(myhash->directory[loc1]->b[i].kv[j].key);
                    if(temp_hash_value>>(64-myhash->global_depth)==loc2){
                        for(k=0;k<8;k++){
                           if(temp[loc2]->b[hash_value&1023].kv[k].key=="NULL"){
                                  temp[loc2]->b[hash_value&1023].kv[k].key=myhash->directory[loc1]->b[i].kv[j].key;
                                  temp[loc2]->b[hash_value&1023].kv[k].value=myhash->directory[loc1]->b[i].kv[j].value;
                                  break;
                                  }
                                        }
                    }else{
                        for(k=0;k<8;k++){
                           if(temp[loc3]->b[hash_value&1023].kv[k].key=="NULL"){
                                  temp[loc3]->b[hash_value&1023].kv[k].key=myhash->directory[loc1]->b[i].kv[j].key;
                                  temp[loc3]->b[hash_value&1023].kv[k].value=myhash->directory[loc1]->b[i].kv[j].value;
                                  break;
                                  }
                    }
              }
           }
        }
        
    }
    
    myhash->directory=temp;
    
    delete p;
    p=NULL;
    
    return;



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
    insert(myhash,"wadaasd",1111);
    insert(myhash,"wadaasd",1111);
    insert(myhash,"wadaasd",1111);
    insert(myhash,"wadaasd",1111);
    insert(myhash,"wadaasd",1111);
    insert(myhash,"wadaasd",1111);
    insert(myhash,"wadaasd",1111);
    insert(myhash,"wadaasd",1111); //8个 
    /*
    if(insert(myhash,"wadaasd",1111)==false){
        expand(myhash,"wadaasd");
    }  */
    if(insert(myhash,"wadaasd",1111)==false){
       std::hash<string> trans;
       size_t hash_value =trans("wadaasd");
       if(myhash->global_depth==myhash->directory[hash_value>>(64-myhash->global_depth)]->local_depth){
            string key="wadaasd";
            std::hash<string> trans;
            size_t hash_value =trans(key);
            int i,j,k;
            int loc1=hash_value>>(64-myhash->global_depth);
            myhash->global_depth=myhash->global_depth+1;
            int newsize=1<<myhash->global_depth;
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
                                if(newtemp[loc2]->b[hash_value&1023].kv[k].key=="NULL"){
                                        newtemp[loc2]->b[hash_value&1023].kv[k].key=myhash->directory[loc1]->b[i].kv[j].key;
                                        newtemp[loc2]->b[hash_value&1023].kv[k].value=myhash->directory[loc1]->b[i].kv[j].value;
                                        break;
                                        }
                                                }
                            }else{
                                for(k=0;k<8;k++){
                                if(newtemp[loc3]->b[hash_value&1023].kv[k].key=="NULL"){
                                        newtemp[loc3]->b[hash_value&1023].kv[k].key=myhash->directory[loc1]->b[i].kv[j].key;
                                        newtemp[loc3]->b[hash_value&1023].kv[k].value=myhash->directory[loc1]->b[i].kv[j].value;
                                        break;
                                        }
                            }
                    }
                }
                }
                
            }
            struct segment *p=myhash->directory[loc1];
            myhash->directory=newtemp;
            
            delete p;
            p=NULL;

               //目录和段深度一致，需要目录倍增
       }else{
            single_segment = new struct segment;
            int i,j,k;
            int loc1=hash_value>>(64-myhash->global_depth),loc2;
            myhash->directory[loc1]->local_depth++;
            for(i=0;i<1024;i++){
                for(j=0;j<8;j++){
                    if(myhash->directory[loc1]->b[i].kv[j].key!="NULL"){
                            std::hash<string> temptrans;
                            size_t temp_hash_value =temptrans(myhash->directory[loc1]->b[i].kv[j].key);
                            if(temp_hash_value>>(64-myhash->global_depth)==loc2){
                                for(k=0;k<8;k++){
                                if(single_segment->b[hash_value&1023].kv[k].key=="NULL"){
                                        single_segment->b[hash_value&1023].kv[k].key=myhash->directory[loc1]->b[i].kv[j].key;
                                        single_segment->b[hash_value&1023].kv[k].value=myhash->directory[loc1]->b[i].kv[j].value;
                                        break;
                                        }
                                                }
                            }else{
                                ;
                    }
                }
                }
                
            }
            
            
       }         //目录和段深度不一致，仅仅需要扩展段即可


    }

    
    int value1;
    search(myhash,"wadaasd",value1);
    cout<<"the value is:"<<value1<<endl;

    insert(myhash,"adwwa",111221);
    insert(myhash,"wad111d",2211);

    int value;
    search(myhash,"adwwa",value);
    cout<<"the value is:"<<value<<endl;
    cout<<"global:"<<myhash->global_depth<<endl;
    insert(myhash,"adwasdasdwa",22222145);
    search(myhash,"adwasdasdwa",value);
    cout<<"the value is:"<<value<<endl;
    
    return 0;
}


    /*   hash函数计算与8字节hash值转换
    std::hash<string> trans;
    size_t hash_value =trans("100asdasdasdasdasdsadasdadasdas");
    cout<<hash_value<<endl;
    
    cout<<sizeof(hash_value)<<endl;*/



















