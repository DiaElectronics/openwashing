#ifndef DIA_CHANNEL_SEC_H
#define DIA_CHANNEL_SEC_H

#define CHANNEL_NOERROR 0
#define CHANNEL_BUFFER_OVERFLOW 1
#define CHANNEL_BUFFER_EMPTY 2
#define CHANNEL_BUFFER_DESTINATION_IS_NULL 3

#define CHANNEL_WAIT_FOR_DATA 1
#define CHANNEL_DO_NOT_WAIT_FOR_DATA 0

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <pthread.h>
#include <stdint.h>
#include <string.h>
#include <string>
#include <list>

template <class T>
class DiaChannel {
private:
    std::list<T*> container;
public:

    int Push(T * newElement) {
        pthread_mutex_lock(&listLock);
        container.push_back(newElement);
        pthread_mutex_unlock(&listLock);
        return 0;
    }
    
    int DropOne() {
        int err = 0;
        pthread_mutex_lock(&listLock);
        if(!container.empty()) {
            T *result = container.front();
            container.pop_front();
            free(result);
        } else {
            err = 1;
        }
        pthread_mutex_unlock(&listLock);
        return err;
    }
    
    int Peek(T ** result) {
        int err = 0;
        pthread_mutex_lock(&listLock);
        if (container.empty()) {
            err = CHANNEL_BUFFER_EMPTY;
            *result = NULL;
        } else {
            *result = container.front();
        }
        pthread_mutex_unlock(&listLock);
        return err;        
    }
    
    int Pop(T ** result) {
        int err = 0;
        pthread_mutex_lock(&listLock);
        if (container.empty()) {
            err = CHANNEL_BUFFER_EMPTY;
            *result = NULL;
        } else {
            *result = container.front();
            container.pop_front();
        }
        pthread_mutex_unlock(&listLock);
        return err;        
    }
    
    DiaChannel() {
        pthread_mutex_init(&listLock, 0);
    }
    
    ~DiaChannel() {
        pthread_mutex_destroy(&listLock);
    }
    
private:    
    pthread_mutex_t listLock;
};
#endif
