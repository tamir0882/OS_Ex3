#include <stdio.h>
#include <windows.h>

#include "HardCodedData.h"
///////////////////////////////////////////////////////////////////////////////////////////////////////

//Description - this function initialize space in memory for a new lock, 
//and initialize the requested synchronization struct for each of it's field attributes.
//Parameters - No input parameters. The output parameter is a pointer to the new initialized lock.
//Returns - pointer to a struct of type Lock.

Lock* InitializeLock()
{
	Lock* lock = (Lock*)malloc(sizeof(Lock));
	if (NULL == lock)
	{
		printf("InitializeLock: Memory Error - malloc didn't work.\n");
		return NULL;
	}

	lock->readers = 0;

	lock->h_mutex = CreateMutexA(
		NULL,              // default security attributes
		FALSE,             // initially not owned
		NULL);             // unnamed mutex
	if (NULL == lock->h_mutex)
	{
		printf("InitializeLock: CreateMutexA on h_mutex didn't work.\n");
		free(lock);
		return NULL;
	}

	lock->h_room_empty = CreateSemaphoreA(
		NULL,           // default security attributes
		INITIAL_SEMAPHORE_COUNT,  // initial count
		MAXIMUM_WAIT_OBJECTS,  // maximum count
		NULL);          // unnamed semaphore
	if (NULL == lock->h_room_empty)
	{
		printf("InitializeLock: CreateSemaphoreA on h_room_empty didn't work.\n");
		if (0 == CloseHandle(lock->h_mutex))
		{
			printf("InitializeLock: Close_Handle couldn't close h_mutex.\n");
		}
		free(lock);
		return NULL;
	}

	lock->h_turnstile = CreateMutexA(
		NULL,              // default security attributes
		FALSE,             // initially not owned
		NULL);             // unnamed mutex
	if (NULL == lock->h_turnstile)
	{
		printf("InitializeLock: CreateMutexA on h_turnstile didn't work.\n");
		if (0 == CloseHandle(lock->h_mutex))
		{
			printf("InitializeLock: Close_Handle couldn't close h_mutex.\n");
		}
		if (0 == CloseHandle(lock->h_room_empty))
		{
			printf("InitializeLock: Close_Handle couldn't close h_room_empty.\n");
		}
		free(lock);
		return NULL;
	}

	return lock;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////


//Description - this function lets the given lock be used by a thread for reading. 
//the thread waits if there is other thread that writes given the state of turnstile attribute, or continues with the operation.
//The thread locks the mutex attribute and if the room attribute of the lock is empty it sets it to not be empty and 
//raises the number of current readers. Then, the mutex attribute is released.
//if all has succeeded, the return value is set to SUCCESS. Else, destroy the lock and the return value is FAILURE.
//Parameters - The input parameter is pointer to a struct of type Lock. The output parameter is an int.
//Returns - int with value that determines if the operation has succeeded or failed.

int read_lock(Lock* lock)
{
	int wait_code = 0;
	int ret_val = 0;

	wait_code = WaitForSingleObject(lock->h_turnstile, WAIT_TIME); //turnstile.wait()
	if (WAIT_OBJECT_0 != wait_code)
	{
		printf("read_lock: WaitForSingleObject on h_turnstile didn't work.\n");
		goto Destroy_Lock;
	}

	ret_val = ReleaseMutex(lock->h_turnstile); //turnstile.signal()
	if (0 == ret_val)
	{
		printf("read_lock: ReleaseMutex on h_turnstile didn't work.\n");
		goto Destroy_Lock;
	}

	wait_code = WaitForSingleObject(lock->h_mutex, WAIT_TIME); //mutex.wait()
	if (WAIT_OBJECT_0 != wait_code)
	{
		printf("read_lock: WaitForSingleObject on h_mutex didn't work.\n");
		goto Destroy_Lock;
	}

	lock->readers += 1;

	if (1 == lock->readers)
	{
		wait_code = WaitForSingleObject(lock->h_room_empty, WAIT_TIME); //room_empty.wait()
		if (WAIT_OBJECT_0 != wait_code)
		{
			printf("read_lock: WaitForSingleObject on h_room_empty didn't work.\n");
			goto Release_Mutex;
		}
	}

Release_Mutex:
	ret_val = ReleaseMutex(lock->h_mutex); // mutex.signal()
	if (0 == ret_val)
	{
		printf("read_lock: ReleaseMutex error on h_mutex didn't work.\n");
		goto Destroy_Lock;
	}

	return SUCCESS;

Destroy_Lock:
	if (FAILURE == DestroyLock(lock))
	{
		printf("write_lock: DestroyLock failed.\n");
	}
	return FAILURE;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////

//Description - this function lets the given lock be freed from the use of a thread from reading. 
//the thread locks the lock's mutex attribute and lowers the number of readers by one. if after that the number of readers is 0, 
//it sets it to be empty. Then, the thread release the mutex attribute for any thread who wants to use it.  
//if all has succeeded, the return value is set to SUCCESS. Else, destroy the lock and the return value is FAILURE.
//Parameters - The input parameter is pointer to a struct of type Lock. The output parameter is an int.
//Returns - int with value that determines if the operation has succeeded or failed.

int read_release(Lock* lock)
{
	int wait_code = 0;
	int ret_val = 0;

	wait_code = WaitForSingleObject(lock->h_mutex, WAIT_TIME); //mutex.wait()
	if (WAIT_OBJECT_0 != wait_code)
	{
		printf("read_release: WaitForSingleObject on h_mutex didn't work.\n");
		goto Destroy_Lock;
	}
	lock->readers -= 1;

	if (0 == lock->readers)
	{
		ret_val = ReleaseSemaphore(lock->h_room_empty, 1, NULL); //room_empty.signal()
		if (0 == ret_val)
		{
			printf("read_release: ReleaseSemaphore on h_room_empty didn't work.\n");
			goto Release_Mutex;
		}
	}

Release_Mutex:
	ret_val = ReleaseMutex(lock->h_mutex); // mutex.signal()
	if (0 == ret_val)
	{
		printf("read_release: ReleaseMutex error on h_mutex didn't work.\n");
		goto Destroy_Lock;
	}

	return SUCCESS;

Destroy_Lock:
	if (FAILURE == DestroyLock(lock))
	{
		printf("read_releas: DestroyLock failed.\n");
	}
	return FAILURE;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////

//Description - this function lets the given lock be used by a thread for writhing.
//the thread locks the turnstile attribute of the lock and the mutex attribute for any thread who wants to use it.  
//if all has succeeded, the return value is set to SUCCESS. Else, destroy the lock and the return value is FAILURE.
//Parameters - The input parameter is pointer to a struct of type Lock. The output parameter is an int.
//Returns - int with value that determines if the operation has succeeded or failed.

int write_lock(Lock* lock)
{
	int wait_code = 0;
	int ret_val = 0;
	wait_code = WaitForSingleObject(lock->h_turnstile, WAIT_TIME); //turnstile.wait()
	if (WAIT_OBJECT_0 != wait_code)
	{
		printf("write_lock: WaitForSingleObject on h_turnstile didn't work.\n");
		goto Destroy_Lock;
	}

	wait_code = WaitForSingleObject(lock->h_room_empty, WAIT_TIME); //room_empty.wait()
	if (WAIT_OBJECT_0 != wait_code)
	{
		printf("write_lock: WaitForSingleObject on h_room_empty didn't work.\n");

		ret_val = ReleaseMutex(lock->h_turnstile);
		if (0 == ret_val)
		{
			printf("write_lock: couldn't release h_turnstile.\n");
		}

		goto Destroy_Lock;
	}

	return SUCCESS;

Destroy_Lock:
	if (FAILURE == DestroyLock(lock))
	{
		printf("write_lock: DestroyLock failed.\n");
	}
	return FAILURE;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////

//Description - this function lets the given lock be freed from the use of a thread from writing.  
//the thread releases the turnstile attribute of the lock and the mutex attribute for any thread who wants to use it.  
//if all has succeeded, the return value is set to SUCCESS. Else, destroy the lock and the return value is FAILURE.
//Parameters - The input parameter is pointer to a struct of type Lock. The output parameter is an int.
//Returns - int with value that determines if the operation has succeeded or failed.

int write_release(Lock* lock)
{
	int ret_val = 0;
	ret_val = ReleaseMutex(lock->h_turnstile); //turnstile.signal()
	if (0 == ret_val)
	{
		printf("write_release: ReleaseMutex on turnstile didn't work.\n");
	}

	ret_val = ReleaseSemaphore(lock->h_room_empty, 1, NULL); //room_empty.signal()
	if (0 == ret_val)
	{
		printf("write_release: ReleaseSemaphore on room_empty didn't work.\n");
		goto Destroy_Lock;
	}

	return SUCCESS;

Destroy_Lock:
	if (FAILURE == DestroyLock(lock))
	{
		printf("write_releas: DestroyLock failed.\n");
	}
	return FAILURE;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////

//Description - this function destorys the lock and prevent any use of it for the future.
// after calling for the function, a check is being made if the handle for any attribute of the lock is NULL.
//if so, continue. If the attribute is not NULL, close the handle, and set the attribute as NULL.
//if all works, the lock frees the resources it has used, turns to NULL, and the return value is SUCCESS. 
//If anything failed in the way, an error is being printed and the return value is FAILURE.
////Parameters - The input parameter is pointer to a struct of type Lock. The output parameter is an int.
//Returns - int with value that determines if the operation has succeeded or failed.

int DestroyLock(Lock* lock)
{
	int exit_code = SUCCESS;
	if (NULL != lock->h_mutex)
	{
		if (0 == CloseHandle(lock->h_mutex))
		{
			printf("DestroyLock: Close_Handle couldn't close h_mutex.\n");
			exit_code = FAILURE;
		}
		lock->h_mutex = NULL;
	}

	if (NULL != lock->h_room_empty)
	{
		if (0 == CloseHandle(lock->h_room_empty))
		{
			printf("DestroyLock: Close_Handle couldn't close h_room_empty.\n");
			exit_code = FAILURE;
		}
		lock->h_room_empty = NULL;
	}

	if (NULL != lock->h_turnstile)
	{
		if (0 == CloseHandle(lock->h_turnstile))
		{
			printf("DestroyLock: Close_Handle couldn't close h_turnstile.\n");
			exit_code = FAILURE;
		}
		lock->h_turnstile = NULL;
	}
	free(lock);
	lock = NULL;
	return exit_code;
}
