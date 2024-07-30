// matrix-multiplication.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <chrono>
using namespace std;



//Initialization of the matrixes
int size_v = 1000;
long long* mat1 = new long long[size_v * size_v];
long long* mat2 = new long long[size_v * size_v];
long long* result = new long long[size_v * size_v];
int main(int argc, char** argv)
{
    //initialize matrixes with 0
    fill_n(mat1, size_v * size_v, 0);
    fill_n(mat2, size_v * size_v, 0);
    fill_n(result, size_v * size_v, 0);
    //Populating the matrixes
        int first = 0;
        for (int i = 0; i < size_v * size_v; ++i) {
            first = first + 1;
            mat1[i] = first;
            mat2[i] = first;

        }

    auto start = std::chrono::steady_clock::now();
    for (int i = 0; i < size_v; i++)
    {
        for (int j = 0; j < size_v; j++)
        {
            for (int k = 0; k < size_v; k++)
            {
                    //matrix row value
                    result[i * size_v + j] += mat1[i * size_v + k] * mat2[k * size_v + j];
            }
        }
    }
    auto end = std::chrono::steady_clock::now();
    // Calculate the duration in milliseconds
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    //Printing the output
    for (int i = 0; i < size_v; i++) {
            for (int j = 0; j < size_v; j++) {
                cout << result[i * size_v + j] << ' ';
            }
            cout << '\n';
        }
    
    cout << "Time to complete matrix multiplication " << duration << endl;
    delete[] mat1;
    delete[] mat2;
    delete[] result;
    return 0;
}


