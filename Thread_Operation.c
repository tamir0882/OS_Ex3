#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

#include "utility_functions.h"
#include "HardCodedData.h"



DWORD WINAPI mission_thread(LPVOID lpParam)
{
	int exit_code = SUCCESS;

	Data* p_thread_data = (Data*)lpParam;

	HANDLE h_mission_file = NULL;
	h_mission_file = open_file_for_read_and_write(p_thread_data->mission_file_name);

	if (NULL == h_mission_file)
	{
		exit_code = FAILURE;
		printf("Thread operation failed - open_file_for_read_and_write failed.\n");
		return exit_code;
	}

	int ret_val = 0;
	ret_val = SetFilePointer(h_mission_file, p_thread_data->index, NULL, FILE_BEGIN);

	if (INVALID_SET_FILE_POINTER == ret_val)
	{
		printf("Thread operation failed - SetFilePointer failed.\n");
		exit_code = FAILURE;
		goto Close_Mission_File_Handle;
	}


	/**********get mission**********/
	/*******************************/

	int mission = 0;
	mission = get_mission(h_mission_file);


	if (FAILURE == mission)
	{
		printf("Thread opertaion failed - get_mission failed.\n");
		exit_code = FAILURE;
		goto Close_Mission_File_Handle;
	}
	/*******************************/




	/*******find prime factors*******/
	/********************************/
	int p_primes_allocation_size = INITIAL_ALLOCATION_SIZE;

	int* p_primes = NULL;
	p_primes = (int*)malloc(p_primes_allocation_size * sizeof(int));
	if (NULL == p_primes)
	{
		printf("memory allocation failed.\n");
		exit_code = FAILURE;
		goto Close_Mission_File_Handle;
	}

	int number_of_primes = 0;
	number_of_primes = find_prime_factors(p_primes, p_primes_allocation_size, mission);

	if (FAILURE == number_of_primes)
	{
		printf("Thread operation failed - find_prime_factors failed.\n");
		exit_code = FAILURE;
		goto Close_Mission_File_Handle;
	}
	/********************************/



	/*******set print format*******/
	/******************************/
	char* output_str = NULL;
	int output_str_allocation_size = FIXED_BUFFER_SIZE + number_of_primes * 3;

	output_str = (char*)malloc(output_str_allocation_size * sizeof(char));
	if (NULL == output_str)
	{
		printf("allocation failure.\n");
		return FAILURE;
	}


	ret_val = set_print_format(output_str, output_str_allocation_size, mission, p_primes, number_of_primes);

	if (FAILURE == ret_val)
	{
		printf("Thread operation faile - set_primt format failed.\n");
		exit_code = FAILURE;
		goto Close_Mission_File_Handle;
	}
	/******************************/



	ret_val = SetFilePointer(h_mission_file, 0, NULL, FILE_END);

	if (ret_val == INVALID_SET_FILE_POINTER)
	{
		printf("Thread operation failed - SetFilePointer failed.\n");
		exit_code = FAILURE;
		goto Free_STR;
	}


	ret_val = WriteFile(h_mission_file, output_str, strlen(output_str), NULL, NULL);
		
	if (0 == ret_val)
	{
		printf("Thread_operation_failed - WriteFile failed.\n");
		exit_code = FAILURE;
		goto Free_STR;
	}

	

Free_STR:
	free(output_str);

Close_Mission_File_Handle:
	if (0 == CloseHandle(h_mission_file))
	{
		printf("Thread operation failed - couldn't close file handle.\n");
	}

	return exit_code;
}