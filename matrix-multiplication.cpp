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

int size_v = 100;
int* mat1 = new int[size_v * size_v];
int * mat2 = new int[size_v * size_v];
long long* result = new long long[size_v * size_v];
int main(int argc, char** argv)
{
    // Initialize the MPI environment
    MPI_Init(&argc, &argv);
    // Get the rank of the process
    int world_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    //number of processors
    int world_size;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    //Populating the matrixes
    if (world_rank == 0) {
        int first = 0;
        for (int i = 0; i < size_v*size_v; ++i) {
            first = first + 1;
                mat1[i] = first;
                mat2[i] = first;
            
        }
	}
    MPI_Bcast(&size_v, 1, MPI_INT, 0, MPI_COMM_WORLD);
 

    /*task();*/
        //5033335000
        
       
     //each processor will get a row of matrix1
     vector<int> sendcounts(world_size);
     vector<int> displs(world_size);
     int remainder = size_v % world_size;

    int rows_per_proc = size_v / world_size;
    int extra_rows = size_v % world_size;
    int current_displ = 0;
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
    int * local_data = new int[sendcounts[world_rank]];
     //scatter row vectors of matrix1 to all processors by scattering pointers of each row in the matrix hence matrix1.data()->data()
     MPI_Scatterv(mat1, sendcounts.data(), displs.data(), MPI_INT, local_data, sendcounts[world_rank] , MPI_INT, 0, MPI_COMM_WORLD);
 
     //cout << "Process " << world_rank << " has received the matrix1bbb data: " << *local_data+5 << endl;
    
    //brodcast matrix2 to all processors
     MPI_Bcast(mat2, size_v * size_v, MPI_INT, 0, MPI_COMM_WORLD);


    //cout << "Process " << world_rank << " has received the matrix2 data: " << matrix2[0][0] << endl;
  //   multiply the matrices

      long long* local_result = new long long[sendcounts[world_rank]];
      fill_n(local_result, sendcounts[world_rank], 0LL); // Initialize
      cout << "Process has received: " << sendcounts[world_rank] << endl;
 //    Launch threads based on local assembled matrix 1 size

     auto start = std::chrono::steady_clock::now();
     Thread_Pool thread_pool(sendcounts[world_rank]/size_v);
     thread_pool.submit([local_data, &local_result, world_rank](int start, int end) {
         for (int i = start; i < end; i++)
         {
             for (int j = 0; j < size_v; j++)
             {
                 for (int k = 0; k < size_v; k++)
                 {
                     //matrix row value
                     local_result[i * size_v + j] += local_data[i * size_v + k] * mat2[k * size_v + j];
                 }
                // cout << "Sums are: " << local_result[0][0] << endl;
             }
         }
	 });
     thread_pool.shutdown();



   
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

     //now gather results from flattened_local_result to gathered_results
     //Note that we are gathering the flattened_local_result so we need not re-calculat the displacements and sendcounts
     MPI_Gatherv(local_result,sendcounts[world_rank], MPI_LONG_LONG, result, sendcounts_gather.data(), displs_gather.data(), MPI_LONG_LONG, 0, MPI_COMM_WORLD);

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
     MPI_Finalize();
    return 0;
}



