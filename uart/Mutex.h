#ifndef MUTEX_H
#define MUTEX_H

#include <pthread.h>

class Mutex
{
public:
    Mutex(){pthread_mutex_init(&mMutex, NULL);}
    ~Mutex(){pthread_mutex_destroy(&mMutex);}

    int Lock(){ return pthread_mutex_lock(&mMutex);}
    int Unlock(){ return pthread_mutex_unlock(&mMutex);}
    int TryLock(){ return pthread_mutex_trylock(&mMutex);}

private:
    pthread_mutex_t mMutex;
};

#endif // MUTEX_H
