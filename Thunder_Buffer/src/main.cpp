#include <cstdio>
#include <string>
#include <iostream>
#include <map>
#include "solutions.h"
#ifdef UNIX
#include <sys/time.h>
#include <cassert>
#endif

using namespace std;

char input_data[5 * 1024 * 1024 * 16];
int magic[128 * 1024 * 1024];

void load_data(FILE * pf, int batch_size, map<string, int>& gt)
{
	fread(input_data, 16, batch_size, pf);
	cout << "batch load done." << endl;

	// prepare ground truth
	for (int i = 0; i < batch_size; ++i) {
		string tmp(input_data + i * 16 + 8, 8);
		gt[tmp] += 1;
	}
	cout << "ground truth done." << endl;
	cout << gt.size() << endl;
}

int run_a_case(pair<string, Solution *>& ps, int batch_size, map<string, int>& gt, FILE * pout, double memory, bool verbose = true)
{
	ps.second->init(int(memory * 1024 * 1024));

	// refresh memory
	for (int j = 0; j < sizeof(magic) / sizeof(int); ++j) {
//		magic[j] = j;
	}
//	if (verbose) {cout << "refresh memory done" << endl;}

	double interval = 0;
#ifdef UNIX
	struct timeval st, ed;
	gettimeofday(&st, NULL);
#endif
	int result = ps.second->build(input_data, batch_size, 16);
#ifdef UNIX
	gettimeofday(&ed, NULL);
	interval = (ed.tv_sec - st.tv_sec) + (ed.tv_usec - st.tv_usec) / 1000000.0;
#endif
	cout << ps.first << " ";
	if (result == 0) {
		// query
		int tot = 0, acc = 0, err = 0;
		for (const auto &kvp : gt) {
			tot++;
			unsigned ret = ps.second->query(get_key(kvp.first.c_str()));
			if (kvp.second == ret) {
				acc++;
			} else {
				err += ret - kvp.second;
			}
		}
		double acc_per = double(acc) / tot;
		double err_per = double(err) / tot;
		cout << acc_per << " " << ps.second->memory_accesses;
		fprintf(pout, "%s, %.4lf, %.6lf, %.6lf, %.6lf, %d\n", ps.first.c_str(), memory, interval, acc_per, err_per, ps.second->memory_accesses);
	} else {
		cout << "build failed at " << result;
	}
	cout << endl;

	delete ps.second;
}

void exp_var_memory(FILE * pout, int batch_size, map<string, int>& gt)
{
	double memorys[] = { 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024 };
	//double memorys[] = { 16 };
	for (unsigned _t = 0; _t < sizeof(memorys) / sizeof(double); ++_t) {
		double memory = memorys[_t];
		pair<string, Solution *> ps[] = {
            pair<string, Solution *>("2500X4R", new SolutionFilteredLinearHash<2500, 4, 1, global_round_robin>()),
			pair<string, Solution *>("5000X2R", new SolutionFilteredLinearHash<5000, 2, 1, global_round_robin>()),
			pair<string, Solution *>("10000X1R", new SolutionFilteredLinearHash < 10000, 1, 1, global_round_robin>()),
            pair<string, Solution *>("2500X4L", new SolutionFilteredLinearHash<2500, 4, 1, approximate_LRU>()),
            pair<string, Solution *>("5000X2L", new SolutionFilteredLinearHash<5000, 2, 1, approximate_LRU>()),
			pair<string, Solution *>("10000X1L", new SolutionFilteredLinearHash < 10000, 1, 1, approximate_LRU>()),
			pair<string, Solution *>("linear hash", new SolutionLinearHash()),
		};
		const int case_cum = sizeof(ps) / sizeof(pair<string, Solution *>);

		for (int i = 0; i < case_cum; ++i) {
			run_a_case(ps[i], batch_size, gt, pout, memory);
		}
	}
}

void exp_var_queue(FILE * pout, int batch_size, map<string, int>& gt)
{
	double memory = 32;

	pair<string, Solution *> ps[] = { 
		pair<string, Solution *>("128@4@global_round_robin", new SolutionFilteredLinearHash<32, 1, 0, global_round_robin>()),
		pair<string, Solution *>("128@4@approximate_LRU", new SolutionFilteredLinearHash<32, 1, 0, approximate_LRU>()),
		pair<string, Solution *>("128@4@evict_clock", new SolutionFilteredLinearHash<32, 1, 0, evict_clock>()),
		pair<string, Solution *>("128@8@global_round_robin", new SolutionFilteredLinearHash<16, 2, 0, global_round_robin>()),
		pair<string, Solution *>("128@8@approximate_LRU", new SolutionFilteredLinearHash<16, 2, 0, approximate_LRU>()),
		pair<string, Solution *>("128@8@evict_clock", new SolutionFilteredLinearHash<16, 2, 0, evict_clock>()),
		pair<string, Solution *>("128@16@global_round_robin", new SolutionFilteredLinearHash<8, 4, 0, global_round_robin>()),
		pair<string, Solution *>("128@16@approximate_LRU", new SolutionFilteredLinearHash<8, 4, 0, approximate_LRU>()),
		pair<string, Solution *>("128@16@evict_clock", new SolutionFilteredLinearHash<8, 4, 0, evict_clock>()),
		pair<string, Solution *>("128@32@global_round_robin", new SolutionFilteredLinearHash<4, 8, 0, global_round_robin>()),
		pair<string, Solution *>("128@32@approximate_LRU", new SolutionFilteredLinearHash<4, 8, 0, approximate_LRU>()),
		pair<string, Solution *>("128@32@evict_clock", new SolutionFilteredLinearHash<4, 8, 0, evict_clock>()),
		pair<string, Solution *>("128@64@global_round_robin", new SolutionFilteredLinearHash<2, 16, 0, global_round_robin>()),
		pair<string, Solution *>("128@64@approximate_LRU", new SolutionFilteredLinearHash<2, 16, 0, approximate_LRU>()),
		pair<string, Solution *>("128@64@evict_clock", new SolutionFilteredLinearHash<2, 16, 0, evict_clock>()),
		pair<string, Solution *>("128@128@global_round_robin", new SolutionFilteredLinearHash<1, 32, 0, global_round_robin>()),
		pair<string, Solution *>("128@128@approximate_LRU", new SolutionFilteredLinearHash<1, 32, 0, approximate_LRU>()),
		pair<string, Solution *>("128@128@evict_clock", new SolutionFilteredLinearHash<1, 32, 0, evict_clock>()),
		pair<string, Solution *>("256@4@global_round_robin", new SolutionFilteredLinearHash<64, 1, 0, global_round_robin>()),
		pair<string, Solution *>("256@4@approximate_LRU", new SolutionFilteredLinearHash<64, 1, 0, approximate_LRU>()),
		pair<string, Solution *>("256@4@evict_clock", new SolutionFilteredLinearHash<64, 1, 0, evict_clock>()),
		pair<string, Solution *>("256@8@global_round_robin", new SolutionFilteredLinearHash<32, 2, 0, global_round_robin>()),
		pair<string, Solution *>("256@8@approximate_LRU", new SolutionFilteredLinearHash<32, 2, 0, approximate_LRU>()),
		pair<string, Solution *>("256@8@evict_clock", new SolutionFilteredLinearHash<32, 2, 0, evict_clock>()),
		pair<string, Solution *>("256@16@global_round_robin", new SolutionFilteredLinearHash<16, 4, 0, global_round_robin>()),
		pair<string, Solution *>("256@16@approximate_LRU", new SolutionFilteredLinearHash<16, 4, 0, approximate_LRU>()),
		pair<string, Solution *>("256@16@evict_clock", new SolutionFilteredLinearHash<16, 4, 0, evict_clock>()),
		pair<string, Solution *>("256@32@global_round_robin", new SolutionFilteredLinearHash<8, 8, 0, global_round_robin>()),
		pair<string, Solution *>("256@32@approximate_LRU", new SolutionFilteredLinearHash<8, 8, 0, approximate_LRU>()),
		pair<string, Solution *>("256@32@evict_clock", new SolutionFilteredLinearHash<8, 8, 0, evict_clock>()),
		pair<string, Solution *>("256@64@global_round_robin", new SolutionFilteredLinearHash<4, 16, 0, global_round_robin>()),
		pair<string, Solution *>("256@64@approximate_LRU", new SolutionFilteredLinearHash<4, 16, 0, approximate_LRU>()),
		pair<string, Solution *>("256@64@evict_clock", new SolutionFilteredLinearHash<4, 16, 0, evict_clock>()),
		pair<string, Solution *>("256@128@global_round_robin", new SolutionFilteredLinearHash<2, 32, 0, global_round_robin>()),
		pair<string, Solution *>("256@128@approximate_LRU", new SolutionFilteredLinearHash<2, 32, 0, approximate_LRU>()),
		pair<string, Solution *>("256@128@evict_clock", new SolutionFilteredLinearHash<2, 32, 0, evict_clock>()),
		pair<string, Solution *>("1024@4@global_round_robin", new SolutionFilteredLinearHash<256, 1, 0, global_round_robin>()),
		pair<string, Solution *>("1024@4@approximate_LRU", new SolutionFilteredLinearHash<256, 1, 0, approximate_LRU>()),
		pair<string, Solution *>("1024@4@evict_clock", new SolutionFilteredLinearHash<256, 1, 0, evict_clock>()),
		pair<string, Solution *>("1024@8@global_round_robin", new SolutionFilteredLinearHash<128, 2, 0, global_round_robin>()),
		pair<string, Solution *>("1024@8@approximate_LRU", new SolutionFilteredLinearHash<128, 2, 0, approximate_LRU>()),
		pair<string, Solution *>("1024@8@evict_clock", new SolutionFilteredLinearHash<128, 2, 0, evict_clock>()),
		pair<string, Solution *>("1024@16@global_round_robin", new SolutionFilteredLinearHash<64, 4, 0, global_round_robin>()),
		pair<string, Solution *>("1024@16@approximate_LRU", new SolutionFilteredLinearHash<64, 4, 0, approximate_LRU>()),
		pair<string, Solution *>("1024@16@evict_clock", new SolutionFilteredLinearHash<64, 4, 0, evict_clock>()),
		pair<string, Solution *>("1024@32@global_round_robin", new SolutionFilteredLinearHash<32, 8, 0, global_round_robin>()),
		pair<string, Solution *>("1024@32@approximate_LRU", new SolutionFilteredLinearHash<32, 8, 0, approximate_LRU>()),
		pair<string, Solution *>("1024@32@evict_clock", new SolutionFilteredLinearHash<32, 8, 0, evict_clock>()),
		pair<string, Solution *>("1024@64@global_round_robin", new SolutionFilteredLinearHash<16, 16, 0, global_round_robin>()),
		pair<string, Solution *>("1024@64@approximate_LRU", new SolutionFilteredLinearHash<16, 16, 0, approximate_LRU>()),
		pair<string, Solution *>("1024@64@evict_clock", new SolutionFilteredLinearHash<16, 16, 0, evict_clock>()),
		pair<string, Solution *>("1024@128@global_round_robin", new SolutionFilteredLinearHash<8, 32, 0, global_round_robin>()),
		pair<string, Solution *>("1024@128@approximate_LRU", new SolutionFilteredLinearHash<8, 32, 0, approximate_LRU>()),
		pair<string, Solution *>("1024@128@evict_clock", new SolutionFilteredLinearHash<8, 32, 0, evict_clock>()),
		pair<string, Solution *>("4096@4@global_round_robin", new SolutionFilteredLinearHash<1024, 1, 0, global_round_robin>()),
		pair<string, Solution *>("4096@4@approximate_LRU", new SolutionFilteredLinearHash<1024, 1, 0, approximate_LRU>()),
		pair<string, Solution *>("4096@4@evict_clock", new SolutionFilteredLinearHash<1024, 1, 0, evict_clock>()),
		pair<string, Solution *>("4096@8@global_round_robin", new SolutionFilteredLinearHash<512, 2, 0, global_round_robin>()),
		pair<string, Solution *>("4096@8@approximate_LRU", new SolutionFilteredLinearHash<512, 2, 0, approximate_LRU>()),
		pair<string, Solution *>("4096@8@evict_clock", new SolutionFilteredLinearHash<512, 2, 0, evict_clock>()),
		pair<string, Solution *>("4096@16@global_round_robin", new SolutionFilteredLinearHash<256, 4, 0, global_round_robin>()),
		pair<string, Solution *>("4096@16@approximate_LRU", new SolutionFilteredLinearHash<256, 4, 0, approximate_LRU>()),
		pair<string, Solution *>("4096@16@evict_clock", new SolutionFilteredLinearHash<256, 4, 0, evict_clock>()),
		pair<string, Solution *>("4096@32@global_round_robin", new SolutionFilteredLinearHash<128, 8, 0, global_round_robin>()),
		pair<string, Solution *>("4096@32@approximate_LRU", new SolutionFilteredLinearHash<128, 8, 0, approximate_LRU>()),
		pair<string, Solution *>("4096@32@evict_clock", new SolutionFilteredLinearHash<128, 8, 0, evict_clock>()),
		pair<string, Solution *>("4096@64@global_round_robin", new SolutionFilteredLinearHash<64, 16, 0, global_round_robin>()),
		pair<string, Solution *>("4096@64@approximate_LRU", new SolutionFilteredLinearHash<64, 16, 0, approximate_LRU>()),
		pair<string, Solution *>("4096@64@evict_clock", new SolutionFilteredLinearHash<64, 16, 0, evict_clock>()),
		pair<string, Solution *>("4096@128@global_round_robin", new SolutionFilteredLinearHash<32, 32, 0, global_round_robin>()),
		pair<string, Solution *>("4096@128@approximate_LRU", new SolutionFilteredLinearHash<32, 32, 0, approximate_LRU>()),
		pair<string, Solution *>("4096@128@evict_clock", new SolutionFilteredLinearHash<32, 32, 0, evict_clock>()),
		pair<string, Solution *>("16384@4@global_round_robin", new SolutionFilteredLinearHash<4096, 1, 0, global_round_robin>()),
		pair<string, Solution *>("16384@4@approximate_LRU", new SolutionFilteredLinearHash<4096, 1, 0, approximate_LRU>()),
		pair<string, Solution *>("16384@4@evict_clock", new SolutionFilteredLinearHash<4096, 1, 0, evict_clock>()),
		pair<string, Solution *>("16384@8@global_round_robin", new SolutionFilteredLinearHash<2048, 2, 0, global_round_robin>()),
		pair<string, Solution *>("16384@8@approximate_LRU", new SolutionFilteredLinearHash<2048, 2, 0, approximate_LRU>()),
		pair<string, Solution *>("16384@8@evict_clock", new SolutionFilteredLinearHash<2048, 2, 0, evict_clock>()),
		pair<string, Solution *>("16384@16@global_round_robin", new SolutionFilteredLinearHash<1024, 4, 0, global_round_robin>()),
		pair<string, Solution *>("16384@16@approximate_LRU", new SolutionFilteredLinearHash<1024, 4, 0, approximate_LRU>()),
		pair<string, Solution *>("16384@16@evict_clock", new SolutionFilteredLinearHash<1024, 4, 0, evict_clock>()),
		pair<string, Solution *>("16384@32@global_round_robin", new SolutionFilteredLinearHash<512, 8, 0, global_round_robin>()),
		pair<string, Solution *>("16384@32@approximate_LRU", new SolutionFilteredLinearHash<512, 8, 0, approximate_LRU>()),
		pair<string, Solution *>("16384@32@evict_clock", new SolutionFilteredLinearHash<512, 8, 0, evict_clock>()),
		pair<string, Solution *>("16384@64@global_round_robin", new SolutionFilteredLinearHash<256, 16, 0, global_round_robin>()),
		pair<string, Solution *>("16384@64@approximate_LRU", new SolutionFilteredLinearHash<256, 16, 0, approximate_LRU>()),
		pair<string, Solution *>("16384@64@evict_clock", new SolutionFilteredLinearHash<256, 16, 0, evict_clock>()),
		pair<string, Solution *>("16384@128@global_round_robin", new SolutionFilteredLinearHash<128, 32, 0, global_round_robin>()),
		pair<string, Solution *>("16384@128@approximate_LRU", new SolutionFilteredLinearHash<128, 32, 0, approximate_LRU>()),
		pair<string, Solution *>("16384@128@evict_clock", new SolutionFilteredLinearHash<128, 32, 0, evict_clock>()),
		pair<string, Solution *>("65536@4@global_round_robin", new SolutionFilteredLinearHash<16384, 1, 0, global_round_robin>()),
		pair<string, Solution *>("65536@4@approximate_LRU", new SolutionFilteredLinearHash<16384, 1, 0, approximate_LRU>()),
		pair<string, Solution *>("65536@4@evict_clock", new SolutionFilteredLinearHash<16384, 1, 0, evict_clock>()),
		pair<string, Solution *>("65536@8@global_round_robin", new SolutionFilteredLinearHash<8192, 2, 0, global_round_robin>()),
		pair<string, Solution *>("65536@8@approximate_LRU", new SolutionFilteredLinearHash<8192, 2, 0, approximate_LRU>()),
		pair<string, Solution *>("65536@8@evict_clock", new SolutionFilteredLinearHash<8192, 2, 0, evict_clock>()),
		pair<string, Solution *>("65536@16@global_round_robin", new SolutionFilteredLinearHash<4096, 4, 0, global_round_robin>()),
		pair<string, Solution *>("65536@16@approximate_LRU", new SolutionFilteredLinearHash<4096, 4, 0, approximate_LRU>()),
		pair<string, Solution *>("65536@16@evict_clock", new SolutionFilteredLinearHash<4096, 4, 0, evict_clock>()),
		pair<string, Solution *>("65536@32@global_round_robin", new SolutionFilteredLinearHash<2048, 8, 0, global_round_robin>()),
		pair<string, Solution *>("65536@32@approximate_LRU", new SolutionFilteredLinearHash<2048, 8, 0, approximate_LRU>()),
		pair<string, Solution *>("65536@32@evict_clock", new SolutionFilteredLinearHash<2048, 8, 0, evict_clock>()),
		pair<string, Solution *>("65536@64@global_round_robin", new SolutionFilteredLinearHash<1024, 16, 0, global_round_robin>()),
		pair<string, Solution *>("65536@64@approximate_LRU", new SolutionFilteredLinearHash<1024, 16, 0, approximate_LRU>()),
		pair<string, Solution *>("65536@64@evict_clock", new SolutionFilteredLinearHash<1024, 16, 0, evict_clock>()),
		pair<string, Solution *>("65536@128@global_round_robin", new SolutionFilteredLinearHash<512, 32, 0, global_round_robin>()),
		pair<string, Solution *>("65536@128@approximate_LRU", new SolutionFilteredLinearHash<512, 32, 0, approximate_LRU>()),
		pair<string, Solution *>("65536@128@evict_clock", new SolutionFilteredLinearHash<512, 32, 0, evict_clock>()),

	};
	const int case_cum = sizeof(ps) / sizeof(pair<string, Solution *>);

	for (int i = 0; i < case_cum; ++i) {
		run_a_case(ps[i], batch_size, gt, pout, memory);
	}
}

int main()
{
	int batch_size = 2000000;
	FILE *pf = fopen("../kosarak_form.dat", "rb");

	cout << sizeof(Buffer<4>) << endl;
	cout << sizeof(Buffer<3>) << endl;
	cout << sizeof(Buffer<2>) << endl;
	cout << sizeof(Buffer<1>) << endl;

	fseek(pf, 0, SEEK_SET);
	int max_group = 5;

	FILE *pout = fopen("../output.csv", "w");
	//FILE *pout_table_access = fopen("../data/output_ta.csv", "w");

	for (int k = 0; k < max_group; ++k) {
		map<string, int> gt;
		load_data(pf, batch_size, gt);
		exp_var_memory(pout, batch_size, gt);
		//exp_var_queue(pout_table_access, batch_size, gt);
	}

	fclose(pout);
	fclose(pf);

	return 0;
}
