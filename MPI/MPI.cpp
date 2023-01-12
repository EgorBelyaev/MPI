#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <time.h>
#include <Windows.h>
#include "mpi.h"
#include <clocale>
#include <iostream>
# define K 1500
using namespace std;


void createMatrix(int* A, int* B, int N) {
	int i, j;
	for (i = 0; i < N; i++)
		for (j = 0; j < N; j++) {
			A[i * N + j] = rand() % 10;
			B[i * N + j] = rand() % 10;
		}
}

void printMatrix(int matrix[],int N)
{
	for (int i = 0; i < N; i++) {
		for (int j = 0; j < N; j++) {
			printf(" %d", matrix[i * N + j]);
		}
		printf("\n");
	}
}


void initializeMatrix(int*& A, int*& B, int*& C, int*& result, int& N) {

	A = new int[N * N];
	B = new int[N * N];
	C = new int[N * N];
	result = new int[N * N];

	createMatrix(A, B, N);
	for (int i = 0; i < N * N; i++) {
		C[i] = 0;
		result[i] = 0;
	}
}

void multiplicationMatrix(int* A, int* B, int* C, int N, int nCommRunk, int nCommSize) {
	int i, j, k;
	for (i = 0; i < N; i++) {
		for (j = 0 + nCommRunk; j < N; j += nCommSize) {
			for (k = 0; k < N; k++) {
				C[i * N + j] += A[i * N + k] * B[k * N + j];
			}
		}
	}
}


int main(int argc, char* argv[]) {
	int* A;
	int* B;
	int* C;
	int* result = 0;
	int N = K;
	int nCommRunk, nCommSize, namelen, nCounter;
	int nIntervals;
	char processor_name[MPI_MAX_PROCESSOR_NAME];
	double startTime, endTime;

	MPI_Status status;
	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &nCommRunk);
	MPI_Comm_size(MPI_COMM_WORLD, &nCommSize);
	MPI_Get_processor_name(processor_name, &namelen);

	if (nCommRunk == 0) {
		initializeMatrix(A, B, C, result, N);
		startTime = MPI_Wtime();
		printf("multiplication of matrix with dimension %d, with MPI \n", N);
		for (nCounter = 1; nCounter < nCommSize; nCounter++) {
			MPI_Send(&N, 1, MPI_INT, nCounter, 0, MPI_COMM_WORLD);
			MPI_Send(A, N * N, MPI_INT, nCounter, 1, MPI_COMM_WORLD);
			MPI_Send(B, N * N, MPI_INT, nCounter, 2, MPI_COMM_WORLD);
			MPI_Send(C, N * N, MPI_INT, nCounter, 3, MPI_COMM_WORLD);
		}
	}
	else {
		MPI_Recv(&N, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		A = new int[N * N];
		B = new int[N * N];
		C = new int[N * N];
		MPI_Recv(A, N * N, MPI_INT, 0, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		MPI_Recv(B, N * N, MPI_INT, 0, 2, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		MPI_Recv(C, N * N, MPI_INT, 0, 3, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
	}

	multiplicationMatrix(A, B, C, N, nCommRunk, nCommSize);
	MPI_Reduce(C, result, N * N, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

	if (nCommRunk == 0) {
		endTime = MPI_Wtime();
		printf("Time: %f", endTime - startTime);
	}
	MPI_Finalize();
}