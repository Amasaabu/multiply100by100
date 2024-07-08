#pragma once
#include "mpi.h"


// General template declaration (undefined)
template<typename T>
MPI_Datatype get_mpi_type();

template<>
inline MPI_Datatype get_mpi_type<int>() {
	return MPI_INT;
}

template<>
inline MPI_Datatype get_mpi_type<long long>() {
	return MPI_LONG_LONG;
}

template<>
inline MPI_Datatype get_mpi_type<float>() {
	return MPI_FLOAT;
}

template<>
inline MPI_Datatype get_mpi_type<double>() {
	return MPI_DOUBLE;
}

template<>
inline MPI_Datatype get_mpi_type<char>() {
	return MPI_BYTE;
}

template<>
inline MPI_Datatype get_mpi_type<unsigned char>() {
	return MPI_BYTE;
}