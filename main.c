


#include <stdio.h>
#include <string.h>
#include <windows.h>


#include "HardCodedData.h"
#include "utility_functions.h"
#include "Thread_operation.h"
#include "Queue.h"



int main(int argc, char** argv)
{
	if (argc != EXPECTED_ARGC)
	{
		printf("%d arguments are expected. Can't run, Exitting.\n", EXPECTED_ARGC);
		return FAILURE;
	}

	int ret_val = 0;
	char mission_file_name[_MAX_PATH] = { 0 };
	ret_val = snprintf(mission_file_name, _MAX_PATH, "%s", argv[MISSION_FILE_OFFSET]);
	if (0 == ret_val)
	{
		printf("mission file name invalid.\n");
		return FAILURE;
	}


	char priority_file_name[_MAX_PATH] = { 0 };
	ret_val = snprintf(priority_file_name, _MAX_PATH, "%s", argv[PRIORITY_FILE_OFFSET]);
	if (0 == ret_val)
	{
		printf("priority file name invalid.\n");
		return FAILURE;
	}

	int exit_code = SUCCESS;

	HANDLE h_priority_file = NULL;
	h_priority_file = create_file_for_read(priority_file_name);

	//check if function failed
	if (NULL == h_priority_file)
	{
		printf("create_file_for_read returned NULL pointer.\n"
			"Exitting program.\n");
		return FAILURE;
	}

	
	int number_of_missions = 0;
	number_of_missions = (int)strtol(argv[NUMBER_OF_LINES_OFFSET], NULL, DECIMAL_BASE);

	if (number_of_missions == 0)
	{
		exit_code = FAILURE;
		printf("ERROR in main - strtol has failed.\n");
		goto Close_Priority_File_Handle;
	}

	int temp = 0;
	int next_line_index = 0;
	int mission_index = 0;
	DWORD thread_id = 0;
	HANDLE thread_handle = NULL;
	Data thread_data = { .index = 0, .mission_file_name = NULL };
	int wait_code = 0;

	ret_val = snprintf(thread_data.mission_file_name, _MAX_PATH, "%s", mission_file_name);
	if (FALSE == ret_val)
	{
		printf("main: snprintf failed.\n");
		exit_code = FAILURE;
		goto Close_Priority_File_Handle;
	}

	for (int i = 0; i < number_of_missions; i++)
	{
		ret_val = SetFilePointer(h_priority_file, next_line_index, NULL, FILE_BEGIN);
		if (INVALID_SET_FILE_POINTER == ret_val)
		{
			printf("main: SetFilePointer failed.\n");
			exit_code = FAILURE;
			goto Close_Priority_File_Handle;
		}
		temp = set_mission_index(h_priority_file, &thread_data);

		if (FAILURE == temp)
		{
			printf("main: get_mission_index failed.\n");
			exit_code = FAILURE;
			goto Close_Priority_File_Handle;
		}
		next_line_index += temp;

		HANDLE h_thread = NULL;
		h_thread = create_thread_simple(mission_thread, &thread_id, &thread_data);
		if (NULL == h_thread)
		{
			printf("main: create_thread_simple failed.\n");
			exit_code = FAILURE;
			goto Close_Priority_File_Handle;
		}

		wait_code = WaitForSingleObject(h_thread, WAIT_TIME);
		if (wait_code != WAIT_OBJECT_0)
		{
			printf("wait failed.\n");
			exit_code = FAILURE;
			goto Close_Priority_File_Handle;
		}

		if (0 == CloseHandle(h_thread))
		{
			printf("handle wasn't close.\n");
			exit_code = FAILURE;
			goto Close_Priority_File_Handle;
		}
		//HERE NEED TO CREATE THREAD THAT WOULD CALCULATE AND PRINT 
		//THE PRIME NUMBERS IN THE CORRECT FORMAT FOR EACH MISSION
	}
	


Close_Priority_File_Handle: 
	if (0 == CloseHandle(h_priority_file))
	{
		printf("close_handle failed.\n");
	}

	return exit_code;
}