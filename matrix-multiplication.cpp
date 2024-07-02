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
int size_v = 100;
vector<vector<int>> matrix1(size_v, std::vector<int>(size_v));
vector<vector<int>> matrix2(size_v, std::vector<int>(size_v));
vector<vector<long long>> result_matrix_result(size_v, std::vector<long long>(size_v, 0));
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
        for (int i = 0; i < size_v; ++i) {
            for (int j = 0; j < size_v; ++j) {
                first = first + 1;
                matrix1[i][j] = first;
                matrix2[i][j] = first;
            }
        }
	}
    MPI_Bcast(&size_v, 1, MPI_INT, 0, MPI_COMM_WORLD);
 

    /*task();*/
        //5033335000
        
       
    //flatten matrix1
    vector<int> flattened_matrix1;
    for (int i = 0; i < size_v; i++) {
        for (int j = 0; j < size_v; j++) {
            flattened_matrix1.push_back(matrix1[i][j]);
        }
    }
     //each processor will get a row of matrix1
     vector<int> sendcounts(world_size);
     vector<int> displs(world_size);
     int remainder = size_v % world_size;

    int rows_per_proc = size_v / world_size;
    int extra_rows = size_v % world_size;
    for (int i = 0; i < world_size; i++) {
       sendcounts[i] = rows_per_proc * size_v; // Each processor's share of rows, times columns
       if (i < extra_rows) sendcounts[i] += size_v; // Distribute extra rows
       displs[i] = (i > 0) ? (displs[i - 1] + sendcounts[i - 1]) : 0;

    }
     vector<int> local_flat_data(sendcounts[world_rank]);
    

     //scatter row vectors of matrix1 to all processors by scattering pointers of each row in the matrix hence matrix1.data()->data()
     MPI_Scatterv(flattened_matrix1.data(), sendcounts.data(), displs.data(), MPI_INT, local_flat_data.data(), sendcounts[world_rank] , MPI_INT, 0, MPI_COMM_WORLD);
     
    //brodcast matrix2 to all processors
     for (int i = 0; i < size_v; ++i) {
         MPI_Bcast(matrix2[i].data(), size_v, MPI_INT, 0, MPI_COMM_WORLD);
     }

     //assemble the local-matrix for easy calculation....Local matrix would hold vectors of what each processor got..And each of those vectors would hold 100(size_v)
     vector<vector<int>> local_matrix1(sendcounts[world_rank]/size_v, vector<int>(size_v));
    // i is the row
     for (int i = 0; i < sendcounts[world_rank]/size_v; ++i) {
         for (int j = 0; j < size_v; ++j) {
             //i*size_v is the row since we know that i*size_v would give us the row
             local_matrix1[i][j] = local_flat_data[i * size_v + j];
         }
     }
   //  cout << "Process " << world_rank << " has received the matrix2 data: " << matrix2[0][0] << endl;
  //   multiply the matrices
     vector<vector<long long>> local_result(sendcounts[world_rank] / size_v, vector<long long>(size_v, 0));
 //    Launch threads based on local assembled matrix 1 size

     auto start = std::chrono::steady_clock::now();
     Thread_Pool thread_pool(sendcounts[world_rank] / size_v);
     thread_pool.submit([local_matrix1, &local_result, world_rank](int start, int end) {
         for (int i = start; i < end; i++)
         {
             for (int j = 0; j < size_v; j++)
             {
                 for (int k = 0; k < size_v; k++)
                 {
                     //matrix row value
                     local_result[i][j] += local_matrix1[i][k] * matrix2[k][j];
                 }
                // cout << "Sums are: " << local_result[0][0] << endl;
             }
         }
	 });
     thread_pool.shutdown();


  //   flatten local_result on each process and thereby preoare for assembling by gatherv
     vector<long long> flattened_local_result;
     for (int i = 0; i < local_result.size(); i++) {
         for (int j = 0; j < size_v; j++) {
             flattened_local_result.push_back(local_result[i][j]);
         }
     }



     // Prepare a vector to receive the gathered results on the root process
     vector<long long> gathered_results;
     if (world_rank == 0) {
         gathered_results.resize(size_v * size_v); // Adjust size accordingly
     }
     vector<int> sendcounts_gather(world_size, 0);
     vector<int> displs_gather(world_size, 0);

     int current_displ = 0;
     for (int i = 0; i < world_size; ++i) {
         // Calculate the number of rows this process is responsible for
         int rows_for_this_proc = rows_per_proc + (i < extra_rows ? 1 : 0);

         // Each process sends back rows_for_this_proc * size_v elements
         sendcounts_gather[i] = rows_for_this_proc * size_v;

         // Displacement for this process's data in the gathered array
         displs_gather[i] = current_displ;

         // Update displacement for the next process
         current_displ += sendcounts_gather[i];
     }

     //now gather results from flattened_local_result to gathered_results
     //Note that we are gathering the flattened_local_result so we need not re-calculat the displacements and sendcounts
     MPI_Gatherv(flattened_local_result.data(), flattened_local_result.size(), MPI_LONG_LONG, gathered_results.data(), sendcounts_gather.data(), displs_gather.data(), MPI_LONG_LONG, 0, MPI_COMM_WORLD);

     //because we scattered the local result we need to assemble it back to the original result  matrix 
     //overall reuslt
     vector<vector<long long>> overall_result(size_v, vector<long long>(size_v, 0));
     if (world_rank==0) {
         //i is the row
		 for (int i = 0; i < size_v; i++) {
			 for (int j = 0; j < size_v; j++) {
				 overall_result[i][j] = gathered_results[i * size_v + j];
			 }
		 }
	 }
     // Mark the end time after gathering the results
     auto end = std::chrono::steady_clock::now();

     // Calculate the duration in milliseconds
     auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
     cout << "Duration is " << duration<<endl;
  //   //process 0 prints the result
     if (world_rank == 0) {
         cout << "Now printing result" << endl;
         for (int i = 0; i < size_v; i++)
         {
             for (int j = 0; j < size_v; j++) {
                 cout << overall_result[i][j] << ' ';
             }
             cout << '\n';
         }
     }
 
	 // Finalize the MPI environment.
    MPI_Finalize();
    return 0;
}



