// matrix-multiplication.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <vector>
#include <chrono>
#include <mpi.h>
#include "Thread_Pool.h"
using namespace std;



//Initialization of the matrixes
int size_v = 1000;
long long* mat1 = new long long[size_v * size_v];
long long* mat2 = new long long[size_v * size_v];
long long* result = new long long[size_v * size_v];
int main(int argc, char** argv)
{
    int world_rank;
    int world_size;
    // Initialize the MPI environment
    MPI_Init(&argc, &argv);
    // Get the rank of the process
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    //number of processors

    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    //Ensuring all matrixes are initialized
    fill_n(mat1, size_v*size_v, 0);
    fill_n(mat2, size_v * size_v, 0);
    fill_n(result, size_v * size_v, 0);
    //Populating the matrixes
    if (world_rank == 0) {
        int first = 0;
        for (int i = 0; i < size_v * size_v; ++i) {
            first = first + 1;
            mat1[i] = first;
            mat2[i] = first;

        }
    }

    //mark start of mpi operations
    auto start = std::chrono::steady_clock::now();

    //brodcast matrix2 to all processors
    MPI_Bcast(mat2, size_v * size_v, MPI_LONG_LONG, 0, MPI_COMM_WORLD);

    vector<int> sendcounts;
    vector<int> displs;
    sendcounts.resize(world_size);
    displs.resize(world_size);

    int rows_per_proc = size_v / world_size;


    int extra_rows = size_v % world_size;
    int current_displ = 0;
    int sum = 0;
    for (int i = 0; i < world_size; ++i) {

        // Calculate the number of rows for this process
        int rows_for_this_process = rows_per_proc + (i < extra_rows ? 1 : 0);

        // Calculate the number of elements (rows * columns) for this process
        sendcounts[i] = rows_for_this_process * size_v;
        // Set the displacement for this process
        displs[i] = current_displ;

        // Update the displacement for the next process
        current_displ += sendcounts[i];
    }

    long long* local_data = new long long[sendcounts[world_rank]];
    fill_n(local_data, sendcounts[world_rank], 0LL); 
    long long* local_result = new long long[sendcounts[world_rank]];
    fill_n(local_result, sendcounts[world_rank], 0LL);


    //scatter matrix1 to all processors
    MPI_Scatterv(mat1, sendcounts.data(), displs.data(), MPI_LONG_LONG, local_data, sendcounts[world_rank], MPI_LONG_LONG, 0, MPI_COMM_WORLD);
    if (sendcounts[world_rank] == 0) {
        return 0;
    }
    int threadPoolSize = sendcounts[world_rank] / size_v;
    Thread_Pool thread_pool(threadPoolSize);
    thread_pool.submit([local_data, &local_result, world_rank](int start, int end) {
        for (int i = start; i < end; i++)
        {
            // std::cout << "work_load_per_thread: " <<end-start << std::endl;
            for (int j = 0; j < size_v; j++)
            {
                for (int k = 0; k < size_v; k++)
                {
                    //matrix row value
                    local_result[i * size_v + j] += local_data[i * size_v + k] * mat2[k * size_v + j];
                }
            }
        }
        });
    thread_pool.shutdown();
    //ensure all threads are joined


    vector<int> sendcounts_gather(world_size, 0);
    vector<int> displs_gather(world_size, 0);

    int curr_displ = 0;
    for (int i = 0; i < world_size; ++i) {
        // Calculate the number of rows this process is responsible for
        int rows_for_this_proc = rows_per_proc + (i < extra_rows ? 1 : 0);

        // Each process sends back rows_for_this_proc * size_v elements
        sendcounts_gather[i] = rows_for_this_proc * size_v;

        // Displacement for this process's data in the gathered array
        displs_gather[i] = curr_displ;

        // Update displacement for the next process
        curr_displ += sendcounts_gather[i];
    }
    MPI_Gatherv(local_result, sendcounts_gather[world_rank], MPI_LONG_LONG, result, sendcounts_gather.data(), displs_gather.data(), MPI_LONG_LONG, 0, MPI_COMM_WORLD);
    //mark end of gathering results
    auto end = std::chrono::steady_clock::now();

    // Calculate the duration in milliseconds
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    if (world_rank == 0) {
        for (int i = 0; i < size_v; i++) {
            for (int j = 0; j < size_v; j++) {
                cout << result[i * size_v + j] << ' ';
            }
            cout << '\n';
        }
    }
    if (world_rank == 0) {
        cout << "Duration after matrix multiplication and gathering results: " << duration << " milliseconds" << endl;
    }

    // Finalize the MPI environment.
    delete[] local_result;
    delete[] mat1;
    delete[] mat2;
    delete[] result;
    delete[] local_data;

    MPI_Finalize();
    return 0;
}


