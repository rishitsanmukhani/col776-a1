#include <iostream>
#include <fstream>
#include <cstring>
#include <cstdio>
#include <vector>
#include <cfloat>
#include <cmath>
#include <map>
using namespace std;

#define EXIT_FAILURE 1
#define EXIT_SUCCESS 0


const int num_chars = 10, max_word_len = 6, max_img_id = 1000;
const char chars[num_chars] = {'a', 'd', 'e', 'h', 'i', 'n', 'o', 'r', 's', 't'};
map<char, int> char_to_id;
	

class Word{
public:
	char actual[max_word_len+1], predicted[max_word_len+1];
	vector<int8_t> actual_int, predicted_int;
	vector<int> img;
	int sz, num_correct;
	double log_p;
	void init(){
		sz = strlen(actual);
		img.resize(sz);
		actual_int.resize(sz);
		predicted_int.resize(sz);
	}
	void convert_to_int(){
		for(int i=0;i<sz;i++){
			actual_int[i] = char_to_id[actual[i]];
		}

	}
	void convert_to_char(){
		num_correct=0;
		for(int i=0;i<sz;i++){
			predicted[i] = chars[predicted_int[i]];
			if(predicted[i]==actual[i])
				num_correct++;
		}
		predicted[sz]='\0';
	}
	void print(){
		printf("Actual   : %s\n", actual);
		printf("Predicted: %s\n", predicted);
		printf("[%d / %d matched] %lf\n\n", num_correct, sz, log_p);
	}
};

vector<vector<int8_t> > per[max_word_len+1];
void init(){
	// n = 1
	for(int i=0;i<num_chars;i++){
		per[1].push_back(vector<int8_t>(1,i));
	}
	// n = 2 ... max_word_len
	for(int n=2; n<=max_word_len ;n++){
		for(auto& v:per[n-1]){
			for(int i=0;i<num_chars;i++){
				per[n].push_back(v);
				per[n].back().push_back(i);
			}
		}
	}
}

vector<Word> words;
int parse_input(string ocr_filename, string word_filename){
	FILE* file1 = fopen(ocr_filename.c_str(), "r");
	FILE* file2 = fopen(word_filename.c_str(), "r");
	if(!file1 || !file2){
		return EXIT_FAILURE;
	}
	Word w;
	while(fscanf(file2, "%s", w.actual)!=EOF){
		w.init();
		for(int i=0;i<w.sz;i++){
			fscanf(file1,"%d", &w.img[i]);
		}
		w.convert_to_int();
		words.push_back(w);
	}
	fclose(file2);
	fclose(file1);
	return EXIT_SUCCESS;
} 

class Model{
public:
	double trans[num_chars][num_chars], log_trans[num_chars][num_chars];
	double ocr[max_img_id+1][num_chars], log_ocr[max_img_id+1][num_chars];
	double skip, log_skip;
	
	int init(string trans_filename, string ocr_filename){
		init_char_to_id();
		if(init_transition_factors(trans_filename)){
			printf("Error parsing %s", trans_filename.c_str());
			return EXIT_FAILURE;
		}
		if(init_ocr_factors(ocr_filename)){
			printf("Error parsing %s", ocr_filename.c_str());
			return EXIT_FAILURE;
		}
		skip = 5.0;
		log_skip = log(skip);
		return EXIT_SUCCESS;
	}
	void init_char_to_id(){
		for(int i=0;i<num_chars;i++)
			char_to_id[chars[i]]=i;
	}
	int init_transition_factors(string filename){
		FILE* file = fopen(filename.c_str(), "r");
		if(!file){
			return EXIT_FAILURE;
		}
		int num_lines = num_chars * num_chars;
		char c1, c2;
		double p;
		for(int i=0;i<num_lines;i++){
			fscanf(file, " %c %c %lf", &c1, &c2, &p);
			trans[char_to_id[c1]][char_to_id[c2]] = p;
			log_trans[char_to_id[c1]][char_to_id[c2]] = log(p);
		}
		fclose(file);
	}
	int init_ocr_factors(string filename){
		FILE* file = fopen(filename.c_str(), "r");
		if(!file){
			return EXIT_FAILURE;
		}
		int num_lines = max_img_id * num_chars, id;
		char c;
		double p;
		for(int i=0;i<num_lines;i++){
			fscanf(file, "%d %c %lf", &id, &c, &p);
			ocr[id][char_to_id[c]] = p;
			log_ocr[id][char_to_id[c]] = log(p);
		}
		fclose(file);
	}
	double log_prob(vector<int>& img, vector<int8_t>& c, int type){
		double res = 0;
		int sz = img.size();
		for(int i=0;i<sz;i++){
			res += log_ocr[img[i]][c[i]];
		}
		if(type>0){
			for(int i=0;i<sz-1;i++){
				res += log_trans[c[i]][c[i+1]];
			}
		}
		if(type>1){
			for(int i=0;i<sz;i++)
				for(int j=i+1;j<sz;j++)
					if(c[i]==c[j] && img[i] == img[j])
						res += log_skip;
		}
		return res;
	}
	void predict(Word& w, int type=0){
		double p=0, zimg = 0, maxi=-DBL_MAX;
		int idx;
		for(int i=0;i<per[w.sz].size();i++){
			p = log_prob(w.img, per[w.sz][i], type);
			if(p>maxi)
				maxi = p, idx = i;
			zimg += exp(p);
		}
		w.predicted_int = per[w.sz][idx];
		w.log_p = log_prob(w.img, w.actual_int, type) - log(zimg);
		w.convert_to_char();
	}
};
Model model;
int main(int argc, char** argv){
	if(argc!=5){
		cout<<"Invalid Arguments!"<<endl;
		return EXIT_FAILURE;
	}
	init();
	if(model.init(string(argv[3]), string(argv[4]))){
		return EXIT_FAILURE;
	}
	if(parse_input(string(argv[1]), string(argv[2]))){
		return EXIT_FAILURE;
	}
	for(int type=0;type<=2;type++){
		printf("Model #%d\n", type);
		for(auto& w:words){
			model.predict(w,type);
			w.print();
		}
		int cor_chars=0, total_chars=0;
		int cor_words=0, total_words=0;
		double log_p=0;
		for(auto w:words){
			cor_chars += w.num_correct;
			total_chars += w.sz;
			cor_words += (w.num_correct == w.sz);
			total_words++;
			log_p += w.log_p;
		}
		printf("Char Accuracy: %lf\n", 100*(double(cor_chars)/total_chars));
		printf("Word Accuracy: %lf\n", 100*(double(cor_words)/total_words));
		printf("Avg log prob : %lf\n\n", (log_p/total_words));
	}
	return 0;
}