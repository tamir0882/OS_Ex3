#include <stdio.h>
#include <string.h>
#include <windows.h>


#include "HardCodedData.h"
#include "utility_functions.h"
#include "Thread_operation.h"
#include "Queue.h"
#include "Lock.h"


int main(int argc, char** argv)
{
	
	int exit_code = SUCCESS;
	int ret_val = 0;
	int wait_code = 0;

	char mission_file_name[_MAX_PATH] = { 0 };
	char priority_file_name[_MAX_PATH] = { 0 };
	HANDLE h_priority_file = NULL;
	int number_of_missions = 0;
	int number_of_threads = 0;


	Queue* q = NULL;
	Lock* lock = NULL;
	HANDLE h_q_mutex = NULL;
	HANDLE h_thread = NULL;
	Data* p_threads_data = NULL;


	exit_code = initialize_main_thread(argc, argv, mission_file_name, priority_file_name, &h_priority_file, &number_of_missions, &number_of_threads);
	if (FAILURE == exit_code)
	{
		printf("main: initialize_main_thread failed.\n");
		return FAILURE;
	}


	q = create_queue(h_priority_file, number_of_missions);
	if (NULL == q)
	{
		printf("main: create_queue failed.\n");
		exit_code = FAILURE;
		goto Resource_Handling;
	}

	
	lock = InitializeLock();
	if (NULL == lock)
	{
		printf("main: InitializeLock failed.\n");
		exit_code = FAILURE;
		goto Resource_Handling;
	}


	h_q_mutex = CreateMutexA(NULL, FALSE, NULL);
	if (NULL == h_q_mutex)
	{
		printf("main: CreateMutexA failed.\n");
		exit_code = FAILURE;
		goto Resource_Handling;
	}

	//p_threads_data = initialize_threads_data();


	Data thread_data = { .q = q,.lock = lock, .h_q_mutex = h_q_mutex,
		.number_of_missions = number_of_missions, .mission_file_name = NULL };

	DWORD thread_ids[MAXIMUM_WAIT_OBJECTS] = { 0 };
	HANDLE thread_handles = { 0 };
	
	
	ret_val = snprintf(thread_data.mission_file_name, _MAX_PATH, "%s", mission_file_name);
	if (FALSE == ret_val)
	{
		printf("main: snprintf failed.\n");
		exit_code = FAILURE;
		goto Resource_Handling;
	}

	
	h_thread = create_thread_simple(mission_thread, thread_ids, &thread_data);
	if (NULL == h_thread)
	{
		printf("main: create_thread_simple failed.\n");
		exit_code = FAILURE;
		goto Resource_Handling;
	}



	wait_code = WaitForSingleObject(h_thread, WAIT_TIME);
	if (wait_code != WAIT_OBJECT_0)
	{
		printf("main: waiting for thread failure.\n");
		exit_code = FAILURE;
		goto Resource_Handling;
	}

	ret_val = GetExitCodeThread(h_thread, &exit_code);
	if (FALSE == ret_val)
	{
		printf("main: GetExitCodeThread failed.\n");
		exit_code = FAILURE;
		goto Resource_Handling;
	}

	if (exit_code == FAILURE)
	{
		printf("main: thread's exit code was FAILURE.\n");
		goto Resource_Handling;
	}
	


Resource_Handling:
	if (NULL != h_thread)
	{
		if (0 == CloseHandle(h_thread))
		{
			printf("main: couldn't close thread handle.\n");
		}
	}

	if (lock != NULL)
	{
		DestroyLock(lock);
	}

	if (q != NULL)
	{
		DestroyQueue(q);
	}


	if (NULL != h_q_mutex)
	{
		if (0 == CloseHandle(h_q_mutex))
		{
			printf("main: couldn't close h_q_mutex handle.\n");
		}
	}


	if (h_priority_file != NULL) 
	{
		if (0 == CloseHandle(h_priority_file))
		{
			printf("main: couldn't close priority file handle.\n");
		}
	}

	return exit_code;
}