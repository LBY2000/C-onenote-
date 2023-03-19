#include <iostream>
#include "Extendible_Hash.h"


using namespace std;
int main(int argc,char **argv){
        Extendible_Hash myhash(INITIAL,DEPTH);
        size_t *ss=new size_t[N];
        for(size_t time=0;time<N;time++){
            size_t rs=(time*2)+1;
            ss[time]=rs;
            while(myhash.insert(rs,time+1)==false){
                myhash.Expand(rs);
            }
        }
        bool is_right=true;
        size_t value;
        int number=0;
        for(int t=0;t<N;t++){
             myhash.search(ss[t],value);
             if(value!=(t+1)){
                is_right=false;
                number++;
             }
        }
        if(is_right){
            cout<<"output right."<<endl;
        }else{
            cout<<"there is "<<number<<" of wrong KVs"<<endl;
        }

        cout<<"global_depth is:"<<myhash.get_global_depth()<<endl;
 
 
   return 0;
 }