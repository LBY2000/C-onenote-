#include <iostream>
#include <map>
#include <string>
using namespace std;
class node{
	public:
		char value;
		node *left,*right;
};
node *build_tree(char root,map<char,pair<char,char> > &m){
	node *temp=new node();
	temp->value=root;
	map<char,pair<char,char> >::iterator it;
	it=m.find(root);
	if(it->second.first=='*'){
		temp->left=NULL;
	}else{
		temp->left=build_tree(it->second.first,m);
	}
	if(it->second.second=='*'){
		temp->right=NULL;
	}else{
		temp->right=build_tree(it->second.second,m);
	}
	return temp;
}
void pre_order(node *tree){
	cout<<tree->value;
	if(tree->left!=NULL){
		pre_order(tree->left);
	}
	if(tree->right!=NULL){
		pre_order(tree->right);
	}
	
	return;
}

int main(int argc, char** argv) {
	int i,n;
	cin>>n;
	string s[n+1];
	for(i=1;i<=n;i++)
	   cin>>s[i];
	map<char,pair<char,char> > m;
	for(i=1;i<=n;i++){
		m.insert(make_pair(s[i][0],pair<char,char>(s[i][1],s[i][2])));
	}
	node *tree=build_tree(s[1][0],m);
	pre_order(tree);
	return 0;
}
