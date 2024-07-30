// matrix-multiplication.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <vector>
#include <chrono>
#include <mpi.h>
#include "Thread_Pool.h"
#include "Mpi_Lib.h"
using namespace std;



//Initialization of the matrixes
int size_v = 1000;
long long* mat1 = new long long[size_v * size_v];
long long* mat2 = new long long[size_v * size_v];
long long* result = new long long[size_v * size_v];
int main(int argc, char** argv)
{
    Mpi_Lib<long long, long long, long long> mpi(argc, argv);
    mpi.init(size_v, size_v);
    int world_rank = mpi.get_world_rank();
    auto sendcounts = mpi.get_sendcounts();
    cout<<"World Rank is: "<<world_rank << endl;
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
    mpi.broadcast(mat2, size_v * size_v, 0, MPI_LONG_LONG);
   
    long long* local_data = new long long[sendcounts[world_rank]];
    fill_n(local_data, sendcounts[world_rank], 0); // Initialize
    long long* local_result = new long long[sendcounts[world_rank]];
    fill_n(local_result, sendcounts[world_rank], 0LL); // Initialize
    //scatter matrix1 to all processors
    mpi.scatterV(mat1, local_data, MPI_LONG_LONG, [local_data, &local_result, world_rank](int start, int end) {
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



    mpi.gather_v(local_result, result,MPI_LONG_LONG,  true);
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
    return 0;
}


