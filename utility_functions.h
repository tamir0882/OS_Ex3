#ifndef __UTILITY_FUNCTIONS_H__
#define __UTILITY_FUNCTIONS_H__

#include <stdio.h>
#include <string.h>
#include <windows.h>
#include "HardCodedData.h"


HANDLE create_file_for_read(LPCSTR file_name);

HANDLE open_existing_file_for_write(LPCSTR file_name);

HANDLE open_file_for_read_and_write(LPCSTR file_name);

HANDLE create_thread_simple(LPTHREAD_START_ROUTINE p_start_routine,
	LPDWORD p_thread_id, Data* thread_data);



int set_mission_index(HANDLE h_priority_file, Element* p_element, int line_index);

int get_mission(HANDLE h_mission_file);

int find_prime_factors(int* Primes, int allocation_size, int n);

int set_print_format(char* str, int string_size, int mission, int* p_primes, int number_of_primes);

int initialize_main_thread(int argc, char** argv, char mission_file_name[_MAX_PATH],
	char priority_file_name[_MAX_PATH], HANDLE h_priority_file, int* p_number_of_missions);

Queue* create_queue(HANDLE h_priority_file, int number_of_missions);



#endif // __UTILITY_FUNCTIONS_H__