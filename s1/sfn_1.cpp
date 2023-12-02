#include <iostream>
#include <vector>
#include <ctime>
#include <chrono>
#include <fstream>
using namespace std;

static int range = 0;
static int edge = 0;
static unsigned long long _cnt_ = 0;
static unsigned long long k_sum = 0;

double RandDouble();
double RandDouble(int range);
int RandInt(int num);
void ShowInfo(int index, double rand_num, double prob_link, int cnt); 

class Node {
    private:
        int k;
        vector<vector<double>> edges;
        vector<double> position;
    public:
        Node(): k(0) {
            position.resize(2, RandDouble(range));
        }
        Node(int n): k(0) {
            position.resize(2);
        }
        friend void Link(std::vector<Node> &sfn, Node &p);
        friend void GetPosition(Node &p);
        friend void LinkEdges(std::vector<Node> &sfn, const int &index, Node &p);
        friend bool IsAvai(Node x, Node p);
        friend void CalculateDistribution(std::vector<Node> sfn, std::vector<double> &dis);
        friend void SaveDistribution(std::vector<Node> sfn, const std::string &filename);
        friend void SavePosition(std::vector<Node> sfn, const std::string &filename);
};

// 友元函数区
void GetPosition(Node &p) {
    p.position[0] = RandDouble(range);
    p.position[1] = RandDouble(range);
}
void LinkEdges(vector<Node> &sfn, const int &index, Node &p) {
    p.edges.push_back(sfn[index].position);
    p.k++;
    sfn[index].k++;
    int sum = 0;
    for(int i = 0; i < sfn.size(); i++) {
        sum += sfn[i].k;
    } k_sum = sum;
}
bool IsAvai(Node x, Node p) {
    for(int i = 0; i < p.edges.size(); i++) {
        if(x.position[0] == p.edges[i][0] && x.position[1] == p.edges[i][1]) {
            return false;
        } else continue;
    } return true;
}
void Link(vector<Node> &sfn, Node &p) {    
    int cnt = 0; // Count the number of nodes connected successfully
    GetPosition(p);
    while(cnt < edge) {
        _cnt_++;
        int index = RandInt(sfn.size()); // Generate a random integer in the range of 0~sfn.size() - 1 as the index
        double rand_num = RandDouble();  // Generate a random double between 0~1 to determine whether to link or not
        double prob_link;
        if(k_sum > 4) {
            prob_link = (double) sfn[index].k / k_sum;             
        } else {
            if(k_sum == 0) {
                prob_link = 1.0;
            } else prob_link = 1 - 1.0 / k_sum;
        }
        // ShowInfo(index, rand_num, prob_link, cnt);
        if(prob_link >= rand_num) {
            if(cnt == 0) {
                LinkEdges(sfn, index, p);
            } else {
                bool tmp = IsAvai(sfn[cnt], p);
                if(tmp) LinkEdges(sfn, index, p);
            }
            cnt++;
        }
    }
    sfn.push_back(p);
}

// Ordinary Functions
void Init(vector<Node> &sfn, int m0) {
    for(int i = 0; i < m0; i++) {
        Node p;
        sfn.push_back(p);        
    }
}
void ShowInfo(int index, double rand_num, double prob_link, int cnt) {
    cout << "index: " << index << endl 
         << "rand_num: " << rand_num << endl
         << "prob_link: " << prob_link << endl
         << "cnt: " << cnt << endl 
         << "k_sum: " << k_sum << endl << endl;
}
template <class T>
void ShowCPUTimes(T &start) {
    auto end = std::chrono::system_clock::now(); // Record the ending time
    std::chrono::duration<double> elapsed_seconds = end - start; // Calculate the time spent
    std::cout << "Elapsed time: " << elapsed_seconds.count() << "s\t   ";
    std::cout << "Calculation times: " << _cnt_ << " times" << endl << endl;
}

// Random number generation function
double RandDouble() { // Generate a random double between 0~1 in order to determine whether to link to the found node or not
    double random_double = static_cast<double>(rand()) / static_cast<double>(RAND_MAX); 
    return random_double;
}
double RandDouble(int range) { 
    double random_double = static_cast<double>(rand()) / RAND_MAX * range;
    return random_double;
}
int RandInt(int num) { // Generate a random integer lower than num-of-nodes as index to find node
    int random_int = static_cast<int>(rand()) % num;  
    return random_int; 
}

// Data saving functions
void CalculateDistribution(vector<Node> sfn, vector<double> &dis) {
    int k_max = sfn[0].k;
    for(int i = 0; i < sfn.size(); i++) {
        if(sfn[i].k > k_max) k_max = sfn[i].k;
    }
    dis.resize(k_max + 1, 0);
    for(int i = 0; i < sfn.size(); i++) {
        dis[sfn[i].k]++;
    }
}
void SaveDistribution(vector<double> sfn, const string &filename) {
    ofstream ofile(filename);
    if(!ofile.is_open()) {
        cout << "FAIL TO OPEN FILE: " << filename << endl;
        return;
    } else {
        for(int i = 0; i < sfn.size(); i++) {
            ofile << i << "," << sfn[i] << endl;
        } cout << "DISTRIBUTION DATA SAVED IN FILE: " << filename << endl;
    }
}
void SavePosition(vector<Node> sfn, const string &filename) {
    ofstream ofile(filename);
    if(!ofile.is_open()) {
        cout << "FAIL TO OPEN FILE: " << filename << endl;
        return;
    } else {
        for(int i = 0; i < sfn.size(); i++) {
            ofile << sfn[i].position[0] << "," << sfn[i].position[1] << ",";
            if(sfn[i].edges.size() < edge) {
                for(int j = 0; j < sfn[i].edges.size(); j++) {
                    ofile << RandDouble(range) << "," << RandDouble(range) << ",";
                } ofile << endl;                
            } else {
                for(int j = 0; j < sfn[i].edges.size(); j++) {
                    ofile << sfn[i].edges[j][0] << "," << sfn[i].edges[j][1] << ",";
                } ofile << endl;                
            } 
        }
        cout << "POSITION DATA SAVED IN FILE: " << filename << endl;
    }
}

int main() {
    //  ========= Generating Network =========  //
    auto start = std::chrono::system_clock::now();   // Record the start time
    srand(static_cast<unsigned int>(time(nullptr))); // Using current time as the seed of random nums
    int m, m0, t;
    cout << "Input m, m0, t: " << endl;
    cin >> m >> m0 >> t;
    range = t; edge = m;
    vector<Node> sfn;
    // Initiate the Scale-Free Network, input m0 as the init num of nodes
    Init(sfn, m0);
    int count = 0;
    for(int i = 0, j = 0; i < t; i++, j++) {
        Node p(m);
        Link(sfn, p);
        if(j == t / 100) {
            count++;
            cout << "Already finished: " << count << " / 100\t";
            j = 0;        
            ShowCPUTimes(start);
        }
    } 
    cout << "Already finished: " << count + 1 << " / 100" << endl;
    ShowCPUTimes(start);
    cout << "<<<<<<<<--------------========== FINISHED =========--------------->>>>>>>>" << endl;

    //  ========= Degree distribution calculation and storage =========  //
    vector<double> dis;
    CalculateDistribution(sfn, dis);
    SaveDistribution(dis, "kDistribution_data.csv");

    //  ========= Node addresses storage =========  //
    SavePosition(sfn, "locations.csv");
}
