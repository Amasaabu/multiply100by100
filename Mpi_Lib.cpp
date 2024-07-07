#include "Mpi_Lib.h"
#include "mpi.h"
#include <vector>
#include "Thread_Pool.h"

// Constructor
Mpi_Lib::Mpi_Lib(int& argc, char** argv, int size_v)
{
    // Initialize the MPI environment
    MPI_Init(&argc, &argv);
    // Get the rank of the process
    MPI_Comm_rank(MPI_COMM_WORLD, &this->world_rank);
    //number of processors

    MPI_Comm_size(MPI_COMM_WORLD, &this->world_size);

	//initialize the sendcounts and displs
	this->sendcounts.resize(this->world_size);
	this->displs.resize(this->world_size);

	//Initialize sendcounts and displs
	int remainder = size_v % world_size;

	this->rows_per_proc = size_v / world_size;
	this->extra_rows = size_v % world_size;
	int current_displ = 0;
	for (int i = 0; i < world_size; ++i) {
		// Calculate the number of rows for this process
		int rows_for_this_process = rows_per_proc + (i < extra_rows ? 1 : 0);

		// Calculate the number of elements (rows * columns) for this process
		this->sendcounts[i] = rows_for_this_process * size_v;

		// Set the displacement for this process
		displs[i] = current_displ;

		// Update the displacement for the next process
		current_displ += this->sendcounts[i];
	}
	//this-> local_data = new int[sendcounts[world_rank]];
}

//implement brodcast
void Mpi_Lib::broadcast(int* data, int count, int root)
{
	MPI_Bcast(data, count, MPI_INT, root, MPI_COMM_WORLD);
}
//return world_rank
int Mpi_Lib::get_world_rank() {
	return this->world_rank;
}
//return sendcounts
vector<int> Mpi_Lib::get_sendcounts() {
	return this->sendcounts;
}

//custom Scatterv
//this would distribute the workload to all processors and accross threads with custom Thread_Pool
void Mpi_Lib::scatterV(int* data, int size_v, int* local_data, Funca f) {


	MPI_Scatterv(data, this->sendcounts.data(), displs.data(), MPI_INT, local_data, sendcounts[world_rank], MPI_INT, 0, MPI_COMM_WORLD);
	if (sendcounts[world_rank] == 0) {
		cout << "-------------Process " << world_rank << " received zero data." << endl;
		return;
	}

	//now split accross threads
	int threadPoolSize = sendcounts[world_rank] / size_v;
	Thread_Pool thread_pool(threadPoolSize); //potentially divide by size_v, i could create an overload
	thread_pool.submit(f);
	//ensure all threads are joined
	thread_pool.shutdown();

}

//Custom Gathrev
void Mpi_Lib::gather_v(long long * local_result, int count_of_workload_to_be_distrtibuted,long long *result, int size_v) {
	//gather the results from all processors
	vector<int> sendcounts_gather(this->world_size, 0);
	vector<int> displs_gather(this->world_size, 0);

	int curr_displ = 0;
	for (int i = 0; i < this->world_size; ++i) {
		// Calculate the number of rows this process is responsible for
		int rows_for_this_proc = rows_per_proc + (i < extra_rows ? 1 : 0);

		// Each process sends back rows_for_this_proc * size_v elements
		sendcounts_gather[i] = rows_for_this_proc * size_v;

		// Displacement for this process's data in the gathered array
		displs_gather[i] = curr_displ;

		// Update displacement for the next process
		curr_displ += sendcounts_gather[i];
	}

	MPI_Gatherv(local_result, sendcounts[world_rank], MPI_LONG_LONG, result, sendcounts_gather.data(), displs_gather.data(), MPI_LONG_LONG, 0, MPI_COMM_WORLD);
}

//return pointer to displa
int* Mpi_Lib::get_displs() {
	return displs.data();
}

//return MPI_Barrier
void Mpi_Lib::barrier() {
	MPI_Barrier(MPI_COMM_WORLD);
}

// Destructor
Mpi_Lib::~Mpi_Lib()
{
	// Finalize the MPI environment
	MPI_Finalize();
	//delete local_data
}
