#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

#include "utility_functions.h"
#include "HardCodedData.h"
#include "Queue.h"


DWORD WINAPI mission_thread(LPVOID lpParam)
{
	int exit_code = SUCCESS;

	Data* p_thread_data = (Data*)lpParam;

	HANDLE h_mission_file = NULL;
	h_mission_file = open_file_for_read_and_write(p_thread_data->mission_file_name);
	if (NULL == h_mission_file)
	{
		printf("Thread operation failed - open_file_for_read_and_write failed.\n");
		return FAILURE;
	}


	Element* q_element = NULL;

	int ret_val = 0;
	int mission = 0;

	int p_primes_allocation_size = INITIAL_ALLOCATION_SIZE;
	int* p_primes = NULL;
	int number_of_primes = 0;

	char* output_str = NULL;
	int output_str_allocation_size = 0;

	for (int i = 0; i < p_thread_data->number_of_missions; i++)
	{
		q_element = Top(p_thread_data->q);
		ret_val = SetFilePointer(h_mission_file, q_element->index, NULL, FILE_BEGIN);

		if (INVALID_SET_FILE_POINTER == ret_val)
		{
			printf("Thread operation failed - SetFilePointer failed.\n");
			exit_code = FAILURE;
			goto Free_STR;
		}


		/**********get mission**********/
		/*******************************/
		mission = get_mission(h_mission_file);

		if (FAILURE == mission)
		{
			printf("Thread opertaion failed - get_mission failed.\n");
			exit_code = FAILURE;
			goto Free_STR;
		}
		/*******************************/



		/*******find prime factors*******/
		/********************************/
		p_primes = (int*)malloc(p_primes_allocation_size * sizeof(int));
		if (NULL == p_primes)
		{
			printf("memory allocation failed.\n");
			exit_code = FAILURE;
			goto Free_STR;
		}

		number_of_primes = find_prime_factors(p_primes, p_primes_allocation_size, mission);

		if (FAILURE == number_of_primes)
		{
			printf("Thread operation failed - find_prime_factors failed.\n");
			exit_code = FAILURE;
			goto Free_STR;
		}
		/********************************/


		/*******set print format*******/
		/******************************/
		output_str_allocation_size = FIXED_BUFFER_SIZE + number_of_primes * WHITESPACE_COMMA_FACTOR;

		output_str = (char*)malloc(output_str_allocation_size * sizeof(char));
		if (NULL == output_str)
		{
			printf("allocation failure.\n");
			exit_code = FAILURE;
			goto Free_STR;
		}

		ret_val = set_print_format(output_str, output_str_allocation_size, mission, p_primes, number_of_primes);
		if (FAILURE == ret_val)
		{
			printf("Thread operation faile - set_primt format failed.\n");
			exit_code = FAILURE;
			goto Free_STR;
		}
		/******************************/




		/*******write to file*******/
		/***************************/
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
		/***************************/

		Pop(p_thread_data->q);
	}
	

Free_STR:
	if (NULL != output_str)
	{
		free(output_str);
	}

Close_Mission_File_Handle:
	if (0 == CloseHandle(h_mission_file))
	{
		printf("Thread operation failed - couldn't close file handle.\n");
	}

	return exit_code;
}