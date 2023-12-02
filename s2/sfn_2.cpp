#include <fstream>
#include <vector>
#include <random>
#include <chrono>
#include <iostream>
#include <unordered_map>
#include <omp.h>
using namespace std;

static int total_degree = 0;
static int scale = 0;
static int edge = 0;
static int total = 0;
static int init = 0;

struct vertex {
    vector<int> o_idx; // 出边索引
    vector<int> i_idx; // 入边索引
    int idx;           // 顶点编号
    int degree;        // 顶点度数

    // 构造函数，用于初始化顶点的出边索引和度数
    vertex(int edges = 0) : degree(0) {
        o_idx.resize(edges, 0); // 根据传入的edges参数初始化出边索引向量
    }
};

class SFN {
private:
    default_random_engine engine; // 随机数生成器
    uniform_real_distribution<double> prob_distribution; // 均匀分布随机数生成器
    uniform_int_distribution<int> index_distribution; // 均匀分布整数随机数生成器

    vector<vertex> graph; // 存储图的顶点

public:
    SFN(int link_edges = 0, int init_vertexes = 0, int num_vertexes = 0);
    void __init_mem_space__();
    void __generation__();
    void __link_vertex__(int a, int b, int &cnt);

    void floyd(vector<int> &dis);
    int find(int target, int idx);
    double coefficient();

    void save_distribution();
    
    double linking_prob();
    double be_linked_prob(const vertex& vx);
    int random_index();
};

// 构造函数
SFN::SFN(int link_edges, int init_vertexes, int num_vertexes): engine(chrono::steady_clock::now().time_since_epoch().count()) {
    edge = link_edges; // 边数
    init = scale = init_vertexes; // 初始顶点数
    total = num_vertexes; // 总顶点数
    __init_mem_space__(); // 初始化存储空间
    __generation__(); // 生成无标度网络
}

// 初始化存储空间
void SFN::__init_mem_space__() {
    cout << "Initialization Started" << endl;
    for (int i = 0; i < init; ++i) {
        vertex vx;
        vx.idx = i;     // 记录顶点对应下标值
        graph.push_back(vx);    // 将顶点连入图的存储空间中
    }   
    cout << "Initialization Finished" << endl;
}

// 生成无标度网络
void SFN::__generation__() {
    cout << "Generation Started" << endl;
    #pragma omp parallel for num_threads(2)
    for (int i = init; i < total; ++i) {
        int cnt = 0;
        unordered_map<int, int> hash;
        #pragma omp parallel for num_threads(4)
        while (cnt < edge) {
            int idx = random_index();   // 随机生成下标值
            if (hash.count(idx) != 0) continue;
            double rand_prob = linking_prob();  // 随机生成一个浮点数判断连接与否
            double linked_prob = be_linked_prob(graph[idx]);    // 生成对应顶点的被连接概率
            if (total_degree < 10) {    // 前十个度数的时候为了防止网络失活进入死循环，直接可以连接
                __link_vertex__(i, idx, cnt);   // 连接函数
                hash[idx] = 0;  // 给哈希表对应下标位置赋一个值，用以保证不会重链接
            } else if (rand_prob < linked_prob) {
                __link_vertex__(i, idx, cnt);
                hash[idx] = 0;  // 给哈希表对应下标位置赋一个值，用以保证不会重链接
            }
        }
        // 打印百分比进度条
        int progress = ((i - init + 1) * 100) / (total - init);
        cout << "Progress: [" << string(progress / 2, '=') << ">" << string(50 - progress / 2, ' ') << "] " << progress << "%" << "\r";
        cout.flush();
    }
    cout << endl;
    cout << "Generation Finished" << endl;
}

// 连接顶点
void SFN::__link_vertex__(int a, int b, int &cnt) {
    // 构造一个有edge个连出边的顶点
    vertex vx(edge);
    // vx的连出边向量加上b, graph[b]的连入边向量加上a
    vx.o_idx.push_back(b);
    graph[b].i_idx.push_back(a);
    // vx度加1，graph[b]的度加1
    vx.degree++;
    graph[b].degree++;
    // 更新cnt的值，用于判断内层循环退出与否，度数每次连边加2
    total_degree += 2;
    cnt++;          
    // 当cnt等于edge时，代表连边完成，将顶点放入图结构中，并增大scale
    if(cnt == edge) {
        vx.idx = a;     // 记录顶点在图存储空间中的下标值
        graph.push_back(vx);    // 存储进入
        scale++;        // 增添网络规模
    }
}

// Floyd算法计算最短距离
void SFN::floyd(vector<int> &dis) {
    int count = 0;
    #pragma omp parallel for num_threads(8) // 使用OpenMP并行化，使用8个线程并行计算
    for(int i = 0; i < graph.size(); ++i) { // 遍历所有节点
        for(int j = 0; j < graph.size() && j != i; ++j) { // 遍历除自身以外的所有节点
            int flag = 0;
            for(int k = 0; k < graph[i].o_idx.size(); ++k) { // 如果节点i直接连接节点j，距离为1
                if(graph[i].o_idx[k] == j) {
                    dis.push_back(1);
                    flag = 1; // 标记已找到直接连接的情况
                    break;
                }                
            }
            if(flag) continue; // 如果找到直接连接的情况，跳过后面的循环
            for(int k = 0; k < graph[i].o_idx.size(); ++k) {
                int num = find(j, graph[i].o_idx[k]); // 调用find函数找到节点i到节点j的最短距离
                if(num >= 0)
                    dis.push_back(num); // 将距离加入dis数组
            }
        }
        #pragma omp atomic // 使用原子操作保证count的线程安全
        count++;
        // 打印百分比进度条
        int progress = (count * 100) / graph.size();
        cout << "Progress: [" << string(progress / 2, '=') << ">" << string(50 - progress / 2, ' ') << "] " << progress << "%" << "\r";
        cout.flush(); // 将输出立即刷新到控制台
    }
    cout << endl; // 打印换行符，避免在进度条之后输出其他内容
}

int SFN::find(int target, int idx) {
    if(target == idx) { // 如果目标节点等于当前节点，直接返回1
        return 1;
    } else {
        vector<int> visited(graph.size(), 0); // 标记是否访问过某个节点
        vector<int> stack; // 使用栈来实现深度优先搜索
        vector<int> steps(graph.size(), 0); // 记录每个节点的步数
        stack.push_back(idx); // 将起始节点加入栈中
        steps[idx] = 1; // 起始节点的步数为1
        while (!stack.empty()) { // 当栈不为空时循环
            int current = stack.back(); // 取出栈顶元素
            stack.pop_back(); // 弹出栈顶元素
            visited[current] = 1; // 标记该节点已访问过
            for (int i = 0; i < graph[current].o_idx.size(); ++i) { // 遍历当前节点的所有相邻节点
                int next = graph[current].o_idx[i];
                if (next == target) { // 如果找到目标节点，返回目标节点的步数
                    return steps[current] + 1;
                }
                if (!visited[next]) { // 如果相邻节点未访问过，将其加入栈中，并记录其步数
                    stack.push_back(next);
                    steps[next] = steps[current] + 1;
                }
            }
        }
        return -1; // 如果未找到路径，返回-1
    }
}


double SFN::coefficient() {
    double Csum = 0;
    for(int i = 0; i < graph.size(); ++i) {
        double Ci = 0;
        int ki = graph[i].o_idx.size() + graph[i].i_idx.size();
        
        // 计算邻居节点之间仍未邻居节点的个数之和
        for(int j = 0; j < graph[i].o_idx.size(); ++j) {
            int neighbor1 = graph[i].o_idx[j];
            for(int k = j+1; k < graph[i].o_idx.size(); ++k) {
                int neighbor2 = graph[i].o_idx[k];
                if(find(neighbor2, neighbor1) == -1) {
                    Ci++;
                }
            }
        }
        Csum += Ci / (ki * (ki - 1) / 2); // 将Ci除以邻居节点对的总数，并累加到Csum中
    }
    return Csum / graph.size(); // 返回平均聚集系数
}

// 存储度分布
void SFN::save_distribution() {
    ofstream ofile("degree_distribution.csv");
    if(ofile.is_open() == false) 
        return;
    int max = graph[0].degree;
    for(int i = 0; i < graph.size(); ++i) 
        if(graph[i].degree > max) 
            max = graph[i].degree;
    vector<int> degrees(max + 1, 0);
    for(int i = 0; i < graph.size(); ++i) 
        degrees[graph[i].degree]++;

    for(int i = 0; i < degrees.size(); ++i) 
        ofile << i << ", " << degrees[i] << endl;
    ofile.close();
}

// 生成连接概率
double SFN::linking_prob() {
    prob_distribution = uniform_real_distribution<double>(0.0, 1.0);
    return prob_distribution(engine);
}

// 生成被连接概率
double SFN::be_linked_prob(const vertex& vx) {
    return (double)vx.degree / total_degree;
}

// 随机生成下标值
int SFN::random_index() {
    index_distribution = uniform_int_distribution<int>(0, scale - 1);
    return index_distribution(engine);
}

// 主函数
int main() {
    int m, m0, t;
    cout << "Link_edges_of_vertexes: "; cin >> m;
    cout << "Init_vertexes: ";          cin >> m0;
    cout << "Num_vertexes: ";           cin >> t;
    auto start_time = chrono::steady_clock::now();

    SFN sfn(m, m0, t);

    sfn.save_distribution(); // 存储度分布

    cout << endl << "Calculating Least Distance" << endl;
    vector<int> dis;
    sfn.floyd(dis); // 计算最短距离
    int sum = 0;
    for(int i = 0; i < dis.size(); ++i) 
        sum += dis[i];
    cout << "Finished" << endl;
    cout << "Sum: " << sum << " Average: " << (double)sum / t << endl;
    cout << "Baseline: " << log(t) / log(2) << endl;

    cout << endl << "Calculating Cluster Coefficient" << endl;
    double coef = sfn.coefficient(); // 计算聚集系数
    cout << "Cluster Coefficient: " << coef << endl;

    auto end_time = chrono::steady_clock::now();
    auto elapsed_time = chrono::duration_cast<chrono::seconds>(end_time - start_time).count();

    cout << endl << "Elapsed time: " << elapsed_time << " seconds." << endl;

    return 0;
}
