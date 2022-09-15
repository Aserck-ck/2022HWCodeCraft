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
LL percent95; //T*0.95 ����ȡ��
LL percent5;  //T-percent95
LL percent_90;
LL percent_10;
vector<int>chose_s_id_for_change;
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
int pre_allocate_piece_t_c_p[maxt][maxm][maxp];  //Ϊ1��ʾ tʱ�� c_id�ſͻ� �ĵ�p_id��СƬ��Ԥ����ʱȷ��������СƬ
bool cmp_bigger(LL a, LL b)
{
	return a > b;
}
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
struct Server
{
	string name = "";
	LL band_limit = 0; //��������
	LL used_band[maxt]; //��¼��̨������ÿ��ʱ�̵����ô���ͣ���ֵ���ܳ���band_limit
	vector<int>neighbor; //��¼���������Ŀͻ�id
	bool is_chosed_for_90 = false;
	int degree() { return neighbor.size(); }
	vector<Node_fare>fare_sequence; //ʵʱά���ļƷ����У�������һ��ʱ��t���Ͱ����ʱ�̵���������ƷѶ��У�����������
	LL p95_band()  //����Ŀǰ�����м�¼��95����
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
	LL free_space(int t) //��ѿռ䣨����ָʣ��Ŷ��������95������Vʱ��Ϊ95����ֵ��������������ΪV
	{
		if (is_chosed_for_90)
		{
			int p90_t = p90_tim();
			LL p90_b = p90_band();
			if (p90_t != t)return max(p90_b, V);
			//�Ʒ������м�¼��95�������ǵ�ǰʱ�̣�����Ѿ�����V��������ѿռ�
			if (p90_b < V)return V;
			return 0;
		}
		int p95_t = p95_tim();
		LL p95_b = p95_band();
		if (p95_t != t)return max(p95_b, V);
		//�Ʒ������м�¼��95�������ǵ�ǰʱ�̣�����Ѿ�����V��������ѿռ�
		if (p95_b < V)return V;
		return 0;
	}

}server[maxn];
void keep_fare_sequence(int s_id, int t) //��used_band[t]ά���Ʒ�����
{
	LL used = server[s_id].used_band[t];
	LL p95_band = server[s_id].p95_band();
	if (used > p95_band) //ֻ�е�used����ԭ����95��������Ҫ���¼Ʒ�����
	{
		vector<Node_fare>& fs = server[s_id].fare_sequence;
		int len = fs.size();
		for (int i = 0; i < len; i++) //�����Ҵ�ʱ���Ƿ��Ѿ��ڼƷѶ�����
		{
			if (fs[i].tim == t) //����Ѿ��ڶ����У������ʹ�������������򼴿�
			{
				fs[i].used = used;
				sort(fs.begin(), fs.end(), cmp_Node_fare);
				return;
			}
		}
		//�������¼�����е�ʱ�̣�δ�����£�Ҳ������֮ǰ�������ڶ������Ų��Ϻ�
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
map<string, int>mp_client_to_id; //�ӿͻ����ֵ��±�
map<string, int>mp_server_to_id; //�ӷ��������ֵ��±�

void assign_a_piece(int& t, int& c_id, int& p_id, int& s_id)//��tʱ�̿ͻ�c_id�ĵ�p_id��СƬ�����������s_id
{
	server[s_id].used_band[t] += D_t_c_p[t][c_id][p_id];
	X_t_c_p[t][c_id][p_id] = s_id;
	D_client_moment[c_id][t] -= D_t_c_p[t][c_id][p_id];
	D_t_c_p[t][c_id][p_id] = 0;
}

//Ԥ����
struct Node_pre
{
	LL demand_sum = 0;
	int tim;
	LD average = 0; //��СƬ��С����ֵ
	LD variance = 0; //����
};
//Ԥ���䵱ǰ���ǵ����ĸ�������
int now_s_id_in_pre_allocate = 0;
bool cmp_Node_pre(Node_pre a, Node_pre b)
{
	LL band_limit = server[now_s_id_in_pre_allocate].band_limit;
	//������ʱ�̵���������������������������ʱ���Ƚ������С�Ѿ�������
	LD k = 1; //0.95�Ѿ�����
	if (a.demand_sum >= band_limit * k && b.demand_sum >= band_limit * k)
	{//��ֵ��ĺ�һЩ����Ϊʲô���������������
		return a.average > b.average;
		//return a.variance > b.variance;
	}
	return a.demand_sum > b.demand_sum;
}
bool cmp_server_for_pre_allocate(int i, int j) //��̫����
{
	return server[i].band_limit > server[j].band_limit;
}

int now_t_in_pre_allocate = 0;
struct cmp_for_pre_allocate { //�������ȶ���
	bool operator ()(int i, int j) { //����tureʱj�ڶѶ�����vector���Զ����������ò�һ��
		return D_client_moment[i][now_t_in_pre_allocate] < D_client_moment[j][now_t_in_pre_allocate];
	}
};
struct Fake_Server
{
	LL used_band[maxt];
}fake_server[maxn];

LL piece_size_threshold = 6000; //Ƭ��С����ֵ����С�ﵽ��ֵ��Ƭ��Ϊ�쳣ֵ���������ʱ����������
//����һ��ʱ����ģ��Ԥ����Ľ��
struct Node_fake_t_result
{
	int tim;  //����ģ���ʱ��
	LL demand_sum = 0;    //��ʱ���ھ�������
	LL used_band = 0;   //ʵ��װ�������
	int put_piece_num = 0;    //װ���Ƭ��
	LL max_piece_size = 0;    //װ������Ƭ��size
	int threshold_exceeded_num = 0;  //װ���Ƭ�У���С������ֵ��Ƭ��
	Node_fake_t_result(int Tim, LL D_sum, LL Used_band, int Put_num, LL Max_size, int Thre_num)
	{
		tim = Tim;
		demand_sum = D_sum; used_band = Used_band; put_piece_num = Put_num; max_piece_size = Max_size; threshold_exceeded_num = Thre_num;
	}
	LD evaluate() //���ض����װ��Ч��������ֵ��Խ������Ϊ��װ��Ч��Խ��
	{
		LD band_limit = server[now_s_id_in_pre_allocate].band_limit;
		return 100.0 * used_band / band_limit + 0.05 * used_band / put_piece_num + 10.0 * threshold_exceeded_num;
	}
};
//��κ�������ʱ�̣��ĸ����ʺϷ���Ԥ���䣿
//Ӧ���ۺϿ��ǵ��������������ʣ�ʱ��������ģ��װ��Ľ��
bool cmp_Node_fake_t_result(Node_fake_t_result a, Node_fake_t_result b)
{
	////��a��bʱ��װ��������Сʱ��Ӧ�ñȽ�������
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
void pre_allocate_all_free2() //Ԥ��������汾2������ÿ��ʱ��ģ���������䣬�ٸ���ÿ��ʱ�̵�װ��Ч��������Ҫ���ļ���ʱ����������Ԥ����
{


	vector<int>s_id_vec;
	for (int i = 1; i <= N; i++)
	{
		s_id_vec.push_back(i);
	}
	//���������򣬾���˭�Ƚ���Ԥ����
	sort(s_id_vec.begin(), s_id_vec.end(), cmp_server_for_pre_allocate);

	vector<int>beixuan_s_id_vec_for_90;
	for (int s_id = 1; s_id <= N; s_id++)
	{
		beixuan_s_id_vec_for_90.push_back(s_id);
	}
	sort(beixuan_s_id_vec_for_90.begin(), beixuan_s_id_vec_for_90.end(), cmp_pre_chose_s_for_90);

	int chose_s_cnt = 0;
	


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
			//��¼װ���Ƭ������С�ﵽ��ֵ��Ƭ����װ������Ƭ�Ĵ�С��ʱ����������
			int put_piece_num = 0; int threshold_exceeded_num = 0; LL max_piece_size = 0; LL demand_sum = 0;
			for (int node_idx = 0; node_idx < piece_num; node_idx++)
			{
				//����Ԥ����һ�����Ӵ�С����СƬ
				Node_piece piece = fake_piece_vec[node_idx];
				int c_id = piece.c_id;
				int p_id = piece.p_id;
				LL piece_size = D_t_c_p[fake_t][c_id][p_id];
				demand_sum += piece_size;                //��¼����֮��
				//ע�⣬ģ�����ʱ���ܸĶ��κζ���������ר��Ϊ�������׼����fake_server��used_band[ ]
				LL capacity = server[s_id].band_limit - fake_server[s_id].used_band[fake_t];

				if (capacity < piece_size)continue; //�Ų���

				//ģ������СƬ��ͬ����¼һЩЧ������ֵ
				if (piece_size >= piece_size_threshold)  //��С������ֵ������
				{
					threshold_exceeded_num++;
				}
				if (piece_size > max_piece_size)           //���Ƭ�����£�ʵ����Ҳ����װ��ĵ�һ��СƬ
				{
					max_piece_size = piece_size;
				}
				put_piece_num++;                         //�ܹ�װ�˼���СƬ������
				fake_server[s_id].used_band[fake_t] += piece_size;    //��ʱ�������ۼ�

			}///fake_tʱ�̵�СƬ���б����������
			//�����¼����Ϣ����ʱ�̣����������
			////����Node_fake_t_result(int Tim,LL D_sum, LL Used_band, int Put_num, LL Max_size, int Thre_num)
			Node_fake_t_result result(fake_t, demand_sum, fake_server[s_id].used_band[fake_t], put_piece_num, max_piece_size, threshold_exceeded_num);
			result_vec.push_back(result);
		}
		//���ϣ�����ʱ���ж�������һ��ģ�����
		sort(result_vec.begin(), result_vec.end(), cmp_Node_fake_t_result);
		//�ź���ǰ��percent5����¼��ʱ�̣�����Ӧ�þ���Ҫ����Ԥ�����ʱ�̣����������Ԥ����
		LL the_free_time_to_pre = 0;
		if (result_vec[percent_10 - 1].used_band >= server[s_id].band_limit / 5&&chose_s_cnt<10) {
			server[s_id].is_chosed_for_90 = true;
			chose_s_cnt++;
			chose_s_id_for_change.push_back(s_id);
		}
		if (server[s_id].is_chosed_for_90)the_free_time_to_pre = percent_10;
		else the_free_time_to_pre = percent5;
		for (int t_idx = 0; t_idx < the_free_time_to_pre; t_idx++) //ǰpercent5�Σ�Ҫô�������أ�Ҫô�������пͻ����ҷ������ھ��������ʱ��
		{
			int t = result_vec[t_idx].tim;
			now_t_in_pre_allocate = t; //��ȫ�ֱ������������ȶ���
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
				///��СƬΪԤ����ʱ��ȷ��������СƬ��Ǩ��ʱ�����ƶ�
				pre_allocate_piece_t_c_p[t][c_id][p_id] = 1;
			}
			//����ʱ�̿�����ɣ�ά���ƷѶ���
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
//////////////Ԥ�����һ�汾���£�ʹ�þ�ֵ����ѡ��ʱ��
void pre_allocate_all_free() //Ԥ�������
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
		if (server[s_id].degree() == 0)continue; //û�г��ȵķ�������û���κ����ü�ֵ
		//���Ƹ÷��������ھӶ���
		vector<int>s_neighbor;
		int len = server[s_id].neighbor.size();
		for (int c_idx = 0; c_idx < len; c_idx++)
		{
			int c_id = server[s_id].neighbor[c_idx];
			s_neighbor.push_back(c_id);
		}
		//ͬһ����������percent5��Ԥ���䶼�����ڲ�ͬʱ��
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
			//ͳ��tʱ���ж��ٸ�СƬ
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
			//��СƬ��С��ֵ
			nod.average = nod.demand_sum * 1.0 / p_num_in_t;
			//�󷽲�
			LD square_sum = 0; //���ƽ����
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
		//��demand_sum�Ѿ�����band_limitʱ���Ƚ�demand_sum�������壬Ӧ�ñȽ�ʲô�����Ƭ��������ϣ���������Ƭ��
		sort(sum_per_day.begin(), sum_per_day.end(), cmp_Node_pre);
		for (int t_idx = 0; t_idx < percent5; t_idx++) //ǰpercent5�Σ�Ҫô�������أ�Ҫô�������пͻ����ҷ������ھ��������ʱ��
		{
			int t = sum_per_day[t_idx].tim;
			now_t_in_pre_allocate = t; //��ȫ�ֱ������������ȶ���
			pre_allocate_server_moment[s_id][t] = 1; //��¼s_id��tʱ�̷���Ԥ����
			vector<Node_piece>piece_vec;
			//��tʱ�̵������ھ�СƬ�����������
			int pNum = PieceNumber_t[t];
			for (int c_idx = 0; c_idx < len; c_idx++)
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
//////////////Ԥ�����һ�汾����




LD delta_Cost_if_add_piece(int s_id, int t, LL a_piece) //tʱ�������a_piece���������s_id�������µĳɱ���������ͬʱ�������μƷѺ�����
{
	if (server[s_id].band_limit - server[s_id].used_band[t] < a_piece)return 1e8; //�����Ų��£����ؼ���ֵ�������Ŵ˷�����������
	LD p95_band = server[s_id].p95_band();
	if (p95_band < V)p95_band = V; //
	LD band_limit = server[s_id].band_limit;
	LD used_band_t = server[s_id].used_band[t];
	LD delta_p95 = a_piece + used_band_t - p95_band;
	if (delta_p95 <= 0)return 0;
	LD S = (1.0 + (2.0 * p95_band + delta_p95 - 2.0 * V) / band_limit) * delta_p95;
	return S;
}
//��ʽ2����˵Ӧ�ñ�����ĸ�³��һ�㣬���ǵ÷�ȴ�����һ���
LD delta_Cost_if_add_piece2(int s_id, int t, LL a_piece)
{
	if (server[s_id].band_limit - server[s_id].used_band[t] < a_piece)return 1e8; //�����Ų��£����ؼ���ֵ�������Ŵ˷�����������
	LD p95_band = server[s_id].p95_band();
	LD band_limit = server[s_id].band_limit;
	LD used_band_t = server[s_id].used_band[t];
	LD new_used_band_t = used_band_t + a_piece;
	if (new_used_band_t <= p95_band)return 0;
	//���汣֤tʱ�̵�����ͻ����p95������Ҳ������ͬһʱ��
	LD old_cost = 0, new_cost = 0;
	if (server[s_id].fare_sequence.size() == 0)old_cost = 0;
	else if (p95_band <= V)old_cost = V;
	else old_cost = (p95_band - V) * (p95_band - V) / band_limit + p95_band;
	if (new_used_band_t <= V)new_cost = V;
	else new_cost = (new_used_band_t - V) * (new_used_band_t - V) / band_limit + new_used_band_t;
	return new_cost - old_cost;
}
//Ĭ��ǰ�᣺���з����������õ���ÿ����Ҫ���ټƷ�V����V�ܴ�ʱ����ȷ��
void first_schedule()
{
	//����for�Կ������򣬵��Ƿ�Ҫ���ּܹ�
	for (int t = 1; t <= T; t++)
	{
		int pNum = PieceNumber_t[t];
		for (int c_id = 1; c_id <= M; c_id++) //�Ƿ�һ��Ҫһ�����ͻ����д���
		{
			//��СƬ���䵽��ѿռ��У��˹��̲����³ɱ���ߣ�Ҳ���ı�95�����¼

			for (int p_id = 1; p_id <= pNum; p_id++)
			{
				LL& a_piece = D_t_c_p[t][c_id][p_id];
				if (a_piece == 0)continue;
				//�������СƬ�÷ָ�˭
				int s_len = client[c_id].degree();
				for (int s_idx = 0; s_idx < s_len; s_idx++)
				{
					//��ʱ˳��š��Ƿ����ѡ���������
					int s_id = client[c_id].neighbor[s_idx];
					//��ѿռ�=max(p95_band, V)����Ŀǰ95��������Vʱ����ѿռ�ΪV
					LL free_capacity = server[s_id].free_space(t) - server[s_id].used_band[t];
					if (free_capacity >= a_piece)//СƬ���ɷָ�
					{
						assign_a_piece(t, c_id, p_id, s_id);
						break;
					}
				}
				//������ĳЩСƬ��û�ֳ�ȥ�����д�����������������
			}
		}
		//ÿ��ʱ�̼��д���û�зֳ�ȥ��СƬ����ЩСƬ���޷��Ž���ѿռ��У�Ҳ�������۷����ĸ��������������ı�95�����˹��̵��³ɱ����
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
				/////////©��ά����
				keep_fare_sequence(selected_s, t);
			}
		}
		//keep_all_fare_sequence(t);
	}
}

void first_schedule2()
{
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
		//��Ѱ����ѿռ䣬���ʹ�á���ѿռ価��������á������ѡ������ʵ���ѷ�������
		//������䷨��ѡ��������ѿռ���з��루Ŀǰ������Ч���Ϻã�
		//��С���䷨��ѡ����С����ѿռ���з���
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
		if (max_free_s_id != 0) //������䷨
		{
			assign_a_piece(t, c_id, p_id, max_free_s_id);
			keep_fare_sequence(max_free_s_id, t);
			continue;
		}
		//if (min_free_s_id != 0) //��С���䷨
		//{
		//	assign_a_piece(t, c_id, p_id, min_free_s_id);
		//	keep_fare_sequence(min_free_s_id, t);
		//	continue;
		//}

		//���û�ܷŵ���ѿռ��У���ѡȡ����֮��ɱ�������С�ķ���������
		//������ѿռ���õ�����£�Ӧ�÷ŵ��ĸ��������ϣ�
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
	cin.clear(); //����˲����ض��������ļ�
	//cout << "Q=" << Q << ", V=" << V << "\n";
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
	copy_D_to_D0(); //���ݶ���ĳ�ʼ����ֻ������
	make_adjacency_matrix();
}
bool cmp_server_by_band_limit(int i, int j) //��ÿ���ͻ���neighbor����������λ�ľ��ǵ�һ�����ˣ���δ�ؾ��������ĵ�һ������
{
	//4����3����2��˵��Ԥ�����кܴ�����
	//int threshold = 4;
	//if (server[i].degree() >= threshold && server[j].degree()<threshold)return true;
	//if (server[i].degree()<threshold && server[j].degree() >= threshold)return false;

	//���ȱȴ������ޣ��������޴����ǰ�������ͬ��ȴ����ǰ
	////////////Ϊʲô�����ֱ�������Ϳ����Ż���
	//if (server[i].degree() != server[j].degree())
	//{
	//	return server[i].degree() > server[j].degree();
	//}
	//return server[i].band_limit < server[j].band_limit;
	if (server[i].band_limit != server[j].band_limit)
	{
		return server[i].band_limit < server[j].band_limit;
	}
	return server[i].degree() < server[j].degree(); //��ϰ�׶��Ƕȴ�ĺã���ʽ���Ƕ�С�ĺ�
}
void pre_sort_client_neighbor()
{
	for (int c_id = 1; c_id <= M; c_id++)
	{
		sort(client[c_id].neighbor.begin(), client[c_id].neighbor.end(), cmp_server_by_band_limit);
	}
}

void regenerate_fare_sequence(int s_id) //�������ɼƷ����У�������ά���϶̳���
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
//Ǩ��ʱ��Ԥ����ʱȷ��λ�õ�Ƭ���ò��ö���
//void redistribution(vector<int>& s_id_vec, int times, int X_t_c_p[maxt][maxm][maxp])
//{
//	//srand(times * 223); //����������õģ����̶���������ӿ������ڽ������
//	//cout << "begin a redistribution\n";
//	srand(unsigned(time(0)));
//	random_shuffle(s_id_vec.begin(), s_id_vec.end());
//	for (int s1_idx = 0; s1_idx < N ; s1_idx++)
//	{
//		int s1 = s_id_vec[s1_idx];
//		//�����ڲ�ͻ��s1�ɱ�������£���s1�������ھӷָ�������������Ƭ�����Ƹ�s1�����У���
//		//��˼·��s1��Ǩ����������������Ƭ�������ߣ���Ǩ��������򲻹ܣ�ʵʱά����Ǩ���������p95��ʣ����ѿռ�
//		int c_len = server[s1].degree();
//		for (int c_idx = 0; c_idx < c_len; c_idx++)
//		{
//			int c_id = server[s1].neighbor[c_idx];
//			for (int t = 1; t <= T; t++)
//			{
//				int pNum = PieceNumber_t[t];
//				for (int p_id = 1; p_id <= pNum; p_id++)
//				{
//					if (pre_allocate_piece_t_c_p[t][c_id][p_id])continue; //Ԥ����ʱȷ��������СƬ�������ƶ�
//					if (X_t_c_p[t][c_id][p_id] == s1) //�ҵ���һ���ָ�s1��СƬ�������ܲ��ָܷ�����
//					{
//
//					}
//					//int s2 = X_t_c_p[t][c_id][p_id];
//					//if (s2 != 0&&s2!=s1) //�����ܲ��ܰ����Ƭ�Ƶ�s1
//					//{
//					//	LL a_piece = D0_t_c_p[t][c_id][p_id]; //��ԭʼ�����У���ѯСƬ�Ĵ�С
//					//	LL free_capacity = server[s1].free_space(t) - server[s1].used_band[t];
//					//	if (free_capacity < a_piece)continue; //СƬ�Ų���
//					//	
//					//	
//					//	//cout << "move piece,t=" << t << ", c_id=" << c_id << ",p_id=" << p_id << ",size=" << a_piece << ", from server s2=" << s2 << " , to s1=" << s1 << "\n";
//					//	//�����СƬ��s2��ȡ��������s1
//					//	//������ҪD_client_moment��D_t_c_p��
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
	//srand(times * 7); //����������õģ����̶���������ӿ������ڽ������
	//srand(times);
	srand(unsigned(time(0)));
	random_shuffle(s_id_vec.begin(), s_id_vec.end());
	for (int s1_idx = 0; s1_idx < N - 1; s1_idx++) //������ԣ���s2�������ƶ���s1
	{
		int s1 = s_id_vec[s1_idx]; //ȷ��s1ΪǨ��
		for (int s2_idx = s1_idx + 1; s2_idx < N; s2_idx++)
		{
			int s2 = s_id_vec[s2_idx]; //ȷ��s2ΪǨ��
			for (int c_id = 1; c_id <= M; c_id++) //ö�����пͻ����ҵ�s1��s2�Ĺ����ھ�
			{
				if (adjacency_c_s[c_id][s1] && adjacency_c_s[c_id][s2]) //ֻ��c_idͬʱ����s1��s2�ſ��Ը���������
				{
					//��c_id��s2��Ƭ�ƶ���s1
					for (int t = 1; t <= T; t++)
					{
						int pNum = PieceNumber_t[t];
						for (int p_id = 1; p_id <= pNum; p_id++)
						{
							if (pre_allocate_piece_t_c_p[t][c_id][p_id])continue; //Ԥ����ȷ����СƬ���ɶ�
							if (X_t_c_p[t][c_id][p_id] == s2) //�ҵ��˷ָ�s2��СƬ�������ܲ��ָܷ�s1
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
		clock_t end_time = clock(); //��������Ϊֹ������ʱ��

		if ((end_time - start_time) / CLOCKS_PER_SEC > time_limit_seconds)break;
		if (times >= loop_times_limit)break;
	}
}
void output_all_server_cost()
{
	LD total_cost = 0;
	cout << "ÿ���������ļƷѳɱ���\n";
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
	cout << "�ܳɱ���" << total_cost << "\n";
}
int main()
{
	srand(unsigned(time(0)));
	start_time = clock();
	std::ios_base::sync_with_stdio(false);
	cin.tie(NULL); cout.tie(NULL);
	//freopen("C:/Users/Touchlion/Desktop/fighting/ACM/2022hwsoft/������������/v0/solution.txt", "w", stdout);
	//��cout�ض���solution.txt�У����ύʱ��һ��Ҫ�����
	freopen("/output/solution.txt", "w", stdout);
	//freopen("solution1.txt", "w", stdout);
	input_PATH = "/data/";

	read_and_copy(); //����ȫ��

	percent95 = ceil(T * 0.95);
	percent5 = T - percent95;
	percent_90 = ceil(T * 0.9);
	percent_10 = T - percent_90 - 1;

	pre_sort_client_neighbor(); //������client��neighbor��������

	pre_allocate_all_free2(); //Ԥ���䣬��ע��ѡ��İ汾��

	first_schedule2();

	//output_all_server_cost();   /////������ʱ���������ǰ�����ĳɱ���Ϣ


	output_ans(X_t_c_p);
}
