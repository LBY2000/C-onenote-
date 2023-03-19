#include <iostream>
#include <string>
#include "Extendible_Hash.h"
//#include "Extendible_Hash.cpp"

using namespace std;




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
        for(j=0;j<4;j++){
        if(this->directory[loc1]->b[i%BUC].kv[j].key==0){
            empty=true;
            this->directory[loc1]->b[i%BUC].kv[j].key=key;
            this->directory[loc1]->b[i%BUC].kv[j].value=value;
            j=5;
            i=(BUC+LINER+10);
            break;
        }
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
        for(j=0;j<4;j++){
            if(this->directory[loc1]->b[i%BUC].kv[j].key==key){
                find=true;
                value=this->directory[loc1]->b[i%BUC].kv[j].value;
                j=5;
                i=(BUC+LINER+10);
                break;
            }
        }
    }
    return find;
}
int Extendible_Hash::get_global_depth(){
    return this->global_depth;
}


