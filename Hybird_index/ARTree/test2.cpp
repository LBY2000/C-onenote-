#include <iostream>
#include "art.h"
#include <string>
using namespace std;
int main(int argc,char **argv){
      art_tree *new_art = new art_tree; 
      init_art_tree(new_art);
      unsigned char a[11]={'a','b','c','d','e','f','g','h','i','j','\0'},c[11]="dsdwea";
      int *b=new int,*t=new int;
      *b=10;
      *t=111;
      art_insert(new_art,a,11,b);
      art_insert(new_art,c,11,t);
      int *d=new int,*e=new int;
      d=(int *)art_search(new_art,a,11);
      e=(int *)art_search(new_art,c,11);
      cout<<"d="<<*d<<endl;
      cout<<"e="<<*e<<endl;
      art_tree_destroy(new_art);
    
    return 0;
}