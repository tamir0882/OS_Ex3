#include <stdio.h>
#include <windows.h>

#include "HardCodedData.h"
///////////////////////////////////////////////////////////////////////////////////////////////////////

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