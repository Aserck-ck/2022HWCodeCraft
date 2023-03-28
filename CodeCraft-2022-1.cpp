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
clock_t start_time; //����ʼ����ʱ�䣬���ڼ�ʱ
const int maxt = 9000, maxm = 40, maxn = 140, maxp = 105; //T ,M ,N ,Pt�����ݷ�Χ
LL Q = 0; //QoSԼ��ֵ
LL T = 0; //ʱ����
LL M = 0; //�ͻ�����
LL N = 0; //����������
LL V = 0; //base_cost
LD A = 0; //center_cost
LL percent95; //T*0.95 ����ȡ��
LL percent5;  //T-percent95
//�������±�ȫ����1��ʼ��һ����˵"СƬ"��ָ����Ŀ��˵��"��"
//����һ��СƬ�ķ���ʱ��Ҫͬʱ����D_t_c_p��D_client_moment��server.used_band��X_t_c_p
LL D_t_c_p[maxt][maxm][maxp]; //D[t][c_id][k]��ʾc_id��tʱ�̵ĵ�k��СƬ(��)��ʣ��������������СƬ���ɷָҪô��ȡ��Ҫôһ��ȡ��
string PieceName_t_p[maxt][maxp]; //PieceName_t_p[t][k]��ʾtʱ�̵ĵ�k��СƬ(��)������
int PieceNumber_t[maxt]; //PieceNumber_t[t]��ʾtʱ���ж�����СƬ
LL D_client_moment[maxm][maxt]; //D[c_id][t]==��c_id�ſͻ��ڵ�tʱ�̵�ʣ������������СƬ֮�ͣ������������һ��
LL D0_t_c_p[maxt][maxm][maxp]; //����ԭʼ���ݣ���Զ����
LL D0_client_moment[maxm][maxt]; //ͬ
//
LL QoS_client_server[maxm][maxn]; //Qos[i][j]==i�ſͻ���j�ŷ�����֮��QoS
int X_t_c_p[maxt][maxm][maxp]; //X_t_c_p[t][c_id][k]��¼tʱ��c_id�ĵ�kСƬ�ָ����ĸ�������s_id
//
int adjacency_c_s[maxm][maxn]; //�ڽӾ���adjacency_c_s[c_id][s_id]Ϊ1��ʾc_id�ſͻ���s_id�ŷ������б�
int pre_allocate_server_moment[maxn][maxt]; //�����ڵ���1����ʾs_id��tʱ�̷���Ԥ����
//֮ǰ��ΪԤ����ʱȷ��������СƬ���ܶ������ڲ������
//int pre_allocate_piece_t_c_p[maxt][maxm][maxp];  //Ϊ1��ʾ tʱ�� c_id�ſͻ� �ĵ�p_id��СƬ��Ԥ����ʱȷ��������СƬ



bool cmp_bigger(LL a, LL b)
{
	return a > b;
}
//bool cmp_bigger(LD a, LD b)
//{
//	return a > b;
//}
struct Node_fare //��fare_sequence�У�������¼������������˳���¼���ĸ�ʱ��
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

////���������Ľڵ������йص�ȫ������
LL MS_t_s_p[maxt][maxn][maxp]; //MS_t_s_p[t][s_id][p_id]==tʱ�̣�������s_id���յ��ĵ�p_id��СƬ�� Max Size
LL MS_t_s[maxt][maxn]; //MS_t_s[t][s_id]==���MS_t_s_p[t][s_id][..]
LL MS_t[maxt]; //MS_t[t]==���MS_t_s_p[t][..][..]��Ҳ�������Ľڵ���tʱ�̵�����

struct Center
{
	vector<Node_fare>fare_sequence; //ʵʱά�����Ľڵ�ļƷ�����
	LL p95_band() //����Ŀǰ�����м�¼��95����
	{
		int len = fare_sequence.size(); //���Ҫ�Ż�ʱ�䣬���Կ��ǰ�����ά��fare_sequence ��length
		if (len <= percent5)return 0;
		return fare_sequence[percent5].used;
	}
	int p95_tim() //����Ŀǰ�����м�¼95��������ڵ�ʱ��
	{
		int len = fare_sequence.size();
		if (len <= percent5)return 0;
		return fare_sequence[percent5].tim;
	}
	LL p96_band() //����Ŀǰ�����м�¼�ı�p95��һλ�Ĵ���Ҳ������Сһ����ѵ�����
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
	LL p100_band() //����Ŀǰ�����м�¼�ģ���ǰ�����������������ֵ��Ҳ�������һ����ѵ�����
	{
		if (fare_sequence.size() > 0)return fare_sequence[0].used;
		return 0;
	}
	int p100_tim()
	{
		if (fare_sequence.size() > 0)return fare_sequence[0].tim;
		return 0;
	}
	void keep_center_fare_sequence(int t) //���Ե�ڵ��ά�������߼���ͬ
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
	bool this_server_is_used = false;  //��������ȫ�շ���������ʹ�÷�������false��ʾȫ�գ�true��ʾ��ʹ��
	LL band_limit = 0; //��������
	LL used_band[maxt]; //��¼��̨������ÿ��ʱ�̵����ô���ͣ���ֵ���ܳ���band_limit
	vector<int>neighbor; //��¼���������Ŀͻ�id
	int degree() { return neighbor.size(); }
	vector<Node_fare>fare_sequence; //ʵʱά���ļƷ����У�������һ��ʱ��t���Ͱ����ʱ�̵���������ƷѶ��У�����������
	LL p95_band() //����Ŀǰ�����м�¼��95����
	{
		int len = fare_sequence.size();
		if (len <= percent5)return 0;
		return fare_sequence[percent5].used;
	}
	int p95_tim() //����Ŀǰ�����м�¼95��������ڵ�ʱ��
	{
		int len = fare_sequence.size();
		if (len <= percent5)return 0;
		return fare_sequence[percent5].tim;
	}
	LL p96_band() //����Ŀǰ�����м�¼�ı�p95��һλ�Ĵ���Ҳ������Сһ����ѵ�����
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
	LL p100_band() //����Ŀǰ�����м�¼�ģ���ǰ�����������������ֵ��Ҳ�������һ����ѵ�����
	{
		if (this_server_is_used)return fare_sequence[0].used;
		return 0;
	}
	int p100_tim()
	{
		if (this_server_is_used)return fare_sequence[0].tim;
		return 0;
	}

	//��ͬ�Ľ׶Σ���Ҫ���ø��Ʒ��йصĺ���
	LL free_space_for_server(int t) //ע�⣬����ֵ����ѿռ��С����Ҫ��ȥ���õ�
	{
		if (this_server_is_used == false)return 0; //ȫ�շ�����û����ѿռ䣬ʹ����Ҫ����V
		//���÷�������������V
		int p95_t = p95_tim();
		LL p95_b = p95_band();
		if (p95_t != t)return max(p95_b, V);
		//�Ʒ������м�¼��95�������ǵ�ǰʱ�̣�����Ѿ�����V��������ѿռ�
		if (p95_b < V)return V;
		return p95_b; //�����޸ģ������ǲ���Ҫ�ĳ�return p95_b
	}

}server[maxn];
void keep_fare_sequence(int s_id, int t) //��used_band[t]ά���Ʒ�����
{
	LL used = server[s_id].used_band[t];
	LL p95_band = server[s_id].p95_band();
	if (used>p95_band) //ֻ�е�used����ԭ����95��������Ҫ���¼Ʒ�����
	{
		vector<Node_fare>&fs = server[s_id].fare_sequence;
		int len = fs.size();
		for (int i = 0; i < len; i++) //�����Ҵ�ʱ���Ƿ��Ѿ��ڼƷѶ�����
		{
			if (fs[i].tim == t) //����Ѿ��ڶ����У������ʹ�������������򼴿�
			{
				fs[i].used = used;
				//�������Ҫ�Ż��ٶȣ����Կ����ڴ˴��ĳ�O(n)�Ĳ���
				sort(fs.begin(), fs.end(), cmp_Node_fare);
				return;
			}
		}
		//�������¼�����е�ʱ�̣�δ�����£�Ҳ������֮ǰ�������ڶ������Ų��Ϻ�
		Node_fare u(used, t);
		fs.push_back(u);
		sort(fs.begin(), fs.end(), cmp_Node_fare);
		while (fs.size() > percent5 + 1)
		{
			fs.pop_back();
		}

	}
}
LD how_much_this_server_cost_now(int s_id) //���ش˷�������ǰ�Ѿ���Ҫ������Ǯ
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
	if (constraint_length)  //���Ƴ���
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
	vector<int>neighbor; //��¼���������ķ�����id
	int degree() { return neighbor.size(); }
}client[maxm];

//һ��СƬ���������ԣ�������ʱ�̣��ͻ���Ƭ��
struct Node_piece
{
	int tim, c_id, p_id;
	Node_piece(int t, int c, int p)
	{
		tim = t; c_id = c; p_id = p;
	}
};

map<string, int>mp_client_to_id; //�ӿͻ����ֵ��±�
map<string, int>mp_server_to_id; //�ӷ��������ֵ��±�

void assign_a_piece(int& t, int& c_id, int& p_id, int& s_id)//��tʱ�̿ͻ�c_id�ĵ�p_id��СƬ�����������s_id
{
	LL& a_piece = D_t_c_p[t][c_id][p_id];
	if (a_piece > MS_t_s_p[t][s_id][p_id]) //���´������Ľڵ���������������
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

//�������Ҫ�Ż��ٶȣ����Ÿĳɴ�����
bool cmp_Node_piece_for_first_schedule2(Node_piece a, Node_piece b)
{
	//Ƭ�����ʤ��ƬС����
	return D_t_c_p[a.tim][a.c_id][a.p_id] > D_t_c_p[b.tim][b.c_id][b.p_id];
}
bool cmp_Node_piece_for_pre_allocate(Node_piece a, Node_piece b)
{
	//Ƭ�����ʤ��ƬС����
	return D_t_c_p[a.tim][a.c_id][a.p_id] > D_t_c_p[b.tim][b.c_id][b.p_id];
}


//Ԥ���䣬�������Ŀ���˳��
bool cmp_server_for_pre_allocate(int i, int j) //��̫���ˡ�Ҳ������������������˳�򻹺�
{
	//return server[i].degree() > server[j].degree();
	return server[i].band_limit > server[j].band_limit;
}
//Ԥ���䵱ǰ���ǵ����ĸ�������
int now_s_id_in_pre_allocate = 0;
//Ԥ���䵱ǰ���ǵ����ĸ�ʱ��
int now_t_in_pre_allocate = 0;
//////��һ��Ԥ���䣬����ϰ��ʹ��(СƬ��С)��ֵ����ʱ��ѡ��ʱ�����С��������׼
struct Node_pre
{
	int tim;
	LL demand_sum = 0;  //ʱ��t���ھ�����֮��
	LD average = 0;     //��СƬ��С����ֵ
};
bool cmp_Node_pre(Node_pre a, Node_pre b)
{
	LL band_limit = server[now_s_id_in_pre_allocate].band_limit;
	//������ʱ�̵���������������������������ʱ���Ƚ������С�Ѿ�������
	LD k = 1; //0.95�Ѿ�����
	if (a.demand_sum >= band_limit*k&&b.demand_sum >= band_limit*k)
	{//��ֵ��ĺ�һЩ����Ϊʲô���������������
		return a.average > b.average;
	}
	return a.demand_sum > b.demand_sum;
}
void pre_allocate_all_free() //Ԥ�����һ�汾
{
	vector<int>s_id_vec;
	for (int i = 1; i <= N; i++)
	{
		s_id_vec.push_back(i);
	}
	//���������򣬾���˭�Ƚ���Ԥ����
	sort(s_id_vec.begin(), s_id_vec.end(), cmp_server_for_pre_allocate);

	//srand(unsigned(time(0)));
	//random_shuffle(s_id_vec.begin(), s_id_vec.end());

	for (int s_idx = 0; s_idx < N; s_idx++)  //���źõ�˳���Ƿ����������޸ģ�Ҳ��������Ҹ���
	{
		int s_id = s_id_vec[s_idx];   //ȡ��������id
		now_s_id_in_pre_allocate = s_id; //��ȫ�ֱ�������ǰ���ǵ��ķ�������s_id
		if (server[s_id].degree() == 0)continue; //û�г��ȵķ�������û���κ����ü�ֵ
		if (server[s_id].degree() <= 3)continue;//////////////////////////////////////////////
		int c_len = server[s_id].neighbor.size();
		//ͬһ����������percent5��Ԥ���䶼�����ڲ�ͬʱ��
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
			//ͳ��tʱ���ж��ٸ�СƬ
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
			//��СƬ��С��ֵ
			nod.average = nod.demand_sum*1.0 / p_num_in_t;
			sum_per_day.push_back(nod);
		}
		//��demand_sum�Ѿ�����band_limitʱ���Ƚ�demand_sum�������壬Ӧ�ñȽ�ʲô��
		sort(sum_per_day.begin(), sum_per_day.end(), cmp_Node_pre);
		for (int t_idx = 0; t_idx < percent5; t_idx++) //ǰpercent5�Σ�Ҫô�������أ�Ҫô�������пͻ����ҷ������ھ��������ʱ��
		{
			//�����޸ģ��˴�����ͨ�����Ƭ�Ĵ�С����Ϣ���ٴξ����Ƿ�Ҫ��t_idx����Ԥ����
			int t = sum_per_day[t_idx].tim;
			now_t_in_pre_allocate = t; //��ȫ�ֱ���
			pre_allocate_server_moment[s_id][t] = 1; //��¼s_id��tʱ�̷���Ԥ����
			vector<Node_piece>piece_vec;
			//��tʱ�̵������ھ�СƬ�����������
			int pNum = PieceNumber_t[t];
			for (int c_idx = 0; c_idx < c_len; c_idx++)
			{
				int c_id = server[s_id].neighbor[c_idx];
				for (int p_id = 1; p_id <= pNum; p_id++)
				{
					if (D_t_c_p[t][c_id][p_id]) //ֻ�зǿ�СƬ����Ҫ����
					{
						Node_piece pie(t, c_id, p_id);
						piece_vec.push_back(pie);
					}
				}
			}
			//��Ƭ��ǰ���������ա���ʵ�ǲ���Ҫ����һ�£�������band���޵�����£���Ҫ��������������СƬ��
			sort(piece_vec.begin(), piece_vec.end(), cmp_Node_piece_for_pre_allocate);
			int neighbor_piece_num = piece_vec.size();
			for (int node_idx = 0; node_idx < neighbor_piece_num; node_idx++)
			{
				Node_piece piece = piece_vec[node_idx];
				int c_id = piece.c_id;
				int p_id = piece.p_id;
				LL capacity = server[s_id].band_limit - server[s_id].used_band[t];
				if (capacity <= 0)break; //���������أ������������Ӧ�ú��ѳ��֣�ָ���÷�������Ϊ�����ܳ�Խ����
				if (capacity < D_t_c_p[t][c_id][p_id])continue; //��һƬ�Ų���
				assign_a_piece(t, c_id, p_id, s_id);
			}
			//����ʱ�̿�����ɣ�ά���ƷѶ���
			keep_fare_sequence(s_id, t);
		}
	}
}


//////�ڶ���Ԥ���䣬����ϰ��ʹ��ʵ����װ���������СƬ�ľ�ֵ����ʱ��ѡ��ʱ����۽ϴ���ֻ�ȵ�һ������2w�����ң���47w��45w��
struct Node_fake_t_result
{
	int tim;  //����ģ���ʱ��
	LL demand_sum = 0;    //��ʱ���ھ�������
	LL used_band = 0;   //ʵ��װ�������
	int put_piece_num = 0;    //װ���Ƭ��
	//LL max_piece_size = 0;    //װ������Ƭ��size
	//int threshold_exceeded_num = 0;  //װ���Ƭ�У���С������ֵ��Ƭ��
	Node_fake_t_result(int Tim, LL D_sum, LL Used_band, int Put_num)
	{
		tim = Tim;
		demand_sum = D_sum; used_band = Used_band; put_piece_num = Put_num;
	}
};
//��������ʱ�̣�˭���ʺϷ���Ԥ����
bool cmp_Node_fake_t_result(Node_fake_t_result a, Node_fake_t_result b)
{
	//�����޸ģ���û�Թ�ֱ�ӱȽ�ʵ��װ�������Һ�����ֵ��ȵ����
	LL band_limit = server[now_s_id_in_pre_allocate].band_limit;
	if (a.demand_sum >= band_limit&&b.demand_sum >= band_limit)
	{//������ʱ�̶��ǲ�����ȫװ���ʱ�̣���Ƚ�װ��ľ�ֵ
		return a.used_band*1.0 / a.put_piece_num > b.used_band*1.0 / b.put_piece_num;
	}
	//��������ʱ���У�������һ��ʱ�̿�����ȫװ��
	return a.used_band > b.used_band;
}
void pre_allocate_all_free2() //Ԥ����ڶ��汾������ÿ��ʱ��ģ���������䣬�ٸ���ÿ��ʱ�̵�װ��Ч��������Ҫ���ļ���ʱ����������Ԥ����
{
	vector<int>s_id_vec;
	for (int i = 1; i <= N; i++)
	{
		s_id_vec.push_back(i);
	}
	//���������򣬾���˭�Ƚ���Ԥ����
	sort(s_id_vec.begin(), s_id_vec.end(), cmp_server_for_pre_allocate);

	for (int s_idx = 0; s_idx < N; s_idx++)  //���źõ�˳���Ƿ�����
	{
		int s_id = s_id_vec[s_idx];   //ȡ��������id
		now_s_id_in_pre_allocate = s_id; //��ȫ�ֱ�������ǰ���ǵ��ķ�������s_id
		int c_len = server[s_id].degree();
		if (c_len == 0)continue; //û�г��ȵķ�������û���κ����ü�ֵ

		vector<Node_fake_t_result>result_vec; //�����洢ÿ��ʱ��ģ��Ԥ����Ľ������������Ϣ��ʧ��
		//ģ��Ԥ���䣬�ڲ�ȷ���˷�����Ӧ������Щʱ�̷���Ԥ����ʱ������ȫ��ʱ����һ�飬������ѡ��Ч����õ�percent5��ʱ�̣���������Ԥ����
		for (int fake_t = 1; fake_t <= T; fake_t++)
		{
			//��fake_t���ʱ����ģ��һ��Ԥ����
			vector<Node_piece>fake_piece_vec;
			//��fake_tʱ���У��ھӵ�СƬ�����ɶ���fake_piece_vec
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
			//���Ƭ��ǰ������Ԥ����Ĺ���һ��
			sort(fake_piece_vec.begin(), fake_piece_vec.end(), cmp_Node_piece_for_pre_allocate);
			int piece_num = fake_piece_vec.size();
			//��¼װ���Ƭ����ʱ������������ʵ��װ����
			int put_piece_num = 0; LL demand_sum = 0; LL fake_used_band_t = 0;
			for (int node_idx = 0; node_idx < piece_num; node_idx++)
			{
				//����Ԥ����һ�����Ӵ�С����СƬ
				Node_piece piece = fake_piece_vec[node_idx];
				int c_id = piece.c_id;
				int p_id = piece.p_id;
				LL piece_size = D_t_c_p[fake_t][c_id][p_id];
				demand_sum += piece_size;                //��¼����֮��
				//ע�⣬ģ�����ʱ���ܸĶ��κζ���������ר��Ϊ�������׼����fake_server��used_band[ ]
				LL capacity = server[s_id].band_limit - fake_used_band_t;

				if (capacity < piece_size)continue; //�Ų���

				//ģ������СƬ��ͬ����¼һЩЧ������ֵ
				put_piece_num++;                         //�ܹ�װ�˼���СƬ������
				fake_used_band_t += piece_size;    //��ʱ�������ۼ�

			}///fake_tʱ�̵�СƬ���б����������
			//�����¼����Ϣ����ʱ�̣����������
			////����Node_fake_t_result(int Tim, LL D_sum, LL Used_band, int Put_num)
			Node_fake_t_result result(fake_t, demand_sum, fake_used_band_t, put_piece_num);
			result_vec.push_back(result);
		}
		//���ϣ�����ʱ���ж�������һ��ģ�����
		sort(result_vec.begin(), result_vec.end(), cmp_Node_fake_t_result);
		//�ź���ǰ��percent5����¼��ʱ�̣�����Ӧ�þ���Ҫ����Ԥ�����ʱ�̣����������Ԥ����
		for (int t_idx = 0; t_idx < percent5; t_idx++) //ǰpercent5�Σ�Ҫô�������أ�Ҫô�������пͻ����ҷ������ھ��������ʱ��
		{
			int t = result_vec[t_idx].tim;
			now_t_in_pre_allocate = t; //��ȫ�ֱ���
			pre_allocate_server_moment[s_id][t] = 1; //��¼s_id��tʱ�̷���Ԥ����
			vector<Node_piece>piece_vec;
			//��tʱ�̵������ھ�СƬ�����������
			int pNum = PieceNumber_t[t];
			for (int c_idx = 0; c_idx < c_len; c_idx++)
			{
				int c_id = server[s_id].neighbor[c_idx];
				for (int p_id = 1; p_id <= pNum; p_id++)
				{
					if (D_t_c_p[t][c_id][p_id]) //ֻ�зǿ�СƬ����Ҫ����
					{
						Node_piece pie(t, c_id, p_id);
						piece_vec.push_back(pie);
					}
				}
			}
			//��Ƭ��ǰ���������ա���ʵ�ǲ���Ҫ����һ�£�������band���޵�����£���Ҫ��������������СƬ��
			sort(piece_vec.begin(), piece_vec.end(), cmp_Node_piece_for_pre_allocate);
			int neighbor_piece_num = piece_vec.size();
			for (int node_idx = 0; node_idx < neighbor_piece_num; node_idx++)
			{
				Node_piece piece = piece_vec[node_idx];
				int c_id = piece.c_id;
				int p_id = piece.p_id;
				LL capacity = server[s_id].band_limit - server[s_id].used_band[t];
				if (capacity <= 0)break; //���������أ������������Ӧ�ú��ѳ��֣�ָ���÷�������Ϊ�����ܳ�Խ����
				if (capacity < D_t_c_p[t][c_id][p_id])continue; //��һƬ�Ų���
				assign_a_piece(t, c_id, p_id, s_id);
			}
			//����ʱ�̿�����ɣ�ά���ƷѶ���
			keep_fare_sequence(s_id, t);
		}
	}
}

struct Node_flow_info
{
	int p_id; //��������
	LL sum = 0; //������������
};
bool cmp_Node_flow_info(Node_flow_info a, Node_flow_info b)
{
	return a.sum > b.sum;
}
/////�������Ľڵ��Ԥ����
void pre_allocate_all_free3()
{
	/*��������ѡ*/
	vector<int>s_id_vec;
	for (int i = 1; i <= N; i++)
	{
		s_id_vec.push_back(i);
	}
	//���������򣬾���˭�Ƚ���Ԥ����
	sort(s_id_vec.begin(), s_id_vec.end(), cmp_server_for_pre_allocate);

	for (int s_idx = 0; s_idx < N; s_idx++)  //���źõ�˳���Ƿ����������޸ģ�Ҳ��������Ҹ���
	{
		int s_id = s_id_vec[s_idx];   //ȡ��������id
		now_s_id_in_pre_allocate = s_id; //��ȫ�ֱ�������ǰ���ǵ��ķ�������s_id
		if (server[s_id].degree() == 0)continue; //û�г��ȵķ�������û���κ����ü�ֵ
		int c_len = server[s_id].neighbor.size();
		/*ʱ��ѡ��*/
		//ͬһ����������percent5��Ԥ���䶼�����ڲ�ͬʱ��
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
			//ͳ��tʱ���ж��ٸ�СƬ
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
			//��СƬ��С��ֵ
			nod.average = nod.demand_sum*1.0 / p_num_in_t;
			sum_per_day.push_back(nod);
		}
		//��demand_sum�Ѿ�����band_limitʱ���Ƚ�demand_sum�������壬Ӧ�ñȽ�ʲô��
		sort(sum_per_day.begin(), sum_per_day.end(), cmp_Node_pre);
		/*���շ�ʽ*/
		for (int t_idx = 0; t_idx < percent5; t_idx++) //ǰpercent5�Σ�Ҫô�������أ�Ҫô�������пͻ����ҷ������ھ��������ʱ��
		{
			//�����޸ģ��˴�����ͨ�����Ƭ�Ĵ�С����Ϣ���ٴξ����Ƿ�Ҫ��t_idx����Ԥ����
			int t = sum_per_day[t_idx].tim;
			now_t_in_pre_allocate = t; //��ȫ�ֱ���
			pre_allocate_server_moment[s_id][t] = 1; //��¼s_id��tʱ�̷���Ԥ����
			int pNum = PieceNumber_t[t];
			vector<Node_flow_info>flow_info_vec;     //ͳ��ÿ�������ܺ�
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
			//��ͬp_id����СƬ�ܺʹӴ�С����
			sort(flow_info_vec.begin(), flow_info_vec.end(), cmp_Node_flow_info);
			for (int f_idx = 0; f_idx < pNum; f_idx++)
			{
				LL capacity = server[s_id].band_limit - server[s_id].used_band[t];
				if (capacity <= 0)break; //���������أ�����������ѳ��֣�
				int p_id = flow_info_vec[f_idx].p_id;
				LL  p_sum = flow_info_vec[f_idx].sum;
				if (capacity < p_sum)continue; //������������ȫ���ɣ���ѡ�����գ�������һ��
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




//�����˷�����ȫ�յ�������Լ���ѻ����ڵ��������δ�������Ľڵ�
LD delta_Cost_if_add_piece_for_server(int s_id, int t, LL a_piece)//tʱ�������a_piece���������s_id�������µĳɱ���������ͬʱ�������μƷѺ�����
{
	if (server[s_id].band_limit - server[s_id].used_band[t] < a_piece)return 1e8; //�����Ų��£����ؼ���ֵ�������Ŵ˷�����������
	if (pre_allocate_server_moment[s_id][t])return 0; //���������ѻ��ᣬ��û���κγɱ�����
	LD p95_band = server[s_id].p95_band();      //ԭ����p95
	LD band_limit = server[s_id].band_limit;    //��������������
	LD used_band_t = server[s_id].used_band[t]; //tʱ��ԭ��������
	LD new_used_band_t = used_band_t + a_piece; //�������Ƭ���tʱ������
	if (new_used_band_t <= p95_band)return 0; //tʱ�̼������Ƭ��û��ͻ��p95���ɱ��ض�������
	//������ζ��new_used_band_tͻ����p95_band����Ҳ�����������µ�p95��Ҫ��p96��һ�£�����˭����p95
	LD old_cost = 0; LD new_cost = 0;
	if (server[s_id].this_server_is_used == false)old_cost = 0;
	else if (p95_band <= V)old_cost = V;
	else old_cost = (p95_band - V)*(p95_band - V)*0.95 / band_limit + p95_band;   //�����ڶ����band_limit�˹���0.95����Ҫ�ڷ���ͬʱ��0.95Լ����������ʵ�ڻ�������£�used���ǲ���ȷ��
	LD p96_band = server[s_id].p96_band();
	LD new_p95_band = min(new_used_band_t, p96_band);
	if (new_p95_band <= V)new_cost = V;
	else new_cost = (new_p95_band - V)*(new_p95_band - V)*0.95 / band_limit + new_p95_band;
	LD ret = new_cost - old_cost;
	return ret;
}

//ֻ�������Ľڵ�
LD delta_Cost_if_add_piece_for_Center(int s_id, int t, int p_id, LL a_piece) //tʱ�������a_piece���������s_id�������µ����ĳɱ���������
{
	LL pre_max_size = MS_t_s_p[t][s_id][p_id];
	if ( pre_max_size>= a_piece)return 0;  //����������ֵ����������˿��������
	LL delta = a_piece - pre_max_size;  //�����µ����Ľڵ�tʱ�̴�������
	LL new_MS_t = MS_t[t] + delta;
	LL center_p95_band = center.p95_band();
	int center_p95_tim = center.p95_tim();
	if (new_MS_t <= center_p95_band)   //Ҳ�����ӵ������Ľڵ����ѿռ��У�����Ӧ������ȫû����
	{
		return 1.0 / (center_p95_band - MS_t[t]);
	}
	LD pre_center_cost = A*center_p95_band;
	LD new_center_cost = A*new_MS_t;
	return new_center_cost - pre_center_cost;
}


void first_schedule()
{
	//����for�Կ�����Ҳ�ɻ�ϣ�����һ��ʱ��һ��ʱ�̵ؿ��ǣ���ʱ���ڵ�c_id��p_id��Ͽ���
	//��Ȼ�������ǰ��Ļ�Ӱ�����ģ���Ӧ����Ҫ�̶�����ÿ�����±�����ѡһ����������ȷ��
	for (int t = 1; t <= T; t++)
	{
		int pNum = PieceNumber_t[t];
		//�ڶ���Ӧ�����������࣬���ǿͻ����������ǻ�ϣ�
		for (int p_id = 1; p_id <= pNum; p_id++)  //pNum<=100
		{
			for (int c_id = 1; c_id <= M; c_id++) //M<=35
			{
				LL& a_piece = D_t_c_p[t][c_id][p_id];
				if (a_piece == 0)continue;
				//�������СƬ�ָ�˭
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


void first_schedule2()  //ȫ������ĳ��η���汾
{
	regenerate_center_fare_sequence(true); //���Ԥ������������Ľڵ�ĵ�ǰ�Ʒ����У��ڳ��η���ʱ��������

	vector<Node_piece>piece_vec;
	//������СƬ����һ������
	for (int t = 1; t <= T; t++)
	{
		int pNum = PieceNumber_t[t];
		for (int c_id = 1; c_id <= M; c_id++)
		{
			for (int p_id = 1; p_id <= pNum; p_id++)
			{
				if (D_t_c_p[t][c_id][p_id]) //ֻ�зǿ�СƬ����Ҫ����
				{
					Node_piece pie(t, c_id, p_id);
					piece_vec.push_back(pie);
				}
			}
		}
	}
	//����СƬ���򣨾�̬˳�򣩴����ǰ
	sort(piece_vec.begin(), piece_vec.end(), cmp_Node_piece_for_first_schedule2);
	int total_piece_num = piece_vec.size();
	//Ϊ����СƬ�ҵ�����������
	for (int node_idx = 0; node_idx < total_piece_num; node_idx++)
	{
		Node_piece piece = piece_vec[node_idx];
		int c_id = piece.c_id;
		int t = piece.tim;
		int p_id = piece.p_id;
		int s_len = client[c_id].degree();
		LL& a_piece = D_t_c_p[t][c_id][p_id];
		/*
		//��Ѱ����ѿռ䣬���ʹ�á���ѿռ価��������á������ѡ������ʵ���ѷ�������
		//������䷨��ѡ��������ѿռ���з��루Ŀǰ������Ч���Ϻã�
		//��С���䷨��ѡ����С����ѿռ���з���
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
		if (max_free_s_id != 0) //������䷨
		{
			assign_a_piece(t, c_id, p_id, max_free_s_id);
			keep_fare_sequence(max_free_s_id, t);
			continue;
		}
		//���û�ܷŵ���ѿռ��У���ѡȡ����֮��ɱ�������С�ķ���������
		//������ѿռ���õ�����£�Ӧ�÷ŵ��ĸ��������ϣ�
		*/

		LD minEva = 1e7; int selected_s = 0;
		LL minCapacity = 1e7; LL max_free_Capacity = 0;
		for (int s_idx = 0; s_idx < s_len; s_idx++)
		{
			int s_id = client[c_id].neighbor[s_idx];
			LL capacity = server[s_id].band_limit - server[s_id].used_band[t];
			if (capacity < a_piece)continue; //�����Ų��£��˷�����������
			LD server_delta = delta_Cost_if_add_piece_for_server(s_id, t, a_piece);  //������СƬ����s_id�Ĵ��ۣ��ɱ�����������
			LD center_delta = delta_Cost_if_add_piece_for_Center(s_id, t, p_id, a_piece);
			LD k = 1.0; //���k���ܴ�Ҳûʲô�仯��˵��center_delta���ǽӽ�0����Ϊ�󲿷ֵ�СƬ��������ֱ�Ӹı����Ľڵ��95�������Ǽ��Ӱ��
			LD eva = server_delta + k*center_delta;
			LL free_capacity = server[s_id].free_space_for_server(t) - server[s_id].used_band[t];
			if (eva < minEva&&fabs(minEva-eva)>1e-12)  
			{
				max_free_Capacity = free_capacity;
				minEva = eva; selected_s = s_id;
			}
			else if (fabs(minEva - eva) <= 1e-12)  //���eva����������������evaΪ0
			{
				if (free_capacity>max_free_Capacity)  //������䷨
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
	cin.clear(); //����˲����ض��������ļ�
	//cout << "Q=" << Q << ", V=" << V <<", A="<<A<<"\n";
}
bool is_character(char ch)
{
	return ch >= 'A' && ch <= 'Z' || ch >= 'a' && ch <= 'z' || ch >= '0' && ch <= '9';
}
void read_demand() //������ʱ�̿��ܲ��ܱ�֤�Ǵӹŵ��񣬵�Ӧ�ò�Ӱ�죬�Ͼ�����ǰ��������ʱ��˳��
{
	string demand_PATH = input_PATH + "demand.csv";
	freopen(demand_PATH.c_str(), "r", stdin);
	string first_line;
	getline(cin, first_line);
	int len = first_line.length();
	int p = 0; while (first_line[p] != 'd')p++; p++;
	p++;
	//����pָ��"stream_id,"�ĺ�һ���ַ�
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
		//���� (����->�±�) ӳ��
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
			T++; PieceNumber_t[T] = 1; last_moment_name = moment_name; //����һ��ʱ��
		}
		else PieceNumber_t[T]++;
		getline(cin, PieceName_t_p[T][PieceNumber_t[T]], ',');
		for (int i = 1; i < M; i++)
		{
			cin >> D_t_c_p[T][i][PieceNumber_t[T]];
			D_client_moment[i][T] += D_t_c_p[T][i][PieceNumber_t[T]]; //˳���ۼӿͻ���tʱ�̵���������
			cin.get(); //��������
		}
		cin >> D_t_c_p[T][M][PieceNumber_t[T]];
		D_client_moment[M][T] += D_t_c_p[T][M][PieceNumber_t[T]];
		if (cin.peek() != EOF)cin.get();
		if (cin.peek() == '\n')cin.get();
	}
	cin.clear();

	/*
	//////����
	cout << "M=" << M << ",T=" << T << "\n    ";
	for (int c_id = 1; c_id <= M; c_id++)
	{
		cout << "<" << client[c_id].name << ">";
	}
	cout << "\n";
	for (int t = 1; t <= T; t++)
	{
		int p_num = PieceNumber_t[t];
		cout << "ʱ��" << t << "��" << p_num << "��СƬ(��)\n";
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
		//���� (����->�±�) ӳ��
		mp_server_to_id[server[N].name] = N;
		cin >> server[N].band_limit;
		if (cin.peek() != EOF)cin.get();
		if (cin.peek() == '\n')cin.get();
		N++;
	}
	N--;
	cin.clear();

	/*
	/////����
	cout<<"N="<<N<<"\n";
	cout << "site_name,bandwidth\n";
	for (int i = 1; i <= N; i++)
	{
	cout <<"<"<< server[i].name << ">," << server[i].band_limit << "\n";
	}
	*/
}
void read_qos() //����Ӧ���Ѿ�����
{
	string qos_PATH = input_PATH + "qos.csv";
	freopen(qos_PATH.c_str(), "r", stdin);
	string first_line;
	getline(cin, first_line);
	int len = first_line.length();
	int p = 0; while (first_line[p] != ',')p++;
	p++;
	//����pָ��"site_name,"�ĺ�һ���ַ�
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
	//��ʱ��c_cnt==M
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
			cin.get(); //��������
		}
		cin >> QoS_client_server[all_c_id[M]][s_id];  //��ĩ������
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
	///////����
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
	copy_D_to_D0(); //���ݶ���ĳ�ʼ����ֻ������
	make_adjacency_matrix();
	for (int s_id = 1; s_id <= N; s_id++) //��������Ƶ����а취
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
void output_all_server_cost()//�Է�����Ϊ�о�����
{
	percent5 = T - percent95; //�ָ�percent5
	for (int s_id = 1; s_id <= N; s_id++)
	{
		server[s_id].band_limit /= 0.95;   //�ָ�band_limit
	}
	regenerate_center_fare_sequence();
	cout << "���Ľڵ㣺\n";
	LD center_p95_band = center.p95_band();
	int center_p95_tim = center.p95_tim();
	LD center_p96_band = center.p96_band();
	int center_p96_tim = center.p96_tim();
	LD center_p100_band = center.p100_band();
	int center_p100_tim = center.p100_tim();
	LD center_cost = center_p95_band*A;
	int center_len = center.fare_sequence.size();
	cout << "���ķ���" << center_cost << ", �Ʒ����г���" << center_len << ",    p95=(" << center_p95_band << " , " << center_p95_tim << ") ,    p96=(" << center_p96_band << " , " << center_p96_tim << "),    p100=(" << center_p100_band << " , " << center_p100_tim << ")\n";
	cout << "��Ե�ڵ㣺\n";
	for (int s_id = 1; s_id <= N; s_id++)
	{
		regenerate_fare_sequence(s_id);
	}
	LD total_cost = 0;
	cout << "    ��������\t�ھ���\t��������\t���³ɱ�\t�Ʒ����г���\tp95(����,ʱ��)\tp96(����,ʱ��)\tp100(����,ʱ��)\n";
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
		cout << s_id << "��" << server[s_id].name << "\t\t" << server[s_id].degree() << "\t" << server[s_id].band_limit << "\t\t" << this_cost << "\t\t" << sequence_length << "\t\t"
			<< p95_band << "," << p95_tim << "\t\t" << p96_band << "," << p96_tim << "\t\t" << p100_band << "," << p100_tim << "\n";
	}
	cout << "��Ե����ܳɱ���" << total_cost << "\n";
	cout << "�����ܳɱ�=" << total_cost << "+" << center_cost << "=" << total_cost + center_cost << "\n\n";

	cout << "���Ľڵ�Ʒ����У�(��λ��������ʱ��)\n";
	for (int i = 0; i < center_len; i++)
	{
		cout << i + 1 << ": " << center.fare_sequence[i].used << " , " << center.fare_sequence[i].tim << "\n";
	}
	for (int t = 1; t <= T; t++)happen_times[t].tim = t;
	cout << "��Ե�ڵ�Ʒ�����(���� ��λ: ������ʱ��)��\n";
	for (int s_id = 1; s_id <= N; s_id++)
	{
		if (server[s_id].degree() == 0)continue;
		cout << s_id << "��" << server[s_id].name << ": \n";
		int len = server[s_id].fare_sequence.size();
		for (int i = 0; i < len; i++)
		{
			int tim = server[s_id].fare_sequence[i].tim;
			cout << s_id << "��" << server[s_id].name << " " << i + 1 << ": " << server[s_id].fare_sequence[i].used << " , " << tim << "\n";
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
		cout << "��t=" << happen_times[i].tim << "ʱ��,����" << happen_times[i].happen_times << "����ѻ���\n";
	}
}

int main()
{
	srand(unsigned(time(0)));
	start_time = clock();
	std::ios_base::sync_with_stdio(false);
	cin.tie(NULL); cout.tie(NULL);
	//freopen("C:/Users/Touchlion/Desktop/fighting/ACM/2022hwsoft/����/������������/v0/solution.txt", "w", stdout);
	//freopen("C:/Users/Touchlion/Desktop/fighting/ACM/2022hwsoft/CodeCraft-2022/CodeCraft-2022/��������/�������(per5+v1pre�ȴ���3����+v2fir��������kΪ1).txt", "w", stdout);
	//��cout�ض���solution.txt�У����ύʱ��һ��Ҫ�����
	freopen("/output/solution.txt", "w", stdout);
	input_PATH = "/data/";

	read_and_copy(); //����ȫ��

	percent95 = ceil(T * 0.95);
	percent5 = T - percent95;
	percent5 /=2;
	pre_allocate_all_free();

	first_schedule2();

	//output_all_server_cost(); /////������ʱ���������ǰ�����ĳɱ���Ϣ


	output_ans(X_t_c_p);
}
