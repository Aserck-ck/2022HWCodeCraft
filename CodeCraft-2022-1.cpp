#include<iostream>
#include<vector>
#include<algorithm>
#include<cmath>
#include<iomanip>
#include<string.h>
#include<string>
#include<cstring>
#include<map>
#include<queue>
#include<set>
#include<stack>
#include<ctime>
#define me(x,y) memset(x,y,sizeof(x))
using namespace std;
typedef long long LL;
typedef long double LD;
typedef pair<LL, LL> PLL;
clock_t start_time; //程序开始运行时间，用于计时
const int maxt = 9000, maxm = 40, maxn = 140, maxp = 105; //T ,M ,N ,Pt的数据范围
LL Q = 0; //QoS约束值
LL T = 0; //时刻数
LL M = 0; //客户数量
LL N = 0; //服务器数量
LL V = 0; //base_cost
LD A = 0; //center_cost
LL percent95; //T*0.95 向上取整
LL percent5;  //T-percent95
//【数组下标全部从1开始，一下所说"小片"均指代题目中说的"流"
//发生一个小片的分配时，要同时更新D_t_c_p，D_client_moment，server.used_band，X_t_c_p
LL D_t_c_p[maxt][maxm][maxp]; //D[t][c_id][k]表示c_id在t时刻的第k号小片(流)的剩余需求量，由于小片不可分割，要么不取，要么一次取空
string PieceName_t_p[maxt][maxp]; //PieceName_t_p[t][k]表示t时刻的第k号小片(流)的名字
int PieceNumber_t[maxt]; //PieceNumber_t[t]表示t时刻有多少种小片
LL D_client_moment[maxm][maxt]; //D[c_id][t]==第c_id号客户在第t时刻的剩余需求量（各小片之和），意义与初赛一致
LL D0_t_c_p[maxt][maxm][maxp]; //复制原始数据，永远不改
LL D0_client_moment[maxm][maxt]; //同
//
LL QoS_client_server[maxm][maxn]; //Qos[i][j]==i号客户与j号服务器之间QoS
int X_t_c_p[maxt][maxm][maxp]; //X_t_c_p[t][c_id][k]记录t时刻c_id的第k小片分给了哪个服务器s_id
//
int adjacency_c_s[maxm][maxn]; //邻接矩阵，adjacency_c_s[c_id][s_id]为1表示c_id号客户和s_id号服务器有边
int pre_allocate_server_moment[maxn][maxt]; //【大于等于1】表示s_id在t时刻发生预分配
//之前认为预分配时确定归属的小片不能动，现在不是如此
//int pre_allocate_piece_t_c_p[maxt][maxm][maxp];  //为1表示 t时刻 c_id号客户 的第p_id号小片是预分配时确定归属的小片



bool cmp_bigger(LL a, LL b)
{
	return a > b;
}
//bool cmp_bigger(LD a, LD b)
//{
//	return a > b;
//}
struct Node_fare //在fare_sequence中，不但记录带宽用量，还顺便记录是哪个时刻
{
	LL used;
	int tim;
	Node_fare(LL Used, int t)
	{
		used = Used; tim = t;
	}
};
bool cmp_Node_fare(Node_fare a, Node_fare b)
{
	return a.used > b.used;
}

////几个与中心节点用量有关的全局数组
LL MS_t_s_p[maxt][maxn][maxp]; //MS_t_s_p[t][s_id][p_id]==t时刻，服务器s_id，收到的第p_id种小片的 Max Size
LL MS_t_s[maxt][maxn]; //MS_t_s[t][s_id]==求和MS_t_s_p[t][s_id][..]
LL MS_t[maxt]; //MS_t[t]==求和MS_t_s_p[t][..][..]，也就是中心节点在t时刻的用量

struct Center
{
	vector<Node_fare>fare_sequence; //实时维护中心节点的计费序列
	LL p95_band() //返回目前序列中记录的95带宽
	{
		int len = fare_sequence.size(); //如果要优化时间，可以考虑把自行维护fare_sequence 的length
		if (len <= percent5)return 0;
		return fare_sequence[percent5].used;
	}
	int p95_tim() //返回目前序列中记录95带宽的属于的时刻
	{
		int len = fare_sequence.size();
		if (len <= percent5)return 0;
		return fare_sequence[percent5].tim;
	}
	LL p96_band() //返回目前序列中记录的比p95大一位的带宽，也就是最小一条免费的用量
	{
		int len = fare_sequence.size();
		if (len <= percent5 - 1)return 0;
		return fare_sequence[percent5 - 1].used;
	}
	int p96_tim()
	{
		int len = fare_sequence.size();
		if (len <= percent5 - 1)return 0;
		return fare_sequence[percent5 - 1].tim;
	}
	LL p100_band() //返回目前序列中记录的，当前服务器带宽用量最大值，也就是最大一条免费的用量
	{
		if (fare_sequence.size() > 0)return fare_sequence[0].used;
		return 0;
	}
	int p100_tim()
	{
		if (fare_sequence.size() > 0)return fare_sequence[0].tim;
		return 0;
	}
	void keep_center_fare_sequence(int t) //与边缘节点的维护函数逻辑相同
	{
		LL used = MS_t[t];
		LL p95 = p95_band();
		if (used > p95)
		{
			vector<Node_fare>&fs = fare_sequence;
			int len = fs.size();
			for (int i = 0; i < len; i++)
			{
				if (fs[i].tim == t)
				{
					fs[i].used = used;
					sort(fs.begin(), fs.end(), cmp_Node_fare);
					return;
				}
			}
			Node_fare u(used, t);
			fs.push_back(u);
			sort(fs.begin(), fs.end(), cmp_Node_fare);
			while (fs.size() > percent5 + 1)
			{
				fs.pop_back();
			}
		}
	}
}center;

struct Server
{
	string name = "";
	bool this_server_is_used = false;  //用于区分全空服务器和已使用服务器，false表示全空，true表示已使用
	LL band_limit = 0; //带宽上限
	LL used_band[maxt]; //记录这台服务器每个时刻的已用带宽和，其值不能超过band_limit
	vector<int>neighbor; //记录所有相连的客户id
	int degree() { return neighbor.size(); }
	vector<Node_fare>fare_sequence; //实时维护的计费序列，分配完一个时刻t，就把这个时刻的用量加入计费队列，并重新排序
	LL p95_band() //返回目前序列中记录的95带宽
	{
		int len = fare_sequence.size();
		if (len <= percent5)return 0;
		return fare_sequence[percent5].used;
	}
	int p95_tim() //返回目前序列中记录95带宽的属于的时刻
	{
		int len = fare_sequence.size();
		if (len <= percent5)return 0;
		return fare_sequence[percent5].tim;
	}
	LL p96_band() //返回目前序列中记录的比p95大一位的带宽，也就是最小一条免费的用量
	{
		int len = fare_sequence.size();
		if (len <= percent5 - 1)return 0;
		return fare_sequence[percent5 - 1].used;
	}
	int p96_tim()
	{
		int len = fare_sequence.size();
		if (len <= percent5 - 1)return 0;
		return fare_sequence[percent5 - 1].tim;
	}
	LL p100_band() //返回目前序列中记录的，当前服务器带宽用量最大值，也就是最大一条免费的用量
	{
		if (this_server_is_used)return fare_sequence[0].used;
		return 0;
	}
	int p100_tim()
	{
		if (this_server_is_used)return fare_sequence[0].tim;
		return 0;
	}

	//不同的阶段，不要共用跟计费有关的函数
	LL free_space_for_server(int t) //注意，返回值是免费空间大小，需要减去已用的
	{
		if (this_server_is_used == false)return 0; //全空服务器没有免费空间，使用需要付费V
		//已用服务器，可以填V
		int p95_t = p95_tim();
		LL p95_b = p95_band();
		if (p95_t != t)return max(p95_b, V);
		//计费序列中记录的95带宽正是当前时刻，如果已经超过V，则无免费空间
		if (p95_b < V)return V;
		return p95_b; //【待修改，这里是不是要改成return p95_b
	}

}server[maxn];
void keep_fare_sequence(int s_id, int t) //用used_band[t]维护计费序列
{
	LL used = server[s_id].used_band[t];
	LL p95_band = server[s_id].p95_band();
	if (used>p95_band) //只有当used超过原来的95带宽，才需要更新计费序列
	{
		vector<Node_fare>&fs = server[s_id].fare_sequence;
		int len = fs.size();
		for (int i = 0; i < len; i++) //先找找此时刻是否已经在计费队列中
		{
			if (fs[i].tim == t) //如果已经在队列中，则更新使用量，重新排序即可
			{
				fs[i].used = used;
				//【如果想要优化速度，可以考虑在此处改成O(n)的插入
				sort(fs.begin(), fs.end(), cmp_Node_fare);
				return;
			}
		}
		//属于是新计入队列的时刻（未必是新，也可能它之前的用量在队列里排不上号
		Node_fare u(used, t);
		fs.push_back(u);
		sort(fs.begin(), fs.end(), cmp_Node_fare);
		while (fs.size() > percent5 + 1)
		{
			fs.pop_back();
		}

	}
}
LD how_much_this_server_cost_now(int s_id) //返回此服务器当前已经需要付多少钱
{
	if (server[s_id].this_server_is_used == false)return 0;
	LD p95_band = server[s_id].p95_band();
	if (p95_band <= V)return V;
	LD band_limit = server[s_id].band_limit;
	return (p95_band - V)*(p95_band - V)*1.0 / band_limit + p95_band;
}
void regenerate_fare_sequence(int s_id)
{
	server[s_id].fare_sequence.clear();
	for (int t = 1; t <= T; t++)
	{
		LL used = server[s_id].used_band[t];
		if (used)
		{
			Node_fare nod(used, t);
			server[s_id].fare_sequence.push_back(nod);
		}
	}
	sort(server[s_id].fare_sequence.begin(), server[s_id].fare_sequence.end(), cmp_Node_fare);
}
void regenerate_center_fare_sequence(bool constraint_length = false)
{
	center.fare_sequence.clear();
	for (int t = 1; t <= T; t++)
	{
		LL used = MS_t[t];
		if (used)
		{
			Node_fare nod(used, t);
			center.fare_sequence.push_back(nod);
		}
	}
	sort(center.fare_sequence.begin(), center.fare_sequence.end(), cmp_Node_fare);
	if (constraint_length)  //限制长度
	{
		while (center.fare_sequence.size() > percent5 + 1)
		{
			center.fare_sequence.pop_back();
		}
	}
}
struct Client
{
	string name = "";
	vector<int>neighbor; //记录所有相连的服务器id
	int degree() { return neighbor.size(); }
}client[maxm];

//一个小片的三个属性：所属的时刻，客户，片号
struct Node_piece
{
	int tim, c_id, p_id;
	Node_piece(int t, int c, int p)
	{
		tim = t; c_id = c; p_id = p;
	}
};

map<string, int>mp_client_to_id; //从客户名字到下标
map<string, int>mp_server_to_id; //从服务器名字到下标

void assign_a_piece(int& t, int& c_id, int& p_id, int& s_id)//将t时刻客户c_id的第p_id号小片分配给服务器s_id
{
	LL& a_piece = D_t_c_p[t][c_id][p_id];
	if (a_piece > MS_t_s_p[t][s_id][p_id]) //更新代表中心节点用量的三个数组
	{
		LL delta = a_piece - MS_t_s_p[t][s_id][p_id];
		MS_t_s_p[t][s_id][p_id] += delta;
		MS_t_s[t][s_id] += delta;
		MS_t[t] += delta;
	}
	server[s_id].used_band[t] += a_piece;
	X_t_c_p[t][c_id][p_id] = s_id;
	D_client_moment[c_id][t] -= a_piece;
	D_t_c_p[t][c_id][p_id] = 0;
	server[s_id].this_server_is_used = true;
}

//【如果需要优化速度，试着改成传引用
bool cmp_Node_piece_for_first_schedule2(Node_piece a, Node_piece b)
{
	//片大的先胜于片小的先
	return D_t_c_p[a.tim][a.c_id][a.p_id] > D_t_c_p[b.tim][b.c_id][b.p_id];
}
bool cmp_Node_piece_for_pre_allocate(Node_piece a, Node_piece b)
{
	//片大的先胜于片小的先
	return D_t_c_p[a.tim][a.c_id][a.p_id] > D_t_c_p[b.tim][b.c_id][b.p_id];
}


//预分配，服务器的考虑顺序
bool cmp_server_for_pre_allocate(int i, int j) //这太怪了【也许随机打乱甚至比这个顺序还好
{
	//return server[i].degree() > server[j].degree();
	return server[i].band_limit > server[j].band_limit;
}
//预分配当前考虑的是哪个服务器
int now_s_id_in_pre_allocate = 0;
//预分配当前考虑的是哪个时刻
int now_t_in_pre_allocate = 0;
//////第一版预分配，无演习，使用(小片大小)均值进行时刻选择，时间代价小，不够精准
struct Node_pre
{
	int tim;
	LL demand_sum = 0;  //时刻t的邻居需求之和
	LD average = 0;     //（小片大小）均值
};
bool cmp_Node_pre(Node_pre a, Node_pre b)
{
	LL band_limit = server[now_s_id_in_pre_allocate].band_limit;
	//当两个时刻的需求量都超过服务器带宽上限时，比较需求大小已经无意义
	LD k = 1; //0.95已经不行
	if (a.demand_sum >= band_limit*k&&b.demand_sum >= band_limit*k)
	{//均值大的好一些，但为什么方差不能用来衡量呢
		return a.average > b.average;
	}
	return a.demand_sum > b.demand_sum;
}
void pre_allocate_all_free() //预分配第一版本
{
	vector<int>s_id_vec;
	for (int i = 1; i <= N; i++)
	{
		s_id_vec.push_back(i);
	}
	//服务器排序，决定谁先进行预分配
	sort(s_id_vec.begin(), s_id_vec.end(), cmp_server_for_pre_allocate);

	//srand(unsigned(time(0)));
	//random_shuffle(s_id_vec.begin(), s_id_vec.end());

	for (int s_idx = 0; s_idx < N; s_idx++)  //按排好的顺序考虑服务器【待修改，也许随机打乱更好
	{
		int s_id = s_id_vec[s_idx];   //取出服务器id
		now_s_id_in_pre_allocate = s_id; //改全局变量，当前考虑到的服务器是s_id
		if (server[s_id].degree() == 0)continue; //没有出度的服务器，没有任何利用价值
		if (server[s_id].degree() <= 3)continue;//////////////////////////////////////////////
		int c_len = server[s_id].neighbor.size();
		//同一个服务器的percent5次预分配都发生于不同时刻
		vector<Node_pre>sum_per_day;
		for (int t = 1; t <= T; t++)
		{
			Node_pre nod; nod.tim = t; nod.demand_sum = 0; nod.average = 0;
			for (int c_idx = 0; c_idx < c_len; c_idx++)
			{
				int c_id = server[s_id].neighbor[c_idx];
				nod.demand_sum += D_client_moment[c_id][t];
			}
			int p_num_in_t = 0;
			int pNum = PieceNumber_t[t];
			//统计t时刻有多少个小片
			for (int c_idx = 0; c_idx < c_len; c_idx++)
			{
				int c_id = server[s_id].neighbor[c_idx];
				for (int p_id = 1; p_id <= pNum; p_id++)
				{
					LL a_piece = D_t_c_p[t][c_id][p_id];
					if (a_piece)
					{
						p_num_in_t++;
					}
				}
			}
			//求小片大小均值
			nod.average = nod.demand_sum*1.0 / p_num_in_t;
			sum_per_day.push_back(nod);
		}
		//当demand_sum已经超过band_limit时，比较demand_sum已无意义，应该比较什么？
		sort(sum_per_day.begin(), sum_per_day.end(), cmp_Node_pre);
		for (int t_idx = 0; t_idx < percent5; t_idx++) //前percent5次，要么导致满载，要么吸光所有客户，且发生在邻居需求最高时刻
		{
			//【待修改，此处可以通过最大片的大小等信息，再次决定是否要在t_idx发生预分配
			int t = sum_per_day[t_idx].tim;
			now_t_in_pre_allocate = t; //改全局变量
			pre_allocate_server_moment[s_id][t] = 1; //记录s_id在t时刻发生预分配
			vector<Node_piece>piece_vec;
			//将t时刻的所有邻居小片都放入队列中
			int pNum = PieceNumber_t[t];
			for (int c_idx = 0; c_idx < c_len; c_idx++)
			{
				int c_id = server[s_id].neighbor[c_idx];
				for (int p_id = 1; p_id <= pNum; p_id++)
				{
					if (D_t_c_p[t][c_id][p_id]) //只有非空小片才需要分配
					{
						Node_piece pie(t, c_id, p_id);
						piece_vec.push_back(pie);
					}
				}
			}
			//大片在前，优先吸收【其实是不是要考虑一下，在自身band有限的情况下，不要抢其他服务器的小片？
			sort(piece_vec.begin(), piece_vec.end(), cmp_Node_piece_for_pre_allocate);
			int neighbor_piece_num = piece_vec.size();
			for (int node_idx = 0; node_idx < neighbor_piece_num; node_idx++)
			{
				Node_piece piece = piece_vec[node_idx];
				int c_id = piece.c_id;
				int p_id = piece.p_id;
				LL capacity = server[s_id].band_limit - server[s_id].used_band[t];
				if (capacity <= 0)break; //服务器满载，但是这种情况应该很难出现（指正好放满，因为不可能超越满载
				if (capacity < D_t_c_p[t][c_id][p_id])continue; //这一片放不下
				assign_a_piece(t, c_id, p_id, s_id);
			}
			//整个时刻考虑完成，维护计费队列
			keep_fare_sequence(s_id, t);
		}
	}
}


//////第二版预分配，有演习，使用实际能装入服务器的小片的均值进行时刻选择，时间代价较大，且只比第一版提升2w分左右（从47w到45w）
struct Node_fake_t_result
{
	int tim;  //进行模拟的时刻
	LL demand_sum = 0;    //该时刻邻居总需求
	LL used_band = 0;   //实际装入的需求
	int put_piece_num = 0;    //装入的片数
	//LL max_piece_size = 0;    //装入的最大片的size
	//int threshold_exceeded_num = 0;  //装入的片中，大小超过阈值的片数
	Node_fake_t_result(int Tim, LL D_sum, LL Used_band, int Put_num)
	{
		tim = Tim;
		demand_sum = D_sum; used_band = Used_band; put_piece_num = Put_num;
	}
};
//衡量两个时刻，谁更适合发生预分配
bool cmp_Node_fake_t_result(Node_fake_t_result a, Node_fake_t_result b)
{
	//【待修改，还没试过直接比较实际装入量，且忽视了值相等的情况
	LL band_limit = server[now_s_id_in_pre_allocate].band_limit;
	if (a.demand_sum >= band_limit&&b.demand_sum >= band_limit)
	{//当两个时刻都是不能完全装入的时刻，则比较装入的均值
		return a.used_band*1.0 / a.put_piece_num > b.used_band*1.0 / b.put_piece_num;
	}
	//否则两个时刻中，至少有一个时刻可以完全装入
	return a.used_band > b.used_band;
}
void pre_allocate_all_free2() //预分配第二版本，先在每个时刻模拟演练分配，再根据每个时刻的装入效果，决定要在哪几个时刻真正进行预分配
{
	vector<int>s_id_vec;
	for (int i = 1; i <= N; i++)
	{
		s_id_vec.push_back(i);
	}
	//服务器排序，决定谁先进行预分配
	sort(s_id_vec.begin(), s_id_vec.end(), cmp_server_for_pre_allocate);

	for (int s_idx = 0; s_idx < N; s_idx++)  //按排好的顺序考虑服务器
	{
		int s_id = s_id_vec[s_idx];   //取出服务器id
		now_s_id_in_pre_allocate = s_id; //改全局变量，当前考虑到的服务器是s_id
		int c_len = server[s_id].degree();
		if (c_len == 0)continue; //没有出度的服务器，没有任何利用价值

		vector<Node_fake_t_result>result_vec; //用来存储每个时刻模拟预分配的结果，但【有信息丢失】
		//模拟预分配，在不确定此服务器应该在哪些时刻发生预分配时，先在全部时刻试一遍，在里面选出效果最好的percent5个时刻，进行真正预分配
		for (int fake_t = 1; fake_t <= T; fake_t++)
		{
			//在fake_t这个时刻中模拟一下预分配
			vector<Node_piece>fake_piece_vec;
			//把fake_t时刻中，邻居的小片都串成队列fake_piece_vec
			int pNum = PieceNumber_t[fake_t];
			for (int c_idx = 0; c_idx < c_len; c_idx++)
			{
				int c_id = server[s_id].neighbor[c_idx];
				for (int p_id = 1; p_id <= pNum; p_id++)
				{
					if (D_t_c_p[fake_t][c_id][p_id])
					{
						Node_piece pie(fake_t, c_id, p_id);
						fake_piece_vec.push_back(pie);
					}
				}
			}
			//大的片在前，与真预分配的过程一样
			sort(fake_piece_vec.begin(), fake_piece_vec.end(), cmp_Node_piece_for_pre_allocate);
			int piece_num = fake_piece_vec.size();
			//记录装入的片数，时刻总需求量，实际装入量
			int put_piece_num = 0; LL demand_sum = 0; LL fake_used_band_t = 0;
			for (int node_idx = 0; node_idx < piece_num; node_idx++)
			{
				//跟真预分配一样，从大到小填入小片
				Node_piece piece = fake_piece_vec[node_idx];
				int c_id = piece.c_id;
				int p_id = piece.p_id;
				LL piece_size = D_t_c_p[fake_t][c_id][p_id];
				demand_sum += piece_size;                //记录需求之和
				//注意，模拟分配时不能改动任何东西，除了专门为这个过程准备的fake_server的used_band[ ]
				LL capacity = server[s_id].band_limit - fake_used_band_t;

				if (capacity < piece_size)continue; //放不下

				//模拟放入此小片，同步记录一些效果衡量值
				put_piece_num++;                         //总共装了几个小片，计数
				fake_used_band_t += piece_size;    //此时刻用量累计

			}///fake_t时刻的小片队列遍历分配完成
			//保存记录的信息及本时刻，存入队列中
			////构造Node_fake_t_result(int Tim, LL D_sum, LL Used_band, int Put_num)
			Node_fake_t_result result(fake_t, demand_sum, fake_used_band_t, put_piece_num);
			result_vec.push_back(result);
		}
		//以上，所有时刻中都进行了一次模拟分配
		sort(result_vec.begin(), result_vec.end(), cmp_Node_fake_t_result);
		//排好序，前面percent5个记录的时刻，就是应该决定要发生预分配的时刻，下面进行真预分配
		for (int t_idx = 0; t_idx < percent5; t_idx++) //前percent5次，要么导致满载，要么吸光所有客户，且发生在邻居需求最高时刻
		{
			int t = result_vec[t_idx].tim;
			now_t_in_pre_allocate = t; //改全局变量
			pre_allocate_server_moment[s_id][t] = 1; //记录s_id在t时刻发生预分配
			vector<Node_piece>piece_vec;
			//将t时刻的所有邻居小片都放入队列中
			int pNum = PieceNumber_t[t];
			for (int c_idx = 0; c_idx < c_len; c_idx++)
			{
				int c_id = server[s_id].neighbor[c_idx];
				for (int p_id = 1; p_id <= pNum; p_id++)
				{
					if (D_t_c_p[t][c_id][p_id]) //只有非空小片才需要分配
					{
						Node_piece pie(t, c_id, p_id);
						piece_vec.push_back(pie);
					}
				}
			}
			//大片在前，优先吸收【其实是不是要考虑一下，在自身band有限的情况下，不要抢其他服务器的小片？
			sort(piece_vec.begin(), piece_vec.end(), cmp_Node_piece_for_pre_allocate);
			int neighbor_piece_num = piece_vec.size();
			for (int node_idx = 0; node_idx < neighbor_piece_num; node_idx++)
			{
				Node_piece piece = piece_vec[node_idx];
				int c_id = piece.c_id;
				int p_id = piece.p_id;
				LL capacity = server[s_id].band_limit - server[s_id].used_band[t];
				if (capacity <= 0)break; //服务器满载，但是这种情况应该很难出现（指正好放满，因为不可能超越满载
				if (capacity < D_t_c_p[t][c_id][p_id])continue; //这一片放不下
				assign_a_piece(t, c_id, p_id, s_id);
			}
			//整个时刻考虑完成，维护计费队列
			keep_fare_sequence(s_id, t);
		}
	}
}

struct Node_flow_info
{
	int p_id; //流的种类
	LL sum = 0; //这种流的总量
};
bool cmp_Node_flow_info(Node_flow_info a, Node_flow_info b)
{
	return a.sum > b.sum;
}
/////考虑中心节点的预分配
void pre_allocate_all_free3()
{
	/*服务器挑选*/
	vector<int>s_id_vec;
	for (int i = 1; i <= N; i++)
	{
		s_id_vec.push_back(i);
	}
	//服务器排序，决定谁先进行预分配
	sort(s_id_vec.begin(), s_id_vec.end(), cmp_server_for_pre_allocate);

	for (int s_idx = 0; s_idx < N; s_idx++)  //按排好的顺序考虑服务器【待修改，也许随机打乱更好
	{
		int s_id = s_id_vec[s_idx];   //取出服务器id
		now_s_id_in_pre_allocate = s_id; //改全局变量，当前考虑到的服务器是s_id
		if (server[s_id].degree() == 0)continue; //没有出度的服务器，没有任何利用价值
		int c_len = server[s_id].neighbor.size();
		/*时刻选择*/
		//同一个服务器的percent5次预分配都发生于不同时刻
		vector<Node_pre>sum_per_day;
		for (int t = 1; t <= T; t++)
		{
			Node_pre nod; nod.tim = t; nod.demand_sum = 0; nod.average = 0;
			for (int c_idx = 0; c_idx < c_len; c_idx++)
			{
				int c_id = server[s_id].neighbor[c_idx];
				nod.demand_sum += D_client_moment[c_id][t];
			}
			int p_num_in_t = 0;
			int pNum = PieceNumber_t[t];
			//统计t时刻有多少个小片
			for (int c_idx = 0; c_idx < c_len; c_idx++)
			{
				int c_id = server[s_id].neighbor[c_idx];
				for (int p_id = 1; p_id <= pNum; p_id++)
				{
					LL a_piece = D_t_c_p[t][c_id][p_id];
					if (a_piece)
					{
						p_num_in_t++;
					}
				}
			}
			//求小片大小均值
			nod.average = nod.demand_sum*1.0 / p_num_in_t;
			sum_per_day.push_back(nod);
		}
		//当demand_sum已经超过band_limit时，比较demand_sum已无意义，应该比较什么？
		sort(sum_per_day.begin(), sum_per_day.end(), cmp_Node_pre);
		/*吸收方式*/
		for (int t_idx = 0; t_idx < percent5; t_idx++) //前percent5次，要么导致满载，要么吸光所有客户，且发生在邻居需求最高时刻
		{
			//【待修改，此处可以通过最大片的大小等信息，再次决定是否要在t_idx发生预分配
			int t = sum_per_day[t_idx].tim;
			now_t_in_pre_allocate = t; //改全局变量
			pre_allocate_server_moment[s_id][t] = 1; //记录s_id在t时刻发生预分配
			int pNum = PieceNumber_t[t];
			vector<Node_flow_info>flow_info_vec;     //统计每种流的总和
			for (int p_id = 1; p_id <= pNum; p_id++)
			{
				Node_flow_info u; u.p_id = p_id; u.sum = 0;
				flow_info_vec.push_back(u);
			}
			for (int c_idx = 0; c_idx < c_len; c_idx++)
			{
				int c_id = server[s_id].neighbor[c_idx];
				for (int p_id = 1; p_id <= pNum; p_id++)
				{
					LL a_piece = D_t_c_p[t][c_id][p_id];
					flow_info_vec[p_id - 1].sum += a_piece;
				}
			}
			//不同p_id，按小片总和从大到小排序
			sort(flow_info_vec.begin(), flow_info_vec.end(), cmp_Node_flow_info);
			for (int f_idx = 0; f_idx < pNum; f_idx++)
			{
				LL capacity = server[s_id].band_limit - server[s_id].used_band[t];
				if (capacity <= 0)break; //服务器满载（这种情况很难出现）
				int p_id = flow_info_vec[f_idx].p_id;
				LL  p_sum = flow_info_vec[f_idx].sum;
				if (capacity < p_sum)continue; //这种流不能完全吸干，就选择不吸收，跳到下一个
				for (int c_idx = 0; c_idx < c_len; c_idx++)
				{
					int c_id = server[s_id].neighbor[c_idx];
					if (D_t_c_p[t][c_id][p_id])
					{
						assign_a_piece(t, c_id, p_id, s_id);
					}
				}
			}
			keep_fare_sequence(s_id, t);
		}
	}
}




//考虑了服务器全空的情况，以及免费机会内的情况，【未考虑中心节点
LD delta_Cost_if_add_piece_for_server(int s_id, int t, LL a_piece)//t时刻如果把a_piece放入服务器s_id，将导致的成本增量，（同时考虑三段计费函数）
{
	if (server[s_id].band_limit - server[s_id].used_band[t] < a_piece)return 1e8; //根本放不下，返回极大值，表明放此服务器不可行
	if (pre_allocate_server_moment[s_id][t])return 0; //如果是在免费机会，则没有任何成本增量
	LD p95_band = server[s_id].p95_band();      //原来的p95
	LD band_limit = server[s_id].band_limit;    //服务器带宽上限
	LD used_band_t = server[s_id].used_band[t]; //t时刻原来的用量
	LD new_used_band_t = used_band_t + a_piece; //加上这个片后的t时刻用量
	if (new_used_band_t <= p95_band)return 0; //t时刻加入这个片后，没有突破p95，成本必定不增加
	//否则意味着new_used_band_t突破了p95_band，但也许它并不是新的p95，要跟p96比一下，决定谁是新p95
	LD old_cost = 0; LD new_cost = 0;
	if (server[s_id].this_server_is_used == false)old_cost = 0;
	else if (p95_band <= V)old_cost = V;
	else old_cost = (p95_band - V)*(p95_band - V)*0.95 / band_limit + p95_band;   //【由于读入的band_limit乘过了0.95，需要在分子同时乘0.95约掉，不过其实在缓存机制下，used都是不精确的
	LD p96_band = server[s_id].p96_band();
	LD new_p95_band = min(new_used_band_t, p96_band);
	if (new_p95_band <= V)new_cost = V;
	else new_cost = (new_p95_band - V)*(new_p95_band - V)*0.95 / band_limit + new_p95_band;
	LD ret = new_cost - old_cost;
	return ret;
}

//只考虑中心节点
LD delta_Cost_if_add_piece_for_Center(int s_id, int t, int p_id, LL a_piece) //t时刻如果把a_piece放入服务器s_id，将导致的中心成本至少增量
{
	LL pre_max_size = MS_t_s_p[t][s_id][p_id];
	if ( pre_max_size>= a_piece)return 0;  //不会更新最大值，中心用量丝毫不增加
	LL delta = a_piece - pre_max_size;  //将导致的中心节点t时刻带宽增量
	LL new_MS_t = MS_t[t] + delta;
	LL center_p95_band = center.p95_band();
	int center_p95_tim = center.p95_tim();
	if (new_MS_t <= center_p95_band)   //也算是扔到了中心节点的免费空间中，但不应该是完全没代价
	{
		return 1.0 / (center_p95_band - MS_t[t]);
	}
	LD pre_center_cost = A*center_p95_band;
	LD new_center_cost = A*new_MS_t;
	return new_center_cost - pre_center_cost;
}


void first_schedule()
{
	//三个for皆可排序，也可混合，比如一个时刻一个时刻地考虑，但时刻内的c_id和p_id混合考虑
	//【然而，如果前面的会影响后面的，则应当不要固定排序，每次重新遍历，选一个才是最正确的
	for (int t = 1; t <= T; t++)
	{
		int pNum = PieceNumber_t[t];
		//第二层应该是流的种类，还是客户机，或者是混合？
		for (int p_id = 1; p_id <= pNum; p_id++)  //pNum<=100
		{
			for (int c_id = 1; c_id <= M; c_id++) //M<=35
			{
				LL& a_piece = D_t_c_p[t][c_id][p_id];
				if (a_piece == 0)continue;
				//决定这个小片分给谁
				int s_len = client[c_id].degree();
				for (int s_idx = 0; s_idx < s_len; s_idx++)
				{
					int s_id = client[c_id].neighbor[s_idx];
					LL free_capacity = server[s_id].free_space_for_server(t) - server[s_id].used_band[t];
					if (free_capacity >= a_piece)
					{
						assign_a_piece(t, c_id, p_id, s_id);
						keep_fare_sequence(s_id, t);
						break;
					}
				}
				a_piece = D_t_c_p[t][c_id][p_id];
				if (a_piece == 0)continue;
				LD minEva = 1e7; int selected_s = 0;
				for (int s_idx = 0; s_idx < s_len; s_idx++)
				{
					int s_id = client[c_id].neighbor[s_idx];
					LD eva = delta_Cost_if_add_piece_for_server(s_id, t, a_piece);
					if (eva < minEva)
					{
						minEva = eva; selected_s = s_id;
					}
				}
				assign_a_piece(t, c_id, p_id, selected_s);
				keep_fare_sequence(selected_s, t);
			}
		}
	}
}


void first_schedule2()  //全局排序的初次分配版本
{
	regenerate_center_fare_sequence(true); //完成预分配后，生成中心节点的当前计费序列，在初次分配时可以利用

	vector<Node_piece>piece_vec;
	//将所有小片串成一个队列
	for (int t = 1; t <= T; t++)
	{
		int pNum = PieceNumber_t[t];
		for (int c_id = 1; c_id <= M; c_id++)
		{
			for (int p_id = 1; p_id <= pNum; p_id++)
			{
				if (D_t_c_p[t][c_id][p_id]) //只有非空小片才需要分配
				{
					Node_piece pie(t, c_id, p_id);
					piece_vec.push_back(pie);
				}
			}
		}
	}
	//所有小片排序（静态顺序）大的在前
	sort(piece_vec.begin(), piece_vec.end(), cmp_Node_piece_for_first_schedule2);
	int total_piece_num = piece_vec.size();
	//为所有小片找到归属服务器
	for (int node_idx = 0; node_idx < total_piece_num; node_idx++)
	{
		Node_piece piece = piece_vec[node_idx];
		int c_id = piece.c_id;
		int t = piece.tim;
		int p_id = piece.p_id;
		int s_len = client[c_id].degree();
		LL& a_piece = D_t_c_p[t][c_id][p_id];
		/*
		//先寻找免费空间，如何使得【免费空间尽量充分利用】，如何选择最合适的免费服务器？
		//最大适配法：选择最大的免费空间进行放入（目前是这种效果较好）
		//最小适配法：选择最小的免费空间进行放入
		LL max_free_capacity = 0; int max_free_s_id = 0;
		//LL min_free_capacity = 1e7; int min_free_s_id = 0;
		for (int s_idx = 0; s_idx < s_len; s_idx++)
		{
			int s_id = client[c_id].neighbor[s_idx];
			LL free_capacity = server[s_id].free_space_for_server(t) - server[s_id].used_band[t];
			if (free_capacity >= a_piece&&free_capacity > max_free_capacity)
			{
				max_free_capacity = free_capacity;
				max_free_s_id = s_id;
			}
		}
		if (max_free_s_id != 0) //最大适配法
		{
			assign_a_piece(t, c_id, p_id, max_free_s_id);
			keep_fare_sequence(max_free_s_id, t);
			continue;
		}
		//如果没能放到免费空间中，则选取放了之后成本增量最小的服务器放入
		//在无免费空间可用的情况下，应该放到哪个服务器上？
		*/

		LD minEva = 1e7; int selected_s = 0;
		LL minCapacity = 1e7; LL max_free_Capacity = 0;
		for (int s_idx = 0; s_idx < s_len; s_idx++)
		{
			int s_id = client[c_id].neighbor[s_idx];
			LL capacity = server[s_id].band_limit - server[s_id].used_band[t];
			if (capacity < a_piece)continue; //根本放不下，此服务器不可行
			LD server_delta = delta_Cost_if_add_piece_for_server(s_id, t, a_piece);  //评估把小片放入s_id的代价（成本至少增量）
			LD center_delta = delta_Cost_if_add_piece_for_Center(s_id, t, p_id, a_piece);
			LD k = 1.0; //这个k调很大也没什么变化，说明center_delta总是接近0，因为大部分的小片，都不会直接改变中心节点的95带宽，而是间接影响
			LD eva = server_delta + k*center_delta;
			LL free_capacity = server[s_id].free_space_for_server(t) - server[s_id].used_band[t];
			if (eva < minEva&&fabs(minEva-eva)>1e-12)  
			{
				max_free_Capacity = free_capacity;
				minEva = eva; selected_s = s_id;
			}
			else if (fabs(minEva - eva) <= 1e-12)  //多个eva相近的情况，例如多个eva为0
			{
				if (free_capacity>max_free_Capacity)  //最大适配法
				{
					max_free_Capacity = free_capacity;
					minEva = eva; selected_s = s_id;
				}
			}
		}
		assign_a_piece(t, c_id, p_id, selected_s);
		keep_fare_sequence(selected_s, t);
		center.keep_center_fare_sequence(t);
	}
}


string input_PATH = "";
void read_Q_and_V_and_A()
{
	string config_PATH = input_PATH + "config.ini";
	freopen(config_PATH.c_str(), "r", stdin);
	string str;
	getline(cin, str, '=');
	cin >> Q;
	getline(cin, str, '=');
	cin >> V;
	getline(cin, str, '=');
	cin >> A;
	cin.clear(); //清空了才能重定向到其他文件
	//cout << "Q=" << Q << ", V=" << V <<", A="<<A<<"\n";
}
bool is_character(char ch)
{
	return ch >= 'A' && ch <= 'Z' || ch >= 'a' && ch <= 'z' || ch >= '0' && ch <= '9';
}
void read_demand() //隐患：时刻可能不能保证是从古到今，但应该不影响，毕竟输出是按照输入的时刻顺序
{
	string demand_PATH = input_PATH + "demand.csv";
	freopen(demand_PATH.c_str(), "r", stdin);
	string first_line;
	getline(cin, first_line);
	int len = first_line.length();
	int p = 0; while (first_line[p] != 'd')p++; p++;
	p++;
	//现在p指向"stream_id,"的后一个字符
	while (p < len)
	{
		M++;
		while (p < len && first_line[p] != ',')
		{
			if (is_character(first_line[p]))
			{
				client[M].name += first_line[p];
			}
			p++;
		}
		p++;
		//保存 (名字->下标) 映射
		mp_client_to_id[client[M].name] = M;
	}
	if (cin.peek() == '\n')cin.get();
	T++;
	string moment_name;
	string last_moment_name = "";
	while (getline(cin, moment_name, ','))
	{
		if (cin.peek() == ',')cin.get();
		if (last_moment_name == "")last_moment_name = moment_name;
		if (moment_name != last_moment_name)
		{
			T++; PieceNumber_t[T] = 1; last_moment_name = moment_name; //换了一个时刻
		}
		else PieceNumber_t[T]++;
		getline(cin, PieceName_t_p[T][PieceNumber_t[T]], ',');
		for (int i = 1; i < M; i++)
		{
			cin >> D_t_c_p[T][i][PieceNumber_t[T]];
			D_client_moment[i][T] += D_t_c_p[T][i][PieceNumber_t[T]]; //顺便累加客户在t时刻的总需求量
			cin.get(); //读掉逗号
		}
		cin >> D_t_c_p[T][M][PieceNumber_t[T]];
		D_client_moment[M][T] += D_t_c_p[T][M][PieceNumber_t[T]];
		if (cin.peek() != EOF)cin.get();
		if (cin.peek() == '\n')cin.get();
	}
	cin.clear();

	/*
	//////测试
	cout << "M=" << M << ",T=" << T << "\n    ";
	for (int c_id = 1; c_id <= M; c_id++)
	{
		cout << "<" << client[c_id].name << ">";
	}
	cout << "\n";
	for (int t = 1; t <= T; t++)
	{
		int p_num = PieceNumber_t[t];
		cout << "时刻" << t << "有" << p_num << "种小片(流)\n";
		for (int p = 1; p <= p_num; p++)
		{
			cout << "<" << PieceName_t_p[t][p] << ">";
			LL p_sum = 0;
			for (int c_id = 1; c_id <= M; c_id++)
			{
				p_sum += D_t_c_p[t][c_id][p];
				cout << "<" << D_t_c_p[t][c_id][p] << ">";
			}
			cout << "<p_sum:" << p_sum << ">";
			cout << "\n";
		}
		cout << "sum:";
		for (int c_id = 1; c_id <= M; c_id++)
		{
			cout << "<" << D_client_moment[c_id][t] << ">";
		}
		cout << "\n";
	}
	*/
}
void read_site_bandwidth()
{
	string site_bandwidth_PATH = input_PATH + "site_bandwidth.csv";
	freopen(site_bandwidth_PATH.c_str(), "r", stdin);
	string first_line;
	getline(cin, first_line);
	N++;
	while (getline(cin, server[N].name, ','))
	{
		//保存 (名字->下标) 映射
		mp_server_to_id[server[N].name] = N;
		cin >> server[N].band_limit;
		if (cin.peek() != EOF)cin.get();
		if (cin.peek() == '\n')cin.get();
		N++;
	}
	N--;
	cin.clear();

	/*
	/////测试
	cout<<"N="<<N<<"\n";
	cout << "site_name,bandwidth\n";
	for (int i = 1; i <= N; i++)
	{
	cout <<"<"<< server[i].name << ">," << server[i].band_limit << "\n";
	}
	*/
}
void read_qos() //隐患应该已经消除
{
	string qos_PATH = input_PATH + "qos.csv";
	freopen(qos_PATH.c_str(), "r", stdin);
	string first_line;
	getline(cin, first_line);
	int len = first_line.length();
	int p = 0; while (first_line[p] != ',')p++;
	p++;
	//现在p指向"site_name,"的后一个字符
	int all_c_id[maxm]; int c_id_p = 0;
	while (p < len)
	{
		c_id_p++;
		string c_name = "";
		while (p < len&&first_line[p] != ',')
		{
			if (is_character(first_line[p]))
			{
				c_name += first_line[p];
			}
			p++;
		}
		p++;
		all_c_id[c_id_p] = mp_client_to_id[c_name];
		//cout << "firstline,c_name=<" << c_name << ">,and c_id=" << all_c_id[c_id_p] << "\n";
	}
	char ch = cin.peek();
	if (!is_character(ch))cin.get();
	ch = cin.peek();
	if (!is_character(ch))cin.get();
	string s_name;
	int s_id = 1;
	//这时候c_cnt==M
	while (getline(cin, s_name, ','))
	{
		s_id = mp_server_to_id[s_name];
		//cout << "server_name=<" << s_name << ">,and s_id=" << s_id << "\n";
		for (int i = 1; i < M; i++)
		{
			cin >> QoS_client_server[all_c_id[i]][s_id];
			if (QoS_client_server[all_c_id[i]][s_id] < Q)
			{
				client[all_c_id[i]].neighbor.push_back(s_id);
				server[s_id].neighbor.push_back(all_c_id[i]);
			}
			cin.get(); //读掉逗号
		}
		cin >> QoS_client_server[all_c_id[M]][s_id];  //行末的数字
		if (QoS_client_server[all_c_id[M]][s_id] < Q)
		{
			client[all_c_id[M]].neighbor.push_back(s_id);
			server[s_id].neighbor.push_back(all_c_id[M]);
		}
		char ch = cin.peek();
		if (ch == EOF)break;
		if (!is_character(ch))cin.get();
		ch = cin.peek();
		if (ch == EOF)break;
		if (!is_character(ch))cin.get();
	}
	cin.clear();

	/*
	///////测试
	for (int i = 1; i <= M; i++)cout <<"  "<< client[i].name ; cout << "\n";
	for (int j = 1; j <= N; j++)
	{
	cout << server[j].name << " ";
	for (int i = 1; i <= M; i++)
	{
	cout << QoS_client_server[i][j] << " ";
	}
	cout << "\n";
	}
	*/
}
void output_ans(int X_t_c_p[maxt][maxm][maxp])
{
	map<int, vector<int> >::iterator iter;
	for (int t = 1; t <= T; t++)
	{
		int pNum = PieceNumber_t[t];
		for (int c_id = 1; c_id <= M; c_id++)
		{
			cout << client[c_id].name << ":";
			map<int, vector<int> >mp;
			for (int p_id = 1; p_id <= pNum; p_id++)
			{
				int s_id = X_t_c_p[t][c_id][p_id];
				if (s_id == 0)continue;
				if (mp.count(s_id))
				{
					mp[s_id].push_back(p_id);
				}
				else
				{
					vector<int>vec; vec.push_back(p_id);
					mp[s_id] = vec;
				}
			}
			int comma_flag = 0;
			for (iter = mp.begin(); iter != mp.end(); iter++)
			{
				int s_id = iter->first;
				vector<int>&vec = iter->second;
				if (comma_flag)cout << ",";
				cout << "<" << server[s_id].name;
				for (auto p_id : vec)
				{
					cout << "," << PieceName_t_p[t][p_id];
				}
				cout << ">";
				comma_flag = 1;
			}
			cout << "\n";
		}
	}
}
void copy_D_to_D0()
{
	for (int c_id = 1; c_id <= M; c_id++)
	{
		for (int t = 1; t <= T; t++)
		{
			D0_client_moment[c_id][t] = D_client_moment[c_id][t];
		}
	}
	for (int t = 1; t <= T; t++)
	{
		int pNum = PieceNumber_t[t];
		for (int c_id = 1; c_id <= M; c_id++)
		{
			for (int p = 1; p <= pNum; p++)
			{
				D0_t_c_p[t][c_id][p] = D_t_c_p[t][c_id][p];
			}
		}
	}
}
void make_adjacency_matrix()
{
	for (int c_id = 1; c_id <= M; c_id++)
	{
		for (int s_id = 1; s_id <= N; s_id++)
		{
			if (QoS_client_server[c_id][s_id] < Q)
			{
				adjacency_c_s[c_id][s_id] = 1;
			}
		}
	}
}
void read_and_copy()
{
	read_Q_and_V_and_A();
	read_demand();
	read_site_bandwidth();
	read_qos();
	copy_D_to_D0(); //备份读入的初始需求，只读不改
	make_adjacency_matrix();
	for (int s_id = 1; s_id <= N; s_id++) //处理缓存机制的暂行办法
	{
		server[s_id].band_limit *= 0.95;
	}
}
struct Node_happen_times
{
	int tim = 0;
	int happen_times = 0;
}happen_times[maxt];
bool cmp_Node_happen_times(Node_happen_times a, Node_happen_times b)
{
	return a.happen_times > b.happen_times;
}
void output_all_server_cost()//以服务器为研究对象
{
	percent5 = T - percent95; //恢复percent5
	for (int s_id = 1; s_id <= N; s_id++)
	{
		server[s_id].band_limit /= 0.95;   //恢复band_limit
	}
	regenerate_center_fare_sequence();
	cout << "中心节点：\n";
	LD center_p95_band = center.p95_band();
	int center_p95_tim = center.p95_tim();
	LD center_p96_band = center.p96_band();
	int center_p96_tim = center.p96_tim();
	LD center_p100_band = center.p100_band();
	int center_p100_tim = center.p100_tim();
	LD center_cost = center_p95_band*A;
	int center_len = center.fare_sequence.size();
	cout << "中心费用" << center_cost << ", 计费序列长度" << center_len << ",    p95=(" << center_p95_band << " , " << center_p95_tim << ") ,    p96=(" << center_p96_band << " , " << center_p96_tim << "),    p100=(" << center_p100_band << " , " << center_p100_tim << ")\n";
	cout << "边缘节点：\n";
	for (int s_id = 1; s_id <= N; s_id++)
	{
		regenerate_fare_sequence(s_id);
	}
	LD total_cost = 0;
	cout << "    服务器名\t邻居数\t带宽上限\t导致成本\t计费序列长度\tp95(用量,时刻)\tp96(用量,时刻)\tp100(用量,时刻)\n";
	for (int s_id = 1; s_id <= N; s_id++)
	{
		if (server[s_id].degree() == 0)continue;
		int sequence_length = server[s_id].fare_sequence.size();
		LD p95_band = server[s_id].p95_band();
		int p95_tim = server[s_id].p95_tim();
		LD p96_band = server[s_id].p96_band();
		int p96_tim = server[s_id].p96_tim();
		LD p100_band = server[s_id].p100_band();
		int p100_tim = server[s_id].p100_tim();
		LD this_cost = how_much_this_server_cost_now(s_id);
		total_cost += this_cost;
		cout << s_id << "号" << server[s_id].name << "\t\t" << server[s_id].degree() << "\t" << server[s_id].band_limit << "\t\t" << this_cost << "\t\t" << sequence_length << "\t\t"
			<< p95_band << "," << p95_tim << "\t\t" << p96_band << "," << p96_tim << "\t\t" << p100_band << "," << p100_tim << "\n";
	}
	cout << "边缘结点总成本：" << total_cost << "\n";
	cout << "最终总成本=" << total_cost << "+" << center_cost << "=" << total_cost + center_cost << "\n\n";

	cout << "中心节点计费序列：(排位：用量，时刻)\n";
	for (int i = 0; i < center_len; i++)
	{
		cout << i + 1 << ": " << center.fare_sequence[i].used << " , " << center.fare_sequence[i].tim << "\n";
	}
	for (int t = 1; t <= T; t++)happen_times[t].tim = t;
	cout << "边缘节点计费序列(名字 排位: 用量，时刻)：\n";
	for (int s_id = 1; s_id <= N; s_id++)
	{
		if (server[s_id].degree() == 0)continue;
		cout << s_id << "号" << server[s_id].name << ": \n";
		int len = server[s_id].fare_sequence.size();
		for (int i = 0; i < len; i++)
		{
			int tim = server[s_id].fare_sequence[i].tim;
			cout << s_id << "号" << server[s_id].name << " " << i + 1 << ": " << server[s_id].fare_sequence[i].used << " , " << tim << "\n";
			if (i < percent5)
			{
				happen_times[tim].happen_times++;
			}
		}
	}
	sort(happen_times + 1, happen_times + T + 1, cmp_Node_happen_times);
	for (int i = 1; i <= T; i++)
	{
		//if (happen_times[i].happen_times == 0)break;
		cout << "在t=" << happen_times[i].tim << "时刻,发生" << happen_times[i].happen_times << "次免费机会\n";
	}
}

int main()
{
	srand(unsigned(time(0)));
	start_time = clock();
	std::ios_base::sync_with_stdio(false);
	cin.tie(NULL); cout.tie(NULL);
	//freopen("C:/Users/Touchlion/Desktop/fighting/ACM/2022hwsoft/复赛/复赛分析工具/v0/solution.txt", "w", stdout);
	//freopen("C:/Users/Touchlion/Desktop/fighting/ACM/2022hwsoft/CodeCraft-2022/CodeCraft-2022/决赛分析/分析结果(per5+v1pre度大于3才用+v2fir考虑中心k为1).txt", "w", stdout);
	//将cout重定向到solution.txt中，【提交时】一定要有这句
	freopen("/output/solution.txt", "w", stdout);
	input_PATH = "/data/";

	read_and_copy(); //读入全部

	percent95 = ceil(T * 0.95);
	percent5 = T - percent95;
	percent5 /=2;
	pre_allocate_all_free();

	first_schedule2();

	//output_all_server_cost(); /////【调试时】，输出当前方案的成本信息


	output_ans(X_t_c_p);
}
