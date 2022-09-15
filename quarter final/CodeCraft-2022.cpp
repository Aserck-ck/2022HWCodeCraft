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
LL percent95; //T*0.95 向上取整
LL percent5;  //T-percent95
LL percent_90;
LL percent_10;
vector<int>chose_s_id_for_change;
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
int pre_allocate_piece_t_c_p[maxt][maxm][maxp];  //为1表示 t时刻 c_id号客户 的第p_id号小片是预分配时确定归属的小片
bool cmp_bigger(LL a, LL b)
{
	return a > b;
}
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
struct Server
{
	string name = "";
	LL band_limit = 0; //带宽上限
	LL used_band[maxt]; //记录这台服务器每个时刻的已用带宽和，其值不能超过band_limit
	vector<int>neighbor; //记录所有相连的客户id
	bool is_chosed_for_90 = false;
	int degree() { return neighbor.size(); }
	vector<Node_fare>fare_sequence; //实时维护的计费序列，分配完一个时刻t，就把这个时刻的用量加入计费队列，并重新排序
	LL p95_band()  //返回目前序列中记录的95带宽
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
	LL p90_band()
	{
		int len = fare_sequence.size();
		if (len <= percent_10)return 0;
		return fare_sequence[percent_10].used;
	}
	int p90_tim()
	{
		int len = fare_sequence.size();
		if (len <= percent_10)return 0;
		return fare_sequence[percent_10].tim;
	}
	LL free_space(int t) //免费空间（不是指剩余哦！），当95带宽超过V时，为95带宽值；若不超过，则为V
	{
		if (is_chosed_for_90)
		{
			int p90_t = p90_tim();
			LL p90_b = p90_band();
			if (p90_t != t)return max(p90_b, V);
			//计费序列中记录的95带宽正是当前时刻，如果已经超过V，则无免费空间
			if (p90_b < V)return V;
			return 0;
		}
		int p95_t = p95_tim();
		LL p95_b = p95_band();
		if (p95_t != t)return max(p95_b, V);
		//计费序列中记录的95带宽正是当前时刻，如果已经超过V，则无免费空间
		if (p95_b < V)return V;
		return 0;
	}

}server[maxn];
void keep_fare_sequence(int s_id, int t) //用used_band[t]维护计费序列
{
	LL used = server[s_id].used_band[t];
	LL p95_band = server[s_id].p95_band();
	if (used > p95_band) //只有当used超过原来的95带宽，才需要更新计费序列
	{
		vector<Node_fare>& fs = server[s_id].fare_sequence;
		int len = fs.size();
		for (int i = 0; i < len; i++) //先找找此时刻是否已经在计费队列中
		{
			if (fs[i].tim == t) //如果已经在队列中，则更新使用量，重新排序即可
			{
				fs[i].used = used;
				sort(fs.begin(), fs.end(), cmp_Node_fare);
				return;
			}
		}
		//属于是新计入队列的时刻（未必是新，也可能它之前的用量在队列里排不上号
		Node_fare u(used, t);
		fs.push_back(u);
		sort(fs.begin(), fs.end(), cmp_Node_fare);
		if (server[s_id].is_chosed_for_90)
		{
			while (fs.size() > percent_10 + 1)
			{
				fs.pop_back();
			}
		}
		else
		{
			while (fs.size() > percent5 + 1)
			{
				fs.pop_back();
			}
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
map<string, int>mp_client_to_id; //从客户名字到下标
map<string, int>mp_server_to_id; //从服务器名字到下标

void assign_a_piece(int& t, int& c_id, int& p_id, int& s_id)//将t时刻客户c_id的第p_id号小片分配给服务器s_id
{
	server[s_id].used_band[t] += D_t_c_p[t][c_id][p_id];
	X_t_c_p[t][c_id][p_id] = s_id;
	D_client_moment[c_id][t] -= D_t_c_p[t][c_id][p_id];
	D_t_c_p[t][c_id][p_id] = 0;
}

//预分配
struct Node_pre
{
	LL demand_sum = 0;
	int tim;
	LD average = 0; //（小片大小）均值
	LD variance = 0; //方差
};
//预分配当前考虑的是哪个服务器
int now_s_id_in_pre_allocate = 0;
bool cmp_Node_pre(Node_pre a, Node_pre b)
{
	LL band_limit = server[now_s_id_in_pre_allocate].band_limit;
	//当两个时刻的需求量都超过服务器带宽上限时，比较需求大小已经无意义
	LD k = 1; //0.95已经不行
	if (a.demand_sum >= band_limit * k && b.demand_sum >= band_limit * k)
	{//均值大的好一些，但为什么方差不能用来衡量呢
		return a.average > b.average;
		//return a.variance > b.variance;
	}
	return a.demand_sum > b.demand_sum;
}
bool cmp_server_for_pre_allocate(int i, int j) //这太怪了
{
	return server[i].band_limit > server[j].band_limit;
}

int now_t_in_pre_allocate = 0;
struct cmp_for_pre_allocate { //用于优先队列
	bool operator ()(int i, int j) { //返回ture时j在堆顶，与vector的自定义排序正好不一样
		return D_client_moment[i][now_t_in_pre_allocate] < D_client_moment[j][now_t_in_pre_allocate];
	}
};
struct Fake_Server
{
	LL used_band[maxt];
}fake_server[maxn];

LL piece_size_threshold = 6000; //片大小的阈值，大小达到此值的片视为异常值，在排序的时候予以重视
//描述一个时刻中模拟预分配的结果
struct Node_fake_t_result
{
	int tim;  //进行模拟的时刻
	LL demand_sum = 0;    //该时刻邻居总需求
	LL used_band = 0;   //实际装入的需求
	int put_piece_num = 0;    //装入的片数
	LL max_piece_size = 0;    //装入的最大片的size
	int threshold_exceeded_num = 0;  //装入的片中，大小超过阈值的片数
	Node_fake_t_result(int Tim, LL D_sum, LL Used_band, int Put_num, LL Max_size, int Thre_num)
	{
		tim = Tim;
		demand_sum = D_sum; used_band = Used_band; put_piece_num = Put_num; max_piece_size = Max_size; threshold_exceeded_num = Thre_num;
	}
	LD evaluate() //返回对这个装入效果的评估值，越大则认为此装入效果越好
	{
		LD band_limit = server[now_s_id_in_pre_allocate].band_limit;
		return 100.0 * used_band / band_limit + 0.05 * used_band / put_piece_num + 10.0 * threshold_exceeded_num;
	}
};
//如何衡量两个时刻，哪个更适合发生预分配？
//应当综合考虑到服务器本身性质，时刻总需求，模拟装入的结果
bool cmp_Node_fake_t_result(Node_fake_t_result a, Node_fake_t_result b)
{
	////当a和b时刻装入量差距较小时，应该比较其他的
	//LD eps = 0.005;
	//if (fabs(a.used_band*1.0 - b.used_band) / a.used_band < eps)
	//{
	//	return a.used_band*1.0 / a.put_piece_num > b.used_band*1.0 / b.put_piece_num;
	//}
	//return a.evaluate() > b.evaluate();
	LL band_limit = server[now_s_id_in_pre_allocate].band_limit;
	if (a.demand_sum >= band_limit && b.demand_sum >= band_limit)
	{
		//if (a.threshold_exceeded_num != b.threshold_exceeded_num)
		//{
		//	return a.threshold_exceeded_num > b.threshold_exceeded_num;
		//}
		return a.used_band * 1.0 / a.put_piece_num > b.used_band * 1.0 / b.put_piece_num;
	}
	return a.demand_sum > b.demand_sum;
}
bool cmp_pre_chose_s_for_90(int i, int j)
{
	return server[i].degree() > server[j].degree();
}
void pre_allocate_all_free2() //预分配操作版本2，先在每个时刻模拟演练分配，再根据每个时刻的装入效果，决定要在哪几个时刻真正进行预分配
{


	vector<int>s_id_vec;
	for (int i = 1; i <= N; i++)
	{
		s_id_vec.push_back(i);
	}
	//服务器排序，决定谁先进行预分配
	sort(s_id_vec.begin(), s_id_vec.end(), cmp_server_for_pre_allocate);

	vector<int>beixuan_s_id_vec_for_90;
	for (int s_id = 1; s_id <= N; s_id++)
	{
		beixuan_s_id_vec_for_90.push_back(s_id);
	}
	sort(beixuan_s_id_vec_for_90.begin(), beixuan_s_id_vec_for_90.end(), cmp_pre_chose_s_for_90);

	int chose_s_cnt = 0;
	


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
			//记录装入的片数，大小达到阈值的片数，装入的最大片的大小，时刻总需求量
			int put_piece_num = 0; int threshold_exceeded_num = 0; LL max_piece_size = 0; LL demand_sum = 0;
			for (int node_idx = 0; node_idx < piece_num; node_idx++)
			{
				//跟真预分配一样，从大到小填入小片
				Node_piece piece = fake_piece_vec[node_idx];
				int c_id = piece.c_id;
				int p_id = piece.p_id;
				LL piece_size = D_t_c_p[fake_t][c_id][p_id];
				demand_sum += piece_size;                //记录需求之和
				//注意，模拟分配时不能改动任何东西，除了专门为这个过程准备的fake_server的used_band[ ]
				LL capacity = server[s_id].band_limit - fake_server[s_id].used_band[fake_t];

				if (capacity < piece_size)continue; //放不下

				//模拟放入此小片，同步记录一些效果衡量值
				if (piece_size >= piece_size_threshold)  //大小超过阈值，计数
				{
					threshold_exceeded_num++;
				}
				if (piece_size > max_piece_size)           //最大片，更新（实际上也就是装入的第一个小片
				{
					max_piece_size = piece_size;
				}
				put_piece_num++;                         //总共装了几个小片，计数
				fake_server[s_id].used_band[fake_t] += piece_size;    //此时刻用量累计

			}///fake_t时刻的小片队列遍历分配完成
			//保存记录的信息及本时刻，存入队列中
			////构造Node_fake_t_result(int Tim,LL D_sum, LL Used_band, int Put_num, LL Max_size, int Thre_num)
			Node_fake_t_result result(fake_t, demand_sum, fake_server[s_id].used_band[fake_t], put_piece_num, max_piece_size, threshold_exceeded_num);
			result_vec.push_back(result);
		}
		//以上，所有时刻中都进行了一次模拟分配
		sort(result_vec.begin(), result_vec.end(), cmp_Node_fake_t_result);
		//排好序，前面percent5个记录的时刻，就是应该决定要发生预分配的时刻，下面进行真预分配
		LL the_free_time_to_pre = 0;
		if (result_vec[percent_10 - 1].used_band >= server[s_id].band_limit / 5&&chose_s_cnt<10) {
			server[s_id].is_chosed_for_90 = true;
			chose_s_cnt++;
			chose_s_id_for_change.push_back(s_id);
		}
		if (server[s_id].is_chosed_for_90)the_free_time_to_pre = percent_10;
		else the_free_time_to_pre = percent5;
		for (int t_idx = 0; t_idx < the_free_time_to_pre; t_idx++) //前percent5次，要么导致满载，要么吸光所有客户，且发生在邻居需求最高时刻
		{
			int t = result_vec[t_idx].tim;
			now_t_in_pre_allocate = t; //改全局变量，用于优先队列
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
				///此小片为预分配时就确定归属的小片，迁移时不可移动
				pre_allocate_piece_t_c_p[t][c_id][p_id] = 1;
			}
			//整个时刻考虑完成，维护计费队列
			keep_fare_sequence(s_id, t);
		}
	}
	for (int s_idx = 0; s_idx < N; s_idx++)
	{
		int s_id = beixuan_s_id_vec_for_90[s_idx];
		if (server[s_id].degree() != 0&& server[s_id].is_chosed_for_90==false)
		{
			chose_s_cnt++;
			chose_s_id_for_change.push_back(s_id);
			if (chose_s_cnt == 10)break;
		}
	}

}
//////////////预分配第一版本如下，使用均值进行选择时刻
void pre_allocate_all_free() //预分配操作
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
		if (server[s_id].degree() == 0)continue; //没有出度的服务器，没有任何利用价值
		//复制该服务器的邻居队列
		vector<int>s_neighbor;
		int len = server[s_id].neighbor.size();
		for (int c_idx = 0; c_idx < len; c_idx++)
		{
			int c_id = server[s_id].neighbor[c_idx];
			s_neighbor.push_back(c_id);
		}
		//同一个服务器的percent5次预分配都发生于不同时刻
		vector<Node_pre>sum_per_day;
		for (int t = 1; t <= T; t++)
		{
			Node_pre nod; nod.tim = t; nod.demand_sum = 0; nod.average = 0; nod.variance = 0;
			for (int c_idx = 0; c_idx < len; c_idx++)
			{
				nod.demand_sum += D_client_moment[s_neighbor[c_idx]][t];
			}
			int p_num_in_t = 0;
			int pNum = PieceNumber_t[t];
			//统计t时刻有多少个小片
			for (int c_idx = 0; c_idx < len; c_idx++)
			{
				int c_id = s_neighbor[c_idx];
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
			nod.average = nod.demand_sum * 1.0 / p_num_in_t;
			//求方差
			LD square_sum = 0; //差的平方和
			for (int c_idx = 0; c_idx < len; c_idx++)
			{
				int c_id = s_neighbor[c_idx];
				for (int p_id = 1; p_id <= pNum; p_id++)
				{
					LL a_piece = D_t_c_p[t][c_id][p_id];
					if (a_piece)
					{
						square_sum += (a_piece - nod.average) * (a_piece - nod.average);
					}
				}
			}
			nod.variance = square_sum * 1.0 / p_num_in_t;
			sum_per_day.push_back(nod);
		}
		//当demand_sum已经超过band_limit时，比较demand_sum已无意义，应该比较什么？大号片的数量！希望挑出大号片！
		sort(sum_per_day.begin(), sum_per_day.end(), cmp_Node_pre);
		for (int t_idx = 0; t_idx < percent5; t_idx++) //前percent5次，要么导致满载，要么吸光所有客户，且发生在邻居需求最高时刻
		{
			int t = sum_per_day[t_idx].tim;
			now_t_in_pre_allocate = t; //改全局变量，用于优先队列
			pre_allocate_server_moment[s_id][t] = 1; //记录s_id在t时刻发生预分配
			vector<Node_piece>piece_vec;
			//将t时刻的所有邻居小片都放入队列中
			int pNum = PieceNumber_t[t];
			for (int c_idx = 0; c_idx < len; c_idx++)
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
//////////////预分配第一版本如上




LD delta_Cost_if_add_piece(int s_id, int t, LL a_piece) //t时刻如果把a_piece放入服务器s_id，将导致的成本增量，（同时考虑三段计费函数）
{
	if (server[s_id].band_limit - server[s_id].used_band[t] < a_piece)return 1e8; //根本放不下，返回极大值，表明放此服务器不可行
	LD p95_band = server[s_id].p95_band();
	if (p95_band < V)p95_band = V; //
	LD band_limit = server[s_id].band_limit;
	LD used_band_t = server[s_id].used_band[t];
	LD delta_p95 = a_piece + used_band_t - p95_band;
	if (delta_p95 <= 0)return 0;
	LD S = (1.0 + (2.0 * p95_band + delta_p95 - 2.0 * V) / band_limit) * delta_p95;
	return S;
}
//公式2按理说应该比上面的更鲁棒一点，但是得分却变差了一点点
LD delta_Cost_if_add_piece2(int s_id, int t, LL a_piece)
{
	if (server[s_id].band_limit - server[s_id].used_band[t] < a_piece)return 1e8; //根本放不下，返回极大值，表明放此服务器不可行
	LD p95_band = server[s_id].p95_band();
	LD band_limit = server[s_id].band_limit;
	LD used_band_t = server[s_id].used_band[t];
	LD new_used_band_t = used_band_t + a_piece;
	if (new_used_band_t <= p95_band)return 0;
	//下面保证t时刻的用量突破了p95，两者也可能是同一时刻
	LD old_cost = 0, new_cost = 0;
	if (server[s_id].fare_sequence.size() == 0)old_cost = 0;
	else if (p95_band <= V)old_cost = V;
	else old_cost = (p95_band - V) * (p95_band - V) / band_limit + p95_band;
	if (new_used_band_t <= V)new_cost = V;
	else new_cost = (new_used_band_t - V) * (new_used_band_t - V) / band_limit + new_used_band_t;
	return new_cost - old_cost;
}
//默认前提：所有服务器均被用到，每个都要至少计费V（当V很大时不正确）
void first_schedule()
{
	//三个for皆可先排序，但是否还要这种架构
	for (int t = 1; t <= T; t++)
	{
		int pNum = PieceNumber_t[t];
		for (int c_id = 1; c_id <= M; c_id++) //是否一定要一个个客户进行处理？
		{
			//把小片分配到免费空间中，此过程不导致成本提高，也不改变95带宽记录

			for (int p_id = 1; p_id <= pNum; p_id++)
			{
				LL& a_piece = D_t_c_p[t][c_id][p_id];
				if (a_piece == 0)continue;
				//决定这个小片该分给谁
				int s_len = client[c_id].degree();
				for (int s_idx = 0; s_idx < s_len; s_idx++)
				{
					//暂时顺序放【是否可以选择最佳适配
					int s_id = client[c_id].neighbor[s_idx];
					//免费空间=max(p95_band, V)，当目前95带宽不超过V时，免费空间为V
					LL free_capacity = server[s_id].free_space(t) - server[s_id].used_band[t];
					if (free_capacity >= a_piece)//小片不可分割
					{
						assign_a_piece(t, c_id, p_id, s_id);
						break;
					}
				}
				//还存在某些小片还没分出去，集中处理，而不是立即处理
			}
		}
		//每个时刻集中处理没有分出去的小片，这些小片都无法放进免费空间中，也就是无论放在哪个服务器，都将改变95带宽，此过程导致成本提高
		for (int c_id = 1; c_id <= M; c_id++)
		{
			for (int p_id = 1; p_id <= pNum; p_id++)
			{
				LL& a_piece = D_t_c_p[t][c_id][p_id];
				if (a_piece == 0)continue;
				int s_len = client[c_id].degree();
				LD minEva = 1e7; int selected_s = 0;
				for (int s_idx = 0; s_idx < s_len; s_idx++)
				{
					int s_id = client[c_id].neighbor[s_idx];
					LD eva = delta_Cost_if_add_piece(s_id, t, a_piece);
					if (eva < minEva)
					{
						minEva = eva; selected_s = s_id;
					}
				}
				assign_a_piece(t, c_id, p_id, selected_s);
				/////////漏了维护！
				keep_fare_sequence(selected_s, t);
			}
		}
		//keep_all_fare_sequence(t);
	}
}

void first_schedule2()
{
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
		//先寻找免费空间，如何使得【免费空间尽量充分利用】，如何选择最合适的免费服务器？
		//最大适配法：选择最大的免费空间进行放入（目前是这种效果较好）
		//最小适配法：选择最小的免费空间进行放入
		LL max_free_capacity = 0; int max_free_s_id = 0;
		LL min_free_capacity = 1e7; int min_free_s_id = 0;
		for (int s_idx = 0; s_idx < s_len; s_idx++)
		{
			int s_id = client[c_id].neighbor[s_idx];
			LL free_capacity = server[s_id].free_space(t) - server[s_id].used_band[t];
			if (free_capacity >= a_piece && free_capacity > max_free_capacity)
			{
				max_free_capacity = free_capacity;
				max_free_s_id = s_id;
			}
			if (free_capacity >= a_piece && free_capacity < min_free_capacity)
			{
				min_free_capacity = free_capacity;
				min_free_s_id = s_id;
			}
		}
		if (max_free_s_id != 0) //最大适配法
		{
			assign_a_piece(t, c_id, p_id, max_free_s_id);
			keep_fare_sequence(max_free_s_id, t);
			continue;
		}
		//if (min_free_s_id != 0) //最小适配法
		//{
		//	assign_a_piece(t, c_id, p_id, min_free_s_id);
		//	keep_fare_sequence(min_free_s_id, t);
		//	continue;
		//}

		//如果没能放到免费空间中，则选取放了之后成本增量最小的服务器放入
		//在无免费空间可用的情况下，应该放到哪个服务器上？
		LD minEva = 1e7; int selected_s = 0;
		for (int s_idx = 0; s_idx < s_len; s_idx++)
		{
			int s_id = client[c_id].neighbor[s_idx];
			LD eva = delta_Cost_if_add_piece(s_id, t, a_piece);
			if (eva < minEva)
			{
				minEva = eva; selected_s = s_id;
			}
		}
		assign_a_piece(t, c_id, p_id, selected_s);
		keep_fare_sequence(selected_s, t);
	}
}


string input_PATH = "";
void read_Q_and_V()
{
	string config_PATH = input_PATH + "config.ini";
	freopen(config_PATH.c_str(), "r", stdin);
	string str;
	getline(cin, str, '=');
	cin >> Q;
	getline(cin, str, '=');
	cin >> V;
	cin.clear(); //清空了才能重定向到其他文件
	//cout << "Q=" << Q << ", V=" << V << "\n";
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
	for (int c_id = 1; c_id <= M; c_id++)
	{
	cout << "<" << D_t_c_p[t][c_id][p] << ">";
	}
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
		while (p < len && first_line[p] != ',')
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
	for (int i = 1; i <= M; i++)cout << client[i].name << " "; cout << "\n";
	for (int j = 1; j <= N; j++)
	{
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
	//cout << server[1].name;
	//for (int s_id = 2; s_id <= 10; s_id++)
	//{
	//	cout << "," << server[s_id].name;
	//}
	//cout << "\n";
	if (chose_s_id_for_change.size() >= 10)
	{
		cout << server[chose_s_id_for_change[0]].name;
		for (int i = 1; i < 10; i++)
		{
			cout << "," << server[chose_s_id_for_change[i]].name;
		}
		cout << "\n";
	}
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
				vector<int>& vec = iter->second;
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
		for (int c_id = 1; c_id <= M; c_id++)
		{
			for (int p = 1; p <= PieceNumber_t[T]; p++)
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
	read_Q_and_V();
	read_demand();
	read_site_bandwidth();
	read_qos();
	copy_D_to_D0(); //备份读入的初始需求，只读不改
	make_adjacency_matrix();
}
bool cmp_server_by_band_limit(int i, int j) //对每个客户的neighbor排序，排在首位的就是第一责任人，但未必就是最合理的第一责任人
{
	//4好于3好于2，说明预排序有很大问题
	//int threshold = 4;
	//if (server[i].degree() >= threshold && server[j].degree()<threshold)return true;
	//if (server[i].degree()<threshold && server[j].degree() >= threshold)return false;

	//优先比带宽上限，带宽上限大的在前，如果相同则度大的在前
	////////////为什么复赛又变成这样就可以优化？
	//if (server[i].degree() != server[j].degree())
	//{
	//	return server[i].degree() > server[j].degree();
	//}
	//return server[i].band_limit < server[j].band_limit;
	if (server[i].band_limit != server[j].band_limit)
	{
		return server[i].band_limit < server[j].band_limit;
	}
	return server[i].degree() < server[j].degree(); //练习阶段是度大的好，正式赛是度小的好
}
void pre_sort_client_neighbor()
{
	for (int c_id = 1; c_id <= M; c_id++)
	{
		sort(client[c_id].neighbor.begin(), client[c_id].neighbor.end(), cmp_server_by_band_limit);
	}
}

void regenerate_fare_sequence(int s_id) //重新生成计费序列，但不再维护较短长度
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
//迁移时，预分配时确定位置的片，该不该动？
//void redistribution(vector<int>& s_id_vec, int times, int X_t_c_p[maxt][maxm][maxp])
//{
//	//srand(times * 223); //这样不是最好的，但固定随机数种子可以用于结果对照
//	//cout << "begin a redistribution\n";
//	srand(unsigned(time(0)));
//	random_shuffle(s_id_vec.begin(), s_id_vec.end());
//	for (int s1_idx = 0; s1_idx < N ; s1_idx++)
//	{
//		int s1 = s_id_vec[s1_idx];
//		//尝试在不突破s1成本的情况下，把s1的所有邻居分给其他服务器的片，都移给s1【不行！】
//		//换思路：s1是迁出服务器，把它的片尽量移走，被迁入服务器则不管，实时维护被迁入服务器的p95和剩余免费空间
//		int c_len = server[s1].degree();
//		for (int c_idx = 0; c_idx < c_len; c_idx++)
//		{
//			int c_id = server[s1].neighbor[c_idx];
//			for (int t = 1; t <= T; t++)
//			{
//				int pNum = PieceNumber_t[t];
//				for (int p_id = 1; p_id <= pNum; p_id++)
//				{
//					if (pre_allocate_piece_t_c_p[t][c_id][p_id])continue; //预分配时确定归属的小片，不可移动
//					if (X_t_c_p[t][c_id][p_id] == s1) //找到了一个分给s1的小片，看看能不能分给别人
//					{
//
//					}
//					//int s2 = X_t_c_p[t][c_id][p_id];
//					//if (s2 != 0&&s2!=s1) //看看能不能把这个片移到s1
//					//{
//					//	LL a_piece = D0_t_c_p[t][c_id][p_id]; //从原始数据中，查询小片的大小
//					//	LL free_capacity = server[s1].free_space(t) - server[s1].used_band[t];
//					//	if (free_capacity < a_piece)continue; //小片放不下
//					//	
//					//	
//					//	//cout << "move piece,t=" << t << ", c_id=" << c_id << ",p_id=" << p_id << ",size=" << a_piece << ", from server s2=" << s2 << " , to s1=" << s1 << "\n";
//					//	//把这个小片从s2中取出，放入s1
//					//	//不再需要D_client_moment，D_t_c_p等
//					//	server[s1].used_band[t] += a_piece;
//					//	server[s2].used_band[t] -= a_piece;
//					//	X_t_c_p[t][c_id][p_id] = s1;
//					//	//keep_fare_sequence(s1, t);
//					//	regenerate_fare_sequence(s1);
//					//	regenerate_fare_sequence(s2);
//					//}
//				}
//			}
//		}
//		for (int si = 1; si <= N; si++)
//		{
//			regenerate_fare_sequence(si);
//			//cout << "now server " << si << ", p95_band=" << server[si].p95_band() << "\n";
//		}
//		
//	}
//	//cout << "end a redistribution\n";
//}
void redistribution2(vector<int>& s_id_vec, int times, int X_t_c_p[maxt][maxm][maxp])
{
	//srand(times * 7); //这样不是最好的，但固定随机数种子可以用于结果对照
	//srand(times);
	srand(unsigned(time(0)));
	random_shuffle(s_id_vec.begin(), s_id_vec.end());
	for (int s1_idx = 0; s1_idx < N - 1; s1_idx++) //如果可以，把s2的流量移动到s1
	{
		int s1 = s_id_vec[s1_idx]; //确定s1为迁入
		for (int s2_idx = s1_idx + 1; s2_idx < N; s2_idx++)
		{
			int s2 = s_id_vec[s2_idx]; //确定s2为迁出
			for (int c_id = 1; c_id <= M; c_id++) //枚举所有客户，找到s1和s2的公共邻居
			{
				if (adjacency_c_s[c_id][s1] && adjacency_c_s[c_id][s2]) //只有c_id同时连接s1和s2才可以改流量分配
				{
					//把c_id给s2的片移动给s1
					for (int t = 1; t <= T; t++)
					{
						int pNum = PieceNumber_t[t];
						for (int p_id = 1; p_id <= pNum; p_id++)
						{
							if (pre_allocate_piece_t_c_p[t][c_id][p_id])continue; //预分配确定的小片不可动
							if (X_t_c_p[t][c_id][p_id] == s2) //找到了分给s2的小片，看看能不能分给s1
							{
								LL a_piece = D0_t_c_p[t][c_id][p_id];
								LL free_capacity = server[s1].free_space(t) - server[s1].used_band[t];
								if (free_capacity >= a_piece)
								{
									server[s1].used_band[t] += a_piece;
									server[s2].used_band[t] -= a_piece;
									X_t_c_p[t][c_id][p_id] = s1;
								}
							}
						}
					}
				}
			}
			regenerate_fare_sequence(s2);
		}
		regenerate_fare_sequence(s1);
	}

}
void loop_redistribution(int X_t_c_p[maxt][maxm][maxp])
{
	int time_limit_seconds = 135;
	int loop_times_limit = 100;
	vector<int>s_id_vec;
	for (int i = 1; i <= N; i++)
	{
		s_id_vec.push_back(i);
	}
	unsigned int times = 0;
	while (1)
	{
		times++;
		redistribution2(s_id_vec, times, X_t_c_p);
		clock_t end_time = clock(); //程序到现在为止的运行时间

		if ((end_time - start_time) / CLOCKS_PER_SEC > time_limit_seconds)break;
		if (times >= loop_times_limit)break;
	}
}
void output_all_server_cost()
{
	LD total_cost = 0;
	cout << "每个服务器的计费成本：\n";
	for (int s_id = 1; s_id <= N; s_id++)
	{
		if (server[s_id].fare_sequence.size() == 0)
		{
			cout << server[s_id].name << ":" << 0 << "\n";
		}
		else if (server[s_id].p95_band() <= V)
		{
			cout << server[s_id].name << ": V=" << V << "\n";
			total_cost += V;
		}
		else
		{
			LD p95_band = server[s_id].p95_band();
			LD cost = (p95_band - V) * (p95_band - V) / server[s_id].band_limit + p95_band;
			cout << server[s_id].name << ": " << cost << "\n";
			total_cost += cost;
		}
	}
	cout << "总成本：" << total_cost << "\n";
}
int main()
{
	srand(unsigned(time(0)));
	start_time = clock();
	std::ios_base::sync_with_stdio(false);
	cin.tie(NULL); cout.tie(NULL);
	//freopen("C:/Users/Touchlion/Desktop/fighting/ACM/2022hwsoft/复赛分析工具/v0/solution.txt", "w", stdout);
	//将cout重定向到solution.txt中，【提交时】一定要有这句
	freopen("/output/solution.txt", "w", stdout);
	//freopen("solution1.txt", "w", stdout);
	input_PATH = "/data/";

	read_and_copy(); //读入全部

	percent95 = ceil(T * 0.95);
	percent5 = T - percent95;
	percent_90 = ceil(T * 0.9);
	percent_10 = T - percent_90 - 1;

	pre_sort_client_neighbor(); //将所有client的neighbor队列排序

	pre_allocate_all_free2(); //预分配，【注意选择的版本】

	first_schedule2();

	//output_all_server_cost();   /////【调试时】，输出当前方案的成本信息


	output_ans(X_t_c_p);
}
