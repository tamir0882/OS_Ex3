#ifndef __LOCK_H__
#define __LOCK_H__

#include "HardCodedData.h"


Lock* InitializeLock();

int read_lock(Lock* lock);

int read_release(Lock* lock);

int write_lock(Lock* lock);

int write_release(Lock* lock);

int DestroyLock(Lock* lock);

#endif //__LOCK_H__