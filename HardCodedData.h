#ifndef __HARD_CODED_DATA_H__
#define __HARD_CODED_DATA_H__
#include <Windows.h>

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
#define WHITESPACE_COMMA_FACTOR 3


#define DEFAULT_STACK_SIZE 0
#define DEFAULT_CREATION_FLAGS 0

#define FIXED_BUFFER_SIZE 50

#define INITIAL_SEMAPHORE_COUNT 1

#define WAIT_TIME 5000000


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


typedef struct Lock
{
	int readers;
	HANDLE h_mutex;
	HANDLE h_room_empty;
	HANDLE h_turnstile;

} Lock;


typedef struct Data
{
	Queue* q;
	Lock* lock;
	HANDLE h_resource_mutex;
	HANDLE h_priority_file;
	char mission_file_name[_MAX_PATH];
	int number_of_missions;
	int next_line_offset;
	int next_line_index;
} Data;


#endif //__HARD_CODED_DATA_H__