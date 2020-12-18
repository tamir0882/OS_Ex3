

#include <stdio.h>
#include <string.h>
#include <windows.h>
#include <math.h>

#include "HardCodedData.h"
#include "Queue.h"

/* HANDLE create_file_for_read: a wrapper for CreateFileA.

* Parameters:
* LPCTR file_name - path for a file to open for reading.

* Return value:
* on success - a handle to a file with GENERIC_READ permission
* on failure - NULL
*/
HANDLE create_file_for_read(char* file_name)
{
	if (NULL == file_name)
	{
		printf("ARG ERROR - NULL pointer was given to create_file_for_read.\n");
		return NULL;
	}

	HANDLE h_file = CreateFileA(file_name, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL, NULL);
	if (ERROR_FILE_NOT_FOUND == GetLastError() || INVALID_HANDLE_VALUE == h_file)
	{
		printf("FILE ERROR - could not open file. ABORT.\n");
		return NULL;
	}
	return h_file;
}




int initialize_main_thread(int argc, char** argv, char mission_file_name[_MAX_PATH],
	char priority_file_name[_MAX_PATH], HANDLE* p_h_priority_file, int* p_number_of_missions)
{
	if (argc != EXPECTED_ARGC)
	{
		printf("%d arguments are expected. Can't run, Exitting.\n", EXPECTED_ARGC);
		return FAILURE;
	}

	int ret_val = 0;


	ret_val = snprintf(mission_file_name, _MAX_PATH, "%s", argv[MISSION_FILE_OFFSET]);
	if (0 == ret_val)
	{
		printf("mission file name invalid.\n");
		return FAILURE;
	}



	ret_val = snprintf(priority_file_name, _MAX_PATH, "%s", argv[PRIORITY_FILE_OFFSET]);
	if (0 == ret_val)
	{
		printf("priority file name invalid.\n");
		return FAILURE;
	}



	*p_h_priority_file = create_file_for_read(priority_file_name);

	//check if function failed
	if (NULL == *p_h_priority_file)
	{
		printf("create_file_for_read returned NULL pointer.\n"
			"Exitting program.\n");
		return FAILURE;
	}


	*p_number_of_missions = (int)strtol(argv[NUMBER_OF_LINES_OFFSET], NULL, DECIMAL_BASE);
	if (*p_number_of_missions == 0)
	{
		printf("ERROR in main - strtol has failed.\n");
		if (0 == CloseHandle(*p_h_priority_file))
		{
			printf("close_handle failed.\n");
		}
		return FAILURE;
	}
	return SUCCESS;
}



/* HANDLE open_existing_file_for_write: a wrapper for CreateFileA.

* Parameters:
* LPCTR file_name - path for a file to create for writing.

* Return value:
* on success - a handle to a file with GENERIC_WRITE permission
* on failure - NULL
*/
HANDLE open_existing_file_for_write(LPCSTR file_name)
{
	if (NULL == file_name)
	{
		printf("ARG ERROR - NULL pointer was given. ABORT.\n");
		return NULL;
	}

	HANDLE h_file = CreateFileA(file_name, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL, NULL);
	if (ERROR_FILE_NOT_FOUND == GetLastError() || INVALID_HANDLE_VALUE == h_file)
	{
		printf("ERROR - could not open file. ABORT\n");
		return NULL;
	}
	return h_file;
}



HANDLE open_file_for_read_and_write(LPCSTR file_name)
{
	if (NULL == file_name)
	{
		printf("ARG ERROR - NULL pointer was given to open_file_for_read_and_write.\n");
		return NULL;
	}

	HANDLE h_file = CreateFileA(file_name, (GENERIC_WRITE | GENERIC_READ), (FILE_SHARE_WRITE | FILE_SHARE_READ),
		NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (ERROR_FILE_NOT_FOUND == GetLastError() || INVALID_HANDLE_VALUE == h_file)
	{
		return NULL;
	}
	return h_file;
}




/* int get_end_index:
* Description - This function finds the end positions in the file for a specific thread.

* Parameters:
* HANDLE h_file - handle to a file with GENERIC_READ permission.
* int lines_for_thread - number of lines from the file for each thread to operate on.
* int start_index - the start position of the thread.

* Return Value:
* on success - returns the end position of a thread (a positive integer).
* on failure - returns FAILURE (integer value -1),
*/


int set_mission_index(HANDLE h_priority_file, Element* p_element, int line_index)
{
	if (NULL == h_priority_file)
	{
		printf("ARG ERROR - NULL pointer was given to get_index.\n");
		return FAILURE;
	}


	int ret_val = 0;

	ret_val = SetFilePointer(h_priority_file, line_index, NULL, FILE_BEGIN);
	if (INVALID_SET_FILE_POINTER == ret_val)
	{
		printf("create_queue: SetFilePointer failed.\n");
		return FAILURE;
	}

	char buffer = 0;
	

	int count_charcters_in_line = 0;
	int index = 0;

	while (TRUE)
	{
		ret_val = ReadFile(h_priority_file, &buffer, BUFFER_SIZE_1, NULL, NULL);
		if (FALSE == ret_val)
		{
			printf("FILE ERROR - ReadFile failed in get_index.\n");
			return FAILURE;
		}

		if (buffer == '\r')
		{
			count_charcters_in_line += 2;
			break;
		}


		if (0 == index)
		{
			index = (int)(strtol(&buffer, NULL, DECIMAL_BASE));
		}
		else
		{
			index = index * 10;
			index += (int)(strtol(&buffer, NULL, DECIMAL_BASE));
		}
		
		if (0 == index)
		{
			if (buffer != '0')
			{
				printf("get_index: strtol failed.\n");
				return FAILURE;
			}
		}
		count_charcters_in_line++;
	}

	p_element->index = index;
	return count_charcters_in_line;
}




int get_mission(HANDLE h_mission_file)
{
	int ret_read = 0;
	char read_buffer = 0;

	int mission = 0;

	while (TRUE)
	{
		ret_read = ReadFile(h_mission_file, &read_buffer, BUFFER_SIZE_1, NULL, NULL);

		if (0 == ret_read)
		{
			printf("get_mission: ReadFile failed.\n");
			return FAILURE;
		}

		if (read_buffer == '\n' || read_buffer == '\r')
		{
			break;
		}

		if (0 == mission)
		{
			mission = (int)(strtol(&read_buffer, NULL, DECIMAL_BASE));
		}
		else
		{
			mission = mission * 10;
			mission += (int)(strtol(&read_buffer, NULL, DECIMAL_BASE));
		}

		if (0 == mission)
		{
			if (read_buffer != '0')
			{
				printf("get_mission: strtol failed.\n");
				return FAILURE;
			}
		}
	}

	return mission;
}


Queue* create_queue(HANDLE h_priority_file, int number_of_missions)
{
	Queue* q = NULL;
	q = InitializeQueue();
	if (NULL == q)
	{
		printf("create_queue: InitializeQueue failed.\n");
		return NULL;
	}

	int ret_val = 0;
	int next_line_index = 0;
	int next_line_offset = 0;

	Element** p_p_elements = NULL;
	p_p_elements = (Element**)malloc(number_of_missions * sizeof(Element*));
	if (NULL == p_p_elements)
	{
		printf("create_queue: Memory allocation failure.\n");
		DestroyQueue(q);
		return NULL;
	}

	for (int i = 0; i < number_of_missions; i++)
	{
		*p_p_elements = (Element*)malloc(sizeof(Element));

		next_line_offset = set_mission_index(h_priority_file, *p_p_elements, next_line_index);
		if (FAILURE == next_line_offset)
		{
			printf("create_queue: set_mission_index failed.\n");
			DestroyQueue(q);
			free(p_p_elements);
			return NULL;
		}
		next_line_index += next_line_offset;

		Push(q, *p_p_elements);
		p_p_elements++;
	}

	p_p_elements -= (number_of_missions);
	free(p_p_elements);
	return q;
}




/* HANDLE create_thread_simple:
* Description - creates a thread using CreateThread.

* Parameters:
* LPTHREAD_START_ROUTINE p_start_routine - the start index of the thread in file.
* LPDWORD p_thread_id - thread's id to use in CreateThread function.
* Data* thread_data - pointer to an array of struct Data containg each thread's data.

* Return Value:
* returns a handle to the thread.
*/
HANDLE create_thread_simple(LPTHREAD_START_ROUTINE p_start_routine,
	LPDWORD p_thread_id, Data* thread_data)
{
	HANDLE thread_handle;

	if (NULL == p_start_routine || NULL == p_thread_id)
	{
		printf("create_thread_simple: Received NULL pointer.\n");
		return NULL;
	}

	thread_handle = CreateThread(
		NULL,            /*  default security attributes */
		DEFAULT_STACK_SIZE,               /*  use default stack size */
		(LPTHREAD_START_ROUTINE)p_start_routine, /*  thread function */
		thread_data,     /*  argument to thread function */
		DEFAULT_CREATION_FLAGS,               /*  use default creation flags */
		p_thread_id);    /*  returns the thread identifier */

	if (NULL == thread_handle)
	{
		printf("create_thread_simple: CreateThread failed.\n");
		return NULL;
	}

	return thread_handle;
}



int find_prime_factors(int* Primes, int allocation_size, int n)
{
	//get number of 2 needed.
	int index = 0;
	int* temp = NULL;

	if (0 == n)
	{
		printf("find_prime_factors: invalid input.\n");
		return FAILURE;
	}

	while ((n % 2) == 0)
	{
		if (index > allocation_size - 1)
		{
			allocation_size = allocation_size * 2;

			temp = (int*)realloc(Primes, allocation_size * sizeof(int));
			if (NULL == temp)
			{
				printf("Memory Error, Couldn't execute realloc. ABORT.");
				free(Primes);
				return FAILURE;
			}
		}

		n = n / 2;
		Primes[index] = 2;
		printf("Primes[%d] = %d\n", index, Primes[index]);

		index += 1;
	}


	//get rest of prime numbers
	int i = 3;
	while (i <= (int)sqrt(n))
	{
		while ((n % i) == 0)
		{
			if (index > allocation_size - 1)
			{
				allocation_size = allocation_size * 2;

				temp = (int*)realloc(Primes, allocation_size * sizeof(int));
				if (NULL == temp)
				{
					printf("Memory Error, Couldn't execute realloc. ABORT.");
					free(Primes);
					return FAILURE;
				}
			}

			n = n / i;
			Primes[index] = i;
			printf("Primes[%d] = %d\n", index, Primes[index]);
			index += 1;
		}
		i += 2;
	}

	if (n > 2)
	{
		allocation_size = index + 1;
		temp = (int*)realloc(Primes, allocation_size * sizeof(int));
		if (NULL == temp)
		{
			printf("Memory Error, Couldn't execute realloc. ABORT.");
			free(Primes);
			return FAILURE;
		}

		Primes[index] = n;
		printf("Primes[%d] = %d\n", index, Primes[index]);

	}
	else if (allocation_size > index)
	{
		allocation_size = index;
		temp = (int*)realloc(Primes, allocation_size * sizeof(int));
		if (NULL == temp)
		{
			printf("Memory Error, Couldn't execute realloc. ABORT.");
			free(Primes);
			return FAILURE;
		}
	}

	return allocation_size;
}


int set_print_format(char* str, int allocation_size, int mission, int* p_primes, int number_of_primes)
{
	int bytes_written = 0;

	bytes_written = snprintf(str, allocation_size, "The prime factors of %d are:", mission);

	if (bytes_written < 0)
	{
		printf("snprintf failed in set_print_format.\n");
		return FAILURE;
	}

	int index = 0;
	for (index; index < number_of_primes - 1; index++)
	{
		bytes_written = snprintf(str, allocation_size, "%s %d,", str, p_primes[index]);

		if (bytes_written < 0)
		{
			printf("snprintf failed in set_print_format.\n");
			return FAILURE;
		}
	}
	bytes_written = snprintf(str, allocation_size, "%s %d\n", str, p_primes[index]);

	if (bytes_written < 0)
	{
		printf("snprintf failed in set_print_format.\n");
		return FAILURE;
	}

	return SUCCESS;
}







