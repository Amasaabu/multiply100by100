// matrix-multiplication.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <vector>
#include <thread>
#include <chrono>
#include <mutex>
#include "Thread_Pool.h"
using namespace std;


//define metrix here
int size_v = 100;
vector<vector<int>> matrix1(size_v, std::vector<int>(size_v));
vector<vector<int>> matrix2(size_v, std::vector<int>(size_v));
vector<vector<long long>> result_matrix_result(size_v, std::vector<long long>(size_v, 0));

//cout mutex lock
mutex lu;

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


void multiply_section(int start_row, int end_row) {
    /*unique_lock<std::mutex> lg(lu);
    cout << "Now starting multiplication" << endl;
    cout << "*****" << this_thread::get_id() << endl;
    lg.unlock();*/
    for (int i = start_row; i < end_row; i++)
    {
        for (int j = 0; j < size_v; j++)
        {
            for (int k = 0; k < size_v; k++)
            {
                //matrix row value
                int matrix_1_row_val = matrix1[i][k];
                int matrix_2_col_val = matrix2[k][j];
                long long r = matrix_1_row_val * matrix_2_col_val;
                result_matrix_result[i][j] = result_matrix_result[i][j] + r;
            }}}}

int main()
{

    //populate the first matrix1
    int first = 0;
    for (int i = 0; i < size_v; ++i) {
        for (int j = 0; j < size_v; ++j) {
            first = first + 1;
            matrix1[i][j] = first;
            matrix2[i][j] = first;
        }
    }
 
    auto start = std::chrono::steady_clock::now();
    /*task();*/
        //5033335000

     Thread_Pool thread_pool(size_v);
     thread_pool.submit(multiply_section);
     thread_pool.shutdown();

    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start);
    cout <<"Time in millsecods: "<< elapsed.count() << endl;
   
    //cout << "Now printing result" << endl;
    //for (int i = 0; i < size_v; i++)
    //{
    //    for (int j = 0; j < size_v;j++) {
    //        cout << result_matrix_result[i][j] << ' ';
    //    }
    //    cout << '\n';
    //}
}



