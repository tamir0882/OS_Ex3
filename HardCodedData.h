#ifndef __HARD_CODED_DATA_H__
#define __HARD_CODED_DATA_H__

#define EXPECTED_ARGC 5
#define FAILURE -1
#define SUCCESS 0

#define BUFFER_SIZE_1 1

#define DECIMAL_BASE 10

#define MISSION_FILE_OFFSET 1
#define PRIORITY_FILE_OFFSET 2
#define NUMBER_OF_LINES_OFFSET 3
#define NUMBER_OF_THREADS_OFFSET 4

#define INITIAL_ALLOCATION_SIZE 3

#define DEFAULT_STACK_SIZE 0
#define DEFAULT_CREATION_FLAGS 0

#define FIXED_BUFFER_SIZE 40


#define WAIT_TIME 5000

typedef struct Data
{
	//int mission;
	int index;
	char mission_file_name[_MAX_PATH];
} Data;

typedef struct Element
{
	int index;
	struct Element* next;
} Element;

typedef struct Queue
{
	struct Element* front;
	struct Element* rear;
} Queue;

#endif //__HARD_CODED_DATA_H__