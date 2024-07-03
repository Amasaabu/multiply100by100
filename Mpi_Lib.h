#pragma once
#include "Mpi_Lib.h"
#include "mpi.h"
#include <vector>
#include "Thread_Pool.h"
using namespace std;
using Funca = std::function<void(int, int)>;
class Mpi_Lib
{
public:
	Mpi_Lib(int&, char**, int);
	void broadcast(int* data, int count, int root);
	int get_world_rank();
	vector<int> get_sendcounts();
	void scatterV(int* data, int size_v,int* , Funca f);
	void gather_v(long long* local_result, int count_of_workload_to_be_distrtibuted, long long* result, int size_v);
	//void scatterV(int* data, int count_of_workload_to_be_distrtibuted, int* local_data, Func f);
	int* get_displs();
	void barrier();
	~Mpi_Lib();

private:
	int world_rank;
	int world_size;
	int remainder;
	vector<int> sendcounts;
	vector<int> displs;
	//int* local_data;

	int rows_per_proc;
	int extra_rows;
};

