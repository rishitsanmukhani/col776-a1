#include <iostream>
#include <algorithm>
#include <cstdio>
#include <random>
#include <vector>
using namespace std;

#define EXIT_FAILURE 1
#define EXIT_SUCCESS 0

vector<vector<int> > G;
vector<int> rv;

int main(int argc, char** argv){
	if(argc<3){
		cout<<"Invalid arguments!"<<endl;
		return EXIT_FAILURE;
	}
	int n,k;
	n = atoi(argv[1]);
	k = atoi(argv[2]);
	
	srand(time(NULL));
	default_random_engine generator(time(NULL));
	uniform_int_distribution<int> distribution(0,k);
	G.resize(n+1);
	for(int i=1;i<=n;i++){
		int u = distribution(generator);
		for(int j=i+1;j<=n;j++){
			G[i].push_back(j);
		}
		random_shuffle(G[i].begin(), G[i].end());
		if(G[i].size()>u)
			G[i].resize(u);
		sort(G[i].begin(), G[i].end());
	}
	printf("%d\n",n);
	for(int i=1;i<=n;i++){
		printf("%d [", i);
		if(G[i].empty()){
			printf("]\n");
		}
		else{
			for(int j=0;j<G[i].size()-1;j++){
				printf("%d,", G[i][j]);
			}
			printf("%d]\n", G[i].back());
		}
	}
	return EXIT_SUCCESS;
}
