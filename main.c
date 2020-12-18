#include <stdio.h>
#include <string.h>
#include <windows.h>


#include "HardCodedData.h"
#include "utility_functions.h"
#include "Thread_operation.h"
#include "Queue.h"



int main(int argc, char** argv)
{
	
	int exit_code = SUCCESS;

	char mission_file_name[_MAX_PATH] = { 0 };
	char priority_file_name[_MAX_PATH] = { 0 };
	HANDLE h_priority_file = NULL;
	int number_of_missions = 0;
	
	exit_code = initialize_main_thread(argc, argv, mission_file_name, priority_file_name, &h_priority_file, &number_of_missions);
	if (FAILURE == exit_code)
	{
		printf("main: initialize_main_thread failed.\n");
		return FAILURE;
	}


	Queue* q = NULL;
	q = create_queue(h_priority_file, number_of_missions);
	if (NULL == q)
	{
		printf("main: create_queue failed.\n");
		exit_code = FAILURE;
		goto Close_Priority_File_Handle;
	}



	Data thread_data = { .q = q,.number_of_missions = number_of_missions, .mission_file_name = NULL };
	DWORD thread_ids[MAXIMUM_WAIT_OBJECTS] = { 0 };
	HANDLE thread_handles = { 0 };
	
	int ret_val = 0;
	ret_val = snprintf(thread_data.mission_file_name, _MAX_PATH, "%s", mission_file_name);
	if (FALSE == ret_val)
	{
		printf("main: snprintf failed.\n");
		exit_code = FAILURE;
		goto Destroy_Queue;
	}

	
	int wait_code = 0;
	HANDLE h_thread = NULL;

	h_thread = create_thread_simple(mission_thread, thread_ids, &thread_data);
	if (NULL == h_thread)
	{
		printf("main: create_thread_simple failed.\n");
		exit_code = FAILURE;
		goto Destroy_Queue;
	}

	wait_code = WaitForSingleObject(h_thread, WAIT_TIME);
	if (wait_code != WAIT_OBJECT_0)
	{
		printf("main: waiting for thread failure.\n");
		exit_code = FAILURE;
		goto Close_Thread_Handle;
	}

	ret_val = GetExitCodeThread(h_thread, &exit_code);
	if (exit_code == FAILURE)
	{
		printf("main: thread exit_code was FAILURE.\n");
		goto Close_Thread_Handle;
	}
	

Close_Thread_Handle:
	if (NULL != h_thread)
	{
		if (0 == CloseHandle(h_thread))
		{
			printf("main: couldn't close thread handle.\n");
		}
	}

Destroy_Queue:
	if (q != NULL)
	{
		DestroyQueue(q);
	}

Close_Priority_File_Handle:
	if (h_priority_file != NULL) 
	{
		if (0 == CloseHandle(h_priority_file))
		{
			printf("close_handle failed.\n");
		}
	}

	return exit_code;
}