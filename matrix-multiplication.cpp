// matrix-multiplication.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include "threadpool.h"
using namespace std;


//define metrix here
int size_v = 4;
vector<vector<int>> matrix1(size_v, std::vector<int>(size_v));
vector<vector<int>> matrix2(size_v, std::vector<int>(size_v));
vector<vector<long long>> result_matrix_result(size_v, std::vector<long long>(size_v, 0));

//cout mutex lock
mutex lu;

void task() {
    for (int i = 0; i < size_v; i++)
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

            }

        }
    }
}


void multiply_section(int start_row, int end_row) {
    unique_lock<std::mutex> lg(lu);
    cout << "Now starting multiplication" << endl;
    cout << "*****" << this_thread::get_id() << endl;
    lg.unlock();
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

            }

        }
    }
}


int main()
{

    //populate the first matrix1
    int first = 0;
    for (int i = 0; i < size_v; ++i) {
        for (int j = 0; j < size_v; ++j) {
            first = first + 1;
            matrix1[i][j] = first;
        }
    }
    //populate the first matrix2
    int second = 0;
    for (int i = 0; i < size_v; ++i) {
        for (int j = 0; j < size_v; ++j) {
            second = second + 1;
            matrix2[i][j] = second;
        }
    }

    const unsigned thread_count = thread::hardware_concurrency();
    vector<thread> threads;
    int rows_per_thread = size_v / thread_count;

  //  task();
    //5033335000

    for (int t = 0; t < thread_count; t++) {
        int start_row = t * rows_per_thread;
        int end_row = (t == thread_count - 1) ? size_v : start_row + rows_per_thread;
        threads.push_back(thread(multiply_section, start_row, end_row));
    }
    for (auto& th : threads) {
        th.join();
    }

   
    cout << "Now printing result" << endl;
    for (int i = 0; i < size_v; i++)
    {
        for (int j = 0; j < size_v;j++) {
            cout << result_matrix_result[i][j] << ' ';
        }
        cout << '\n';
    }
    //cout << "Number of threads that worked: " << thread_count<<endl;
}