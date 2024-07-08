#pragma once
#include "Mpi_Lib.h"
#include "mpi.h"
#include <vector>
#include "Thread_Pool.h"
using namespace std;
using Funca = std::function<void(int, int)>;

template <class T, class U, class R>
class Mpi_Lib
{
public:
	Mpi_Lib(int&, char**);
	void init(int&, int elements_per_unit = 1);
	void broadcast(U*, int count, int root);
	int get_world_rank();
	vector<int> get_sendcounts();
	void scatterV(T*, T*, Funca f);
	void gather_v(R* local_result, R* result,bool use_element_per_unit_scaling=false);
	//void scatterV(int* data, int count_of_workload_to_be_distrtibuted, int* local_data, Func f);
	int* get_displs();
	void barrier();
	~Mpi_Lib();

private:
	int world_rank;
	int world_size;
	int remainder;
	int size_of_data;
	int elements_per_unit;
	vector<int> sendcounts;
	vector<int> displs;
	//int* local_data;

	int rows_per_proc;
	int extra_rows;
};

// Constructor
template <class T, class U, class R>
Mpi_Lib<T, U, R>::Mpi_Lib(int& argc, char** argv)
{
	// Initialize the MPI environment
	MPI_Init(&argc, &argv);
	// Get the rank of the process
	MPI_Comm_rank(MPI_COMM_WORLD, &this->world_rank);
	//number of processors

	MPI_Comm_size(MPI_COMM_WORLD, &this->world_size);
}

//initialize MPI
template <class T, class U, class R>
void Mpi_Lib<T, U, R>::init(int& size_v, int elements_per_unit) {


	//initialize the sendcounts and displs
	this->sendcounts.resize(this->world_size);
	this->displs.resize(this->world_size);
	this->size_of_data = size_v;
	this->elements_per_unit = elements_per_unit;
	this->rows_per_proc = this->size_of_data / world_size;
	this->extra_rows = this->size_of_data % world_size;
	int current_displ = 0;
	int sum = 0;
	for (int i = 0; i < world_size; ++i) {
		//sendcounts[i] = (numOptions / world_size) * elements_per_unit;
		//if (remainder > 0) {
		//	sendcounts[i] += elements_per_unit;
		//	remainder--;
		//}
		//displs[i] = sum;
		//sum = sum + sendcounts[i];

		// Calculate the number of rows for this process
		int rows_for_this_process = rows_per_proc + (i < extra_rows ? 1 : 0);

		// Calculate the number of elements (rows * columns) for this process
		this->sendcounts[i] = rows_for_this_process * this->elements_per_unit;
		// Set the displacement for this process
		displs[i] = current_displ;

		// Update the displacement for the next process
		current_displ += this->sendcounts[i];
	}
}

//implement brodcast
template <class T, class U, class R>
void Mpi_Lib<T, U, R>::broadcast(U* data_to_brodcast, int count, int root)
{
	MPI_Bcast(data_to_brodcast, count, MPI_INT, root, MPI_COMM_WORLD);
}
//return world_rank
template <class T, class U, class R>
int Mpi_Lib<T, U, R>::get_world_rank() {
	return this->world_rank;
}
//return sendcounts#
template <class T, class U, class R>
vector<int> Mpi_Lib<T, U, R>::get_sendcounts() {
	return this->sendcounts;
}

//custom Scatterv
//this would distribute the workload to all processors and accross threads with custom Thread_Pool
template <class T, class U, class R>
void Mpi_Lib<T, U, R>::scatterV(T* data_to_scatter, T* local_data, Funca f) {


	MPI_Scatterv(data_to_scatter, this->sendcounts.data(), displs.data(), MPI_INT, local_data, sendcounts[world_rank], MPI_INT, 0, MPI_COMM_WORLD);


	//now split accross threads
	int threadPoolSize = sendcounts[world_rank] / this->elements_per_unit;
	cout << "sendcounts[world_rank]: " << sendcounts[world_rank] << endl;
	cout << "size_of_data: " << size_of_data << endl;
	cout << "threadPoolSize: " << threadPoolSize << endl;
	//ensure every processor has work to do
	if (sendcounts[world_rank] == 0) {
		cout << "-------------Process " << world_rank << " received zero data." << endl;
		return;
	}
	Thread_Pool thread_pool(threadPoolSize); //potentially divide by size_v, i could create an overload
	thread_pool.submit(f);
	//ensure all threads are joined
	thread_pool.shutdown();

}

//Custom Gathrev
template <class T, class U, class R>
void Mpi_Lib<T, U, R>::gather_v(R* local_result, R* result, bool use_element_per_unit_scaling) {

	//std::vector<int> recvcounts;
	//std::vector<int> rdispls;
	//	recvcounts.resize(world_size);
	//	rdispls.resize(world_size);
	//	int total_results = this->size_of_data;
	//	int remainder = total_results % world_size;
	//	int sum = 0;
	//	for (int i = 0; i < world_size; i++) {
	//		recvcounts[i] = (total_results / world_size);
	//		if (remainder > 0) {
	//			recvcounts[i]++;
	//			remainder--;
	//		}
	//		rdispls[i] = sum;
	//		sum += recvcounts[i];
	//	}
	//


	//MPI_Gatherv(local_result, this->sendcounts[world_rank] / this->elements_per_unit, MPI_FLOAT,
	//	result, recvcounts.data(), rdispls.data(), MPI_FLOAT,
	//	0, MPI_COMM_WORLD);


	//gather the results from all processors
	vector<int> sendcounts_gather(this->world_size, 0);
	vector<int> displs_gather(this->world_size, 0);

	int curr_displ = 0;
	for (int i = 0; i < this->world_size; ++i) {
		// Calculate the number of rows this process is responsible for
		int rows_for_this_proc = rows_per_proc + (i < extra_rows ? 1 : 0);
		if (use_element_per_unit_scaling) {
			// Each process sends back rows_for_this_proc * size_v elements
			sendcounts_gather[i] = rows_for_this_proc * this->elements_per_unit;
		}
		else {
			sendcounts_gather[i] = rows_for_this_proc;
		}


		// Displacement for this process's data in the gathered array
		displs_gather[i] = curr_displ;

		// Update displacement for the next process
		curr_displ += sendcounts_gather[i];
	}
	cout << "********************" << endl;
	cout << "Processor " << world_rank << " is gathering results" << endl;
	cout << "Processor " << world_rank << "has " << this->sendcounts[world_rank] << " elements" << endl;
	MPI_Gatherv(local_result, sendcounts_gather[world_rank], MPI_LONG_LONG, result, sendcounts_gather.data(), displs_gather.data(), MPI_LONG_LONG, 0, MPI_COMM_WORLD);
	cout << "Processor " << world_rank << " has finished work" << endl;
}

//return pointer to displa
template <class T, class U, class R>
int* Mpi_Lib<T, U, R>::get_displs() {
	return displs.data();
}

//return MPI_Barrier
template <class T, class U, class R>
void Mpi_Lib<T, U, R>::barrier() {
	MPI_Barrier(MPI_COMM_WORLD);
}

// Destructor
template <class T, class U, class R>
Mpi_Lib<T, U, R>::~Mpi_Lib()
{
	// Finalize the MPI environment
	MPI_Finalize();
	//delete local_data
}
