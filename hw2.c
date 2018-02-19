/***	Yoel Elias
	*	CSC4320 Homework2
	*	Multithreaded Sudoku Solution Checker
	*	This program will use 11 threads to check if the inputted 9x9 sudoku grid has been solved correctly.
	*	A Sudoku grid is solved when every row, column, and 3x3 grid in the 9x9 grid contain the numbers 1-9.
	*	This program will print out the grid and a message stating whether the grid is solved correctly or not.
***/


#include <pthread.h>	//	pthreads.h is included to be able to multi-thread
#include <stdio.h>	//	stdio.h is used for standard i/o
#include <stdlib.h>	//	stdlib.h is used for memory allocation
#include <ctype.h>	//	ctype.h is used for the isspace() function

/***	MACRO Definitions
	*	-----------------
	*	SIZE is defined as 9 because the standard sudoku grid size is 9x9
	*		Used when declaring the global int grid[][]
	*	-----------------
	*	TOTAL_SUM is defined as 45 because it is the sum of numbers from 1-9
	*		Used to check for Sudoku solution. We know that every row, column, and 3x3 has to be 1-9.
	*		Then if we run a sum of the input grid's rows, columns, 3x3's each individual sum should be 45.
	*		if it is not then it is not a valid Sudoku Solution
	*		Obviously there are other combinations of numbers that will give 45 when added. So this program
	*		is assuming the user is following the Sudoku rules and code of ethics.
	*	----------------
	*	MAX_THREADS is defined as 11 because it is the project's requirement to use 11 threads.
	*		Used when declaring the size of pthread_t id[].
	*		Also used when declaring the size int solver[]. Each element of solver corresponds to a created thread.
	*		Created threads will store 1 to solver[] if the sum of (row/col/3x3) == TOTAL_SUM
	*		and store 0 if the sum != TOTAL_SUM.
***/

#define SIZE 9
#define TOTAL_SUM 45
#define MAX_THREADS 11

pthread_t id[MAX_THREADS];

//	grid and solver are global variables so that threads can access them
int grid[SIZE][SIZE];

int solver[MAX_THREADS];


typedef struct	//	Used to pass information to threads on where they must start and where to store information
{
	int row;		//	Row and Col give the 3x3 workers a place to start searching in 9x9 matrix
	int col;
	int id;		// id is used to keep track of where each thread should store their outcome in solver[]
} parameters;

/***
	*	Function: gridInput
	*	-------------------
	*	The objective of this function is to take the commandline input grid and create a 9x9 matrix
	*		Fills in grid[][] using fgetc() and checking to not add a space or EOF to the grid
	*
	*	*finput: Input file pointer passed by command line. Must be a 9x9 grid file
	*
	*	Return:	Nothing
***/
void gridInput(FILE *finput)
{
	char c;
	int row, col, i;
	row = 0;
	col = 0;
	while (row < SIZE)
	{
		while (col < SIZE)
		{
			c = fgetc(finput);
			if (!isspace(c) && c != EOF) {
				i = c - '0';	//	This line is used to convert string number to it's corresponding int
				grid[row][col] = i;
				col++;		//	Only goes to the next column if an addition to grid[][] was made
			}
		}
		row++;
		col = 0;	//	Resets col to zero once edge of matrix is reached
	}
}

/***
	*	Function: column_worker
	*	-----------------------
	*	The objective of this function is for the column thread to sum each and every column in grid matrix
	*		After each column is added, the sum is compared to TOTAL_SUM
	*
	*	*param:	A struct is passed as input to keep track of the id
	*
	*	Return: Nothing
***/
void *column_worker(void *param)
{
	parameters *data;
	int row, col, sum, id;
	data = (parameters*) param;
	id = (int) data->id;
	sum = 0;

	for (col = (int) data->col; col < SIZE; col++)
	{
		for (row = (int) data->row; row < SIZE; row++)
		{
			sum += grid[row][col];
		}
		if (sum != TOTAL_SUM) {
			solver[id] = 0;
			pthread_exit(NULL);
		}
		sum = 0;
	}
	solver[id] = 1;

	free(param);
	pthread_exit(NULL);
}

/***
	*	Function: row_worker
	*	---------------------
	*	The objective of this function is for the row thread to sum each and every row in grid[][]
	*		After each individual row is added, the sum is compared to TOTAL_SUM
	*
	*	*param:  A struct is passed as input to keep track of the id
	*
	*	Return: Nothing
***/
void *row_worker(void *param)
{
	parameters *data;
	int row, col, sum, id;

	data = (parameters*) param;
	id = (int) data->id;
	sum = 0;

	for (row = (int) data->row; row < SIZE; row++)
	{
		for (col = (int) data->col; col < SIZE; col++)
		{
			sum += grid[row][col];
		}
		if (sum != TOTAL_SUM) {
			solver[id] = 0;
			pthread_exit(NULL);
		}
		sum = 0;
	}
	solver[id] = 1;
	free(param);
	pthread_exit(NULL);
}

/***
	*	Function: _3x3_worker
	*	----------------------
	*	The objective of this function is for the 3x3 threads to check if each 3x3 in the 9x9 grid[][] is equal to TOTAL_SUM
	*
	*	*param: A struct is passed to keep track of id, starting row and starting column
	*
	*	Return: Nothing
***/
void *_3x3_worker(void *param)
{
	parameters *data;
	//	row_end and col_end are used to know where the 3x3 matrix ends
	int row, col, sum, row_end, col_end, id;

	data = (parameters*) param;
	row = (int) data->row;
	col = (int) data->col;
	id = (int) data->id;
	//	By adding 3 to passed starting row and col we can get the desired 3x3 matrix
	row_end = row + 3;
	col_end = col + 3;
	sum = 0;

	for (row = (int) data->row; row < row_end; row++)
	{
		for (col = (int) data->col; col < col_end; col++)
		{
			sum += grid[row][col];
		}
	}
	if (sum != TOTAL_SUM) {
		solver[id] = 0;
		pthread_exit(NULL);
	}
	solver[id] = 1;
	free(param);
	pthread_exit(NULL);
}

int main(int argc, char *argv[])
{
	FILE *finput;
	parameters *test, *test1, *test2;
	int row, col, i, err;

	if (argc == 1) {
		fprintf(stderr, "Usage: ./hw2 <file containing 9x9 solved Sudoku grid>\n");
		return(1);
	}

	if (argc > 2) {
		fprintf(stderr, "Usage: ./hw2 <SINGLE FILE containing 9x9 solved Sudoku grid>\n");
		return(1);
	}

	finput = fopen(argv[1], "r");
	if (finput == NULL) {
		fprintf(stderr, "Error opening file.\n");
		return(1);
	}

	gridInput(finput);

	fclose(finput);

	/*	Creating of our 11 threads
		the first two threads are the column and row threads respectively
		the rest are the 3x3 workers
		A parameter is allocated memory and given starting values for row, col, and id
	*/
	for (i = 0; i < 11; i++)
	{
		if (i == 0) {
			test = (parameters*) malloc(sizeof(parameters));
			test->row = 0;
			test->col = 0;
			test->id = i;
			err = pthread_create(&(id[i]), NULL, &column_worker, (void *) test);
			if (err != 0) {
				fprintf(stderr, "Cannot create column thread worker\n");
				free(test);
			}
		}
		else if (i == 1) {
			test1 = (parameters*) malloc(sizeof(parameters));
			test1->row = 0;
			test1->col = 0;
			test1->id = 1;
			err = pthread_create(&(id[i]), NULL, &row_worker, (void *) test1);
			if (err != 0) {
				fprintf(stderr, "Cannot create row thread worker\n");
				free(test1);
			}
		}
		else {
			test2 = (parameters *) malloc(sizeof(parameters));
			test2->row = row;
			test2->col = col;
			test2->id = i;
			err = pthread_create(&(id[i]), NULL, &_3x3_worker, (void *) test2);
			if (err != 0) {
				fprintf(stderr, "Error creating 3x3 thread worker\n");
				free(test2);
			}
			/*	To give the 3x3 threads the correct starting place we add 3 to col
				Once col hits 9 (edge of matrix) we reset col to 0 and add 3 to row
				Should give a staring point for every 3x3 thread in the 9x9 grid
			*/
			col += 3;
			if (col == 9) {
				col = 0;
				row += 3;
			}
		}
	}

	//	Tell parent thread to wait for children
	pthread_join(id[0], NULL);
	pthread_join(id[1], NULL);
	pthread_join(id[2], NULL);
	pthread_join(id[3], NULL);
	pthread_join(id[4], NULL);
	pthread_join(id[5], NULL);
	pthread_join(id[6], NULL);
	pthread_join(id[7], NULL);
	pthread_join(id[8], NULL);
	pthread_join(id[9], NULL);
	pthread_join(id[10], NULL);

	//	Prints Matrix just for clarity
	for (row = 0; row < SIZE; row++)
	{
		printf("{");
		for (col = 0; col < SIZE; col++)
		{
			printf("%d ", grid[row][col]);
		}
		printf("\b};\n");
	}

	/*	Loops over solver to check if the grid is solved correctly or not
		If all values of solver are 1 then it is solved correctly
	*/
	for (i = 0; i < MAX_THREADS; i++) {
		if (solver[i] == 0) {
			printf("Input grid is not a valid Sudoku solution\n");
			return(0);
		}
	}

	printf("Input grid is a valid Sudoku Solution\n");
	printf("Good-Bye!\n");

	return(0);
}
