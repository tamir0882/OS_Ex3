#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

#include "utility_functions.h"
#include "HardCodedData.h"
#include "Queue.h"
#include "Lock.h"

DWORD WINAPI mission_thread(LPVOID lpParam)
{
	int exit_code = SUCCESS;

	Data* p_thread_data = (Data*)lpParam;

	Element* p_element = NULL;

	int ret_val = 0;
	int wait_code = 0;

	int mission = 0;

	char* temp = NULL;
	char* output_str = NULL;
	int output_str_allocation_size = FIXED_BUFFER_SIZE;

	int p_primes_allocation_size = INITIAL_ALLOCATION_SIZE;
	int* p_primes = NULL;
	int number_of_primes = 0;

	HANDLE h_mission_file = NULL;


	h_mission_file = open_file_for_read_and_write(p_thread_data->mission_file_name);
	if (NULL == h_mission_file)
	{
		printf("Thread operation failed - open_file_for_read_and_write failed.\n");
		return FAILURE;
	}



	p_primes = (int*)malloc(p_primes_allocation_size * sizeof(int));
	if (NULL == p_primes)
	{
		printf("memory allocation failed.\n");
		exit_code = FAILURE;
		goto Resource_Handling;
	}


	output_str = (char*)malloc(output_str_allocation_size * sizeof(char));
	if (NULL == output_str)
	{
		printf("allocation failure.\n");
		exit_code = FAILURE;
		goto Resource_Handling;
	}



	while (!Empty(p_thread_data->q))
	{

		/*********************read file*********************/
		/***************************************************/

		exit_code = read_lock(p_thread_data->lock);
		if (FAILURE == exit_code)
		{
			printf("Thread operation failed - read_lock failed.\n");
			goto Resource_Handling;
		}
		
		wait_code = WaitForSingleObject(p_thread_data->h_q_mutex, WAIT_TIME);
		if (WAIT_OBJECT_0 != wait_code)
		{
			printf("Thread operation failed - h_q_mutex.wait() failed.\n");
			exit_code = FAILURE;
			goto Resource_Handling;
		}

		if (Empty(p_thread_data->q))
		{
			break;
		}

		p_element = Top(p_thread_data->q);
		if (NULL == p_element)
		{
			printf("Thread operation failed - Top returned NULL pointer.\n");
			exit_code = FAILURE;
			goto Resource_Handling;
		}

		ret_val = SetFilePointer(h_mission_file, p_element->index, NULL, FILE_BEGIN);
		if (INVALID_SET_FILE_POINTER == ret_val)
		{
			printf("Thread operation failed - SetFilePointer failed.\n");
			exit_code = FAILURE;
			goto Resource_Handling;
		}

		Pop(p_thread_data->q);

		ret_val = ReleaseMutex(p_thread_data->h_q_mutex);
		if (0 == ret_val)
		{
			printf("Thread operation failed - h_q_mutex.signal() failed.\n");
			exit_code = FAILURE;
			goto Resource_Handling;
		}

		/**********get mission**********/
		/*******************************/
		mission = get_mission(h_mission_file);

		if (FAILURE == mission)
		{
			printf("Thread opertaion failed - get_mission failed.\n");
			exit_code = FAILURE;
			goto Resource_Handling;
		}


		exit_code = read_release(p_thread_data->lock);
		if (FAILURE == exit_code)
		{
			printf("Thread opertaion failed - get_mission failed.\n");
			goto Resource_Handling;
		}

		/*******************************/
		/***************************************************/


		/*******find prime factors*******/
		/********************************/

		p_primes = find_prime_factors(p_primes, &p_primes_allocation_size, mission, &number_of_primes);
		if (NULL == p_primes)
		{
			printf("Thread operation failed - find_prime_factors failed.\n");
			exit_code = FAILURE;
			goto Resource_Handling;
		}
		/********************************/


		/*******set print format*******/
		/******************************/
		output_str_allocation_size = FIXED_BUFFER_SIZE + number_of_primes * WHITESPACE_COMMA_FACTOR;

		temp = (char*)realloc(output_str, output_str_allocation_size * sizeof(char));
		if (NULL == temp)
		{
			printf("allocation failure.\n");
			exit_code = FAILURE;
			goto Resource_Handling;
		}
		output_str = temp; 

		ret_val = set_print_format(output_str, output_str_allocation_size, mission, p_primes, number_of_primes);
		if (FAILURE == ret_val)
		{
			printf("Thread operation faile - set_primt format failed.\n");
			exit_code = FAILURE;
			goto Resource_Handling;
		}
		/******************************/




		/**************write to file**************/
		/*****************************************/
		exit_code = write_lock(p_thread_data->lock);
		if (FAILURE == exit_code)
		{
			printf("Thread operation failed - write_lock failed.\n");
			goto Resource_Handling;
		}

		ret_val = SetFilePointer(h_mission_file, 0, NULL, FILE_END);
		if (ret_val == INVALID_SET_FILE_POINTER)
		{
			printf("Thread operation failed - SetFilePointer failed.\n");
			exit_code = FAILURE;
			goto Resource_Handling;
		}

		ret_val = WriteFile(h_mission_file, output_str, strlen(output_str), NULL, NULL);
		if (0 == ret_val)
		{
			printf("Thread_operation_failed - WriteFile failed.\n");
			exit_code = FAILURE;
			goto Resource_Handling;
		}

		
		exit_code = write_release(p_thread_data->lock);
		if (FAILURE == exit_code)
		{
			printf("Thread operation failed - write_release failed.\n");
			goto Resource_Handling;
		}
		/*****************************************/

	}
	

Resource_Handling:
	if (NULL != output_str)
	{
		free(output_str);
	}

	if (NULL != p_primes)
	{
		free(p_primes);
	}
	if (NULL != h_mission_file)
	{
		if (0 == CloseHandle(h_mission_file))
		{
			printf("Thread operation failed - couldn't close file handle.\n");
		}
	}
	return exit_code;
}