#include <stdio.h>
#include <windows.h>

int SUCCESS = 0;
int FAILURE = -1;
int WAIT_TIME = 5000;

typedef struct Lock
{
	int readers;
	HANDLE h_mutex;
	HANDLE h_room_empty;
	HANDLE h_turnstile;

} Lock;

///////////////////////////////////////////////////////////////////////////////////////////////////////

Lock* InitializeLock()
{
	Lock* lock = (Lock*)malloc(sizeof(Lock));
	if (NULL == lock)
	{
		printf("InitializeLock: Memory Error - malloc didn't work");
		return NULL;
	}
	
	lock->readers = 0;
	
	lock->h_mutex = CreateMutexA(
		NULL,              // default security attributes
		FALSE,             // initially not owned
		NULL);             // unnamed mutex
	if (NULL == lock->h_mutex)
	{ 
		printf("InitializeLock: CreateMutexA on h_mutex didn't work");
		free(lock);
		return NULL;
	}
	
	lock->h_room_empty = CreateSemaphoreA(
		NULL,           // default security attributes
		1,  // initial count
		MAXIMUM_WAIT_OBJECTS,  // maximum count
		NULL);          // unnamed semaphore
	if (NULL == lock->h_room_empty)
	{
		printf("InitializeLock: CreateSemaphoreA on h_room_empty didn't work");
		if (0 == CloseHandle(lock->h_mutex))
		{
			printf("InitializeLock: Close_Handle couldn't close h_mutex");
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
		printf("InitializeLock: CreateMutexA on h_turnstile didn't work");
		if (0 == CloseHandle(lock->h_mutex))
		{
			printf("InitializeLock: Close_Handle couldn't close h_mutex");
		}
		if (0 == CloseHandle(lock->h_room_empty))
		{
			printf("InitializeLock: Close_Handle couldn't close h_room_empty");
		}
		free(lock);
		return NULL;
	}
	
	return lock;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////

//need to address the operation for single thread

int read_lock(Lock* lock)
{
	int wait_code = 0;
	wait_code = WaitForSingleObject(lock->h_turnstile, WAIT_TIME); //turnstile.wait()
	if (WAIT_OBJECT_0 != wait_code)
	{
		printf("read_lock: WaitForSingleObject on h_turnstile didn't work");
		goto Close_Handles;
	}
	
	ReleaseMutex(lock->h_turnstile); //turnstile.signal()
	if (0 == ReleaseMutex(lock->h_turnstile)) 
	{
		printf("read_lock: ReleaseMutex on h_turnstile didn't work");
		goto Close_Handles;;
	}

	wait_code = WaitForSingleObject(lock->h_mutex, WAIT_TIME); //mutex.wait()
	if (WAIT_OBJECT_0 != wait_code)
	{
		printf("read_lock: WaitForSingleObject on h_mutex didn't work");
		goto Close_Handles;
	}

	lock->readers += 1;

	if (1 == lock->readers)
	{
		wait_code = WaitForSingleObject(lock->h_room_empty, WAIT_TIME); //room_empty.wait()
		if (WAIT_OBJECT_0 != wait_code)
		{
			printf("read_lock: WaitForSingleObject on h_room_empty didn't work");
			goto Close_Handles;
		}
	}

	ReleaseMutex(lock->h_mutex); // mutex.signal()
	if (0 == ReleaseMutex(lock->h_mutex))
	{
		printf("read_lock: ReleaseMutex error on h_mutex didn't work");
		goto Close_Handles;
	}

	return SUCCESS;

Close_Handles:
	if (NULL != lock->h_mutex)
	{
		if (0 == CloseHandle(lock->h_mutex))
		{
			printf("read_lock: Close_Handle couldn't close h_mutex");
		}
	}
	if (NULL != lock->h_room_empty)
	{
		if (0 == CloseHandle(lock->h_room_empty))
		{
			printf("read_lock: Close_Handle couldn't close h_room_empty");
		}
	}
	if (NULL != lock->h_turnstile)
	{
		if (0 == CloseHandle(lock->h_turnstile))
		{
			printf("read_lock: Close_Handle couldn't close h_turnstile");
		}
	}
	free(lock);
	return FAILURE;


}

///////////////////////////////////////////////////////////////////////////////////////////////////////

//need to address the operation for single thread

int read_release(Lock* lock)
{
	int wait_code = 0;
	wait_code = WaitForSingleObject(lock->h_mutex, WAIT_TIME); //mutex.wait()
	if (WAIT_OBJECT_0 != wait_code)
	{
		printf("read_release: WaitForSingleObject on h_mutex didn't work");
		goto Close_Handles;
	}
	lock->readers -= 1;

	if (0 == lock->readers)
	{
		ReleaseSemaphore(lock->h_room_empty, 1, NULL); //room_empty.signal()
		if (0 == ReleaseSemaphore(lock->h_room_empty, 1, NULL))
		{
			printf("read_release: ReleaseSemaphore on h_room_empty didn't work");
			goto Close_Handles;
		}
	}

	ReleaseMutex(lock->h_mutex); // mutex.signal()
	if (0 == ReleaseMutex(lock->h_mutex))
	{
		printf("read_release: ReleaseMutex error on h_mutex didn't work");
		goto Close_Handles;
	}

	return SUCCESS;

Close_Handles:
	if (NULL != lock->h_mutex)
	{
		if (0 == CloseHandle(lock->h_mutex))
		{
			printf("read_release: Close_Handle couldn't close h_mutex");
		}
	}
	if (NULL != lock->h_room_empty)
	{
		if (0 == CloseHandle(lock->h_room_empty))
		{
			printf("read_release: Close_Handle couldn't close h_room_empty");
		}
	}
	if (NULL != lock->h_turnstile)
	{
		if (0 == CloseHandle(lock->h_turnstile))
		{
			printf("read_release: Close_Handle couldn't close h_turnstile");
		}
	}
	free(lock);
	return FAILURE;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////

//need to address the operation for single thread and read while write.

int write_lock(Lock* lock)
{
	int wait_code = 0;
	wait_code = WaitForSingleObject(lock->h_turnstile, WAIT_TIME); //turnstile.wait()
	if (WAIT_OBJECT_0 != wait_code)
	{
		printf("write_lock: WaitForSingleObject on h_turnstile didn't work");
		goto Close_Handles;
	}

	wait_code = WaitForSingleObject(lock->h_room_empty, WAIT_TIME); //room_empty.wait()
	if (WAIT_OBJECT_0 != wait_code)
	{
		printf("write_lock: WaitForSingleObject on h_room_empty didn't work");
		goto Close_Handles;
	}

	return SUCCESS;

Close_Handles:
	if (NULL != lock->h_mutex)
	{
		if (0 == CloseHandle(lock->h_mutex))
		{
			printf("write_lock: Close_Handle couldn't close h_mutex");
		}
	}
	if (NULL != lock->h_room_empty)
	{
		if (0 == CloseHandle(lock->h_room_empty))
		{
			printf("write_lock: Close_Handle couldn't close h_room_empty");
		}
	}
	if (NULL != lock->h_turnstile)
	{
		if (0 == CloseHandle(lock->h_turnstile))
		{
			printf("write_lock: Close_Handle couldn't close h_turnstile");
		}
	}
	free(lock);
	return FAILURE;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////

//need to address the operation for single thread

int write_release(Lock* lock)
{
	ReleaseMutex(lock->h_turnstile); //turnstile.signal()
	if (0 == ReleaseMutex(lock->h_turnstile))
	{
		printf("write_release: ReleaseMutex on turnstile didn't work");
		goto Close_Handles;;
	}

	ReleaseSemaphore(lock->h_room_empty, 1, NULL); //room_empty.signal()
	if (0 == ReleaseSemaphore(lock->h_room_empty, 1, NULL))
	{
		printf("write_release: ReleaseSemaphore on room_empty didn't work");
		goto Close_Handles;
	}

	return SUCCESS;

Close_Handles:
	if (NULL != lock->h_mutex)
	{
		if (0 == CloseHandle(lock->h_mutex))
		{
			printf("write_release: Close_Handle couldn't close h_mutex");
		}
	}
	if (NULL != lock->h_room_empty)
	{
		if (0 == CloseHandle(lock->h_room_empty))
		{
			printf("write_release: Close_Handle couldn't close h_room_empty");
		}
	}
	if (NULL != lock->h_turnstile)
	{
		if (0 == CloseHandle(lock->h_turnstile))
		{
			printf("write_release: Close_Handle couldn't close h_turnstile");
		}
	}
	free(lock);
	return FAILURE;
}

void DestroyLock(Lock* lock)
{
	if (NULL != lock->h_mutex)
	{
		if (0 == CloseHandle(lock->h_mutex))
		{
			printf("DestroyLock: Close_Handle couldn't close h_mutex");
		}
		lock->h_mutex = NULL;
	}
	
	if (NULL != lock->h_room_empty)
	{
		if (0 == CloseHandle(lock->h_room_empty))
		{
			printf("DestroyLock: Close_Handle couldn't close h_room_empty");
		}
		lock->h_room_empty = NULL;
	}
	
	if (NULL != lock->h_turnstile)
	{
		if (0 == CloseHandle(lock->h_turnstile))
		{
			printf("DestroyLock: Close_Handle couldn't close h_turnstile");
		}
		lock->h_turnstile = NULL;
	}
	free(lock);
	lock = NULL;
	return;
}