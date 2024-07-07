// matrix-multiplication.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <vector>
#include <thread>
#include <chrono>
#include <mutex>
#include <mpi.h>
#include "Thread_Pool.h"
#include <vector>
#include "Mpi_Lib.h"
using namespace std;


//void task() {
//    for (int i = 0; i < size_v; i++)
//    {
//        for (int j = 0; j < size_v; j++)
//        {
//            for (int k = 0; k < size_v; k++)
//            {
//                //matrix row value
//                int matrix_1_row_val = matrix1[i][k];
//                int matrix_2_col_val = matrix2[k][j];
//                long long r = matrix_1_row_val * matrix_2_col_val;
//                result_matrix_result[i][j] = result_matrix_result[i][j] + r;
//
//            }
//
//        }
//    }
//}


//void multiply_section(int start_row, int end_row) {
//    /*unique_lock<std::mutex> lg(lu);
//    cout << "Now starting multiplication" << endl;
//    cout << "*****" << this_thread::get_id() << endl;
//    lg.unlock();*/
//    for (int i = start_row; i < end_row; i++)
//    {
//        for (int j = 0; j < size_v; j++)
//        {
//            for (int k = 0; k < size_v; k++)
//            {
//                //matrix row value
//                int matrix_1_row_val = matrix1[i][k];
//                int matrix_2_col_val = matrix2[k][j];
//                long long r = matrix_1_row_val * matrix_2_col_val;
//                result_matrix_result[i][j] = result_matrix_result[i][j] + r;
//            }}}}
        //define metrix here
//mutex 

int size_v = 5;
int* mat1 = new int[size_v * size_v];
int * mat2 = new int[size_v * size_v];
long long* result = new long long[size_v * size_v];
int main(int argc, char** argv)
{
   Mpi_Lib mpi(argc, argv, size_v);
   int world_rank = mpi.get_world_rank();
   auto sendcounts = mpi.get_sendcounts();
    //Populating the matrixes
    if (world_rank == 0) {
        int first = 0;
        for (int i = 0; i < size_v*size_v; ++i) {
            first = first + 1;
                mat1[i] = first;
                mat2[i] = first;
            
        }
	}
    //mpi.barrier();

    /*task();*/
        //5033335000
        
    // Mark the start time
    auto start = std::chrono::steady_clock::now();


    //brodcast matrix2 to all processors
    mpi.broadcast(mat2, size_v * size_v, 0);

    int* local_data = new int[sendcounts[world_rank]];
    fill_n(local_data, sendcounts[world_rank], 0); // Initialize
    long long* local_result = new long long[sendcounts[world_rank]];
    fill_n(local_result, sendcounts[world_rank], 0LL); // Initialize
    //scatter matrix1 to all processors
    mpi.scatterV(mat1, size_v, local_data, [local_data, &local_result, world_rank](int start, int end) {
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
 
     //cout << "Process " << world_rank << " has received the matrix1bbb data: " << *local_data+5 << endl;
    

  //   //ensure every processor has some work to do
  //   if (sendcounts[world_rank] == 0) {
  //       cout<<"Processor "<<world_rank<<" has no work to do"<<endl;
		// MPI_Finalize();
		// return 0;
	 //}

    //cout << "Process " << world_rank << " has received the matrix2 data: " << matrix2[0][0] << endl;
  //   multiply the matrices

   
     mpi.gather_v(local_result, sendcounts[world_rank], result, size_v);
     if (world_rank == 0) {
         for (int i = 0; i < size_v; i++) {
             for (int j = 0; j < size_v; j++) {
                 cout << result[i * size_v + j] << ' ';
             }
             cout << '\n';
         }
     }

     // Mark the end time after gathering the results
     auto end = std::chrono::steady_clock::now();

     // Calculate the duration in milliseconds
     auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
     cout << "Duration is " << duration<<endl;
	 // Finalize the MPI environment.
     delete [] local_result;
     delete [] mat1;
     delete [] mat2;
     delete [] result;
     delete[] local_data;
    return 0;
}



