#ifndef CIRCLE_H
#define CIRCLE_H

#define BUC 2048
#define MOD 2047
#define LINER 50
#define INITIAL 4 //初始的段数
#define DEPTH 2     //初始深度
#define N 10000000   //测试规模
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
class Extendible_Hash{
	public:
		Extendible_Hash(int initial_segment_num,int initial_depth);
		void Expand(size_t rs);
		bool insert(size_t key,size_t value);
		bool search(size_t key,size_t &value);
		int get_global_depth();
	protected:
		struct segment **directory;
		int global_depth;
}; 
#endif