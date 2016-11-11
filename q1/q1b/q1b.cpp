#include <iostream>
#include <cstdlib>
#include <sstream>
#include <fstream>
#include <vector>
using namespace std;

#define EXIT_FAILURE 1
#define EXIT_SUCCESS 0

class BayesianNetwork{
public:
	int n,x,y;
	vector<vector<int> > adj_in, adj_out;
	vector<int> active_trail, is_obs, vis;
	BayesianNetwork(){
		n=0;
	}
	~BayesianNetwork(){
		adj_in.clear();
		adj_out.clear();
	}
public:
	void init(int _n){
		n = _n;
		adj_in.assign(n+1, vector<int>());
		adj_out.assign(n+1, vector<int>());
	}
	void addEdge(int u, int v){
		adj_out[u].push_back(v);
		adj_in[v].push_back(u);
	}
	void print(){
		for(int i=1;i<=n;i++){
			for(int j=0;j<adj_out[i].size();j++){
				cout<<adj_out[i][j]<<" ";
			}
			cout<<endl;
		}
	}
	bool dfs(int k, bool is_incoming){
		if(vis[k])return 0;
		vis[k] = 1;
		if(k==y){
			active_trail.push_back(k);
			return true;
		}
		if(!is_incoming){
			if(!is_obs[k]){
				for(int i=0;i<adj_out[k].size();i++){
					if(dfs(adj_out[k][i], 1)){
						active_trail.push_back(k);
						return true;
					}
				}
				for(int i=0;i<adj_in[k].size();i++){
					if(dfs(adj_in[k][i], 0)){
						active_trail.push_back(k);
						return true;
					}
				}
			}
		}
		else{
			if(!is_obs[k]){
				for(int i=0;i<adj_out[k].size();i++){
					if(dfs(adj_out[k][i], 1)){
						active_trail.push_back(k);
						return true;
					}
					if(is_obs[adj_out[k][i]])
						is_obs[k]=1;
				}
			}
			if(is_obs[k]){
				for(int i=0;i<adj_in[k].size();i++){
					if(dfs(adj_in[k][i], 0)){
						active_trail.push_back(k);
						return true;
					}
				}
			}
		}
		return false;
	}
	bool isDSeparated(int X, int Y, vector<int>& obs){
		x=X,y=Y;
		is_obs.assign(n+1, 0);
		vis.assign(n+1, 0);
		for(auto x:obs)is_obs[x]=1;

		active_trail.clear();
		vis[X]=1;
		for(int i=0;i<adj_out[X].size();i++){
			int v = adj_out[X][i];
			if(dfs(v, 1)){
				active_trail.push_back(X);
				return true;
			}
		}
		for(int i=0;i<adj_in[X].size();i++){
			int v = adj_in[X][i];
			if(dfs(v, 0)){
				active_trail.push_back(X);
				return true;
			}
		}
		return false;
	}
};

int parse_network_file(string network_file, BayesianNetwork& bn){
	ifstream fin(network_file, ifstream::in);
	if(fin.fail()){
		return EXIT_FAILURE;
	}
	int n, u, v;
	string str;
	fin>>n;
	bn.init(n);
	for(int i=1;i<=n;i++){
		fin>>u>>str;
		stringstream ss(str.substr(1));
		while(ss>>v){
			bn.addEdge(u, v);
			if(ss.peek() == ',') ss.ignore();
			else if(ss.peek() == ']') break;
		}
	}
	return EXIT_SUCCESS;
}

struct Query{
	string str;
	int x, y;
	vector<int> obs;
	void print(){
		printf("%d %d %s\n", x, y, str.c_str());
	}
};

int parse_query_file(string query_file, vector<Query>& query){
	ifstream fin(query_file, ifstream::in);
	if(fin.fail()){
		return EXIT_FAILURE;
	}
	Query q;
	int i;
	while(fin>>q.x>>q.y>>q.str){
		stringstream ss(q.str.substr(1));
		q.obs.clear();
		while(ss>>i){
			q.obs.push_back(i);
			if(ss.peek() == ',') ss.ignore();
			else if(ss.peek() == ']') break;
		}
		query.push_back(q);
	}
	return EXIT_SUCCESS;
}

int main(int argc, char** argv){
	if(argc != 3){
		cout << "Invalid arguments!" << endl;
		cout << "./a.out <network-file> <query-file>" << endl;
		return EXIT_FAILURE;
	}
	string network_file(argv[1]);
	BayesianNetwork bn;
	if(parse_network_file(network_file, bn)){
		cout << "Error parsing network file!" << endl;
		return EXIT_FAILURE;
	}
	string query_file(argv[2]);
	vector<Query> query;
	if(parse_query_file(query_file, query)){
		cout << "Error parsing query file!" << endl;
		return EXIT_FAILURE;
	}
	for(auto q:query){
		q.print();
		bool ans = bn.isDSeparated(q.x, q.y, q.obs);
		if(!ans)printf("yes\n");
		else{
			printf("no [");
			for(int i=bn.active_trail.size()-1; i>0 ;i--)
				printf("%d,", bn.active_trail[i]);
			printf("%d]\n", bn.active_trail[0]);
		}
	}
	return EXIT_SUCCESS;
}