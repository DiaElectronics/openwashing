#ifndef _DIA_CHANNEL_H
#define _DIA_CHANNEL_H

#include <pthread.h>
#define CHANNEL_NOERROR 0
#define CHANNEL_BUFFER_OVERFLOW 1
#define CHANNEL_BUFFER_EMPTY 2
#define CHANNEL_BUFFER_DESTINATION_IS_NULL 3

#define CHANNEL_WAIT_FOR_DATA 1
#define CHANNEL_DO_NOT_WAIT_FOR_DATA 0

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

template <class T>
class DiaChannel{
public:
    pthread_mutex_t _BufferReaderLock = PTHREAD_MUTEX_INITIALIZER;
    pthread_mutex_t _BufferWriterLock = PTHREAD_MUTEX_INITIALIZER;

    pthread_cond_t CrossCondition = PTHREAD_COND_INITIALIZER;

    T * _Buffer; //it's just simpler to work with int
    int NumberOfElements;
    int SizeOfElement;
    int _ReaderCursor;
    int _ReaderLockedWithNoData;
    int _WriterCursor;

    void Unlock() {
        if(_ReaderLockedWithNoData) {
            pthread_cond_signal(&CrossCondition);
        }
    }

    int Push(T * newElement) {
        pthread_mutex_lock(&_BufferWriterLock);

        char * charLink = (char *)_Buffer + (SizeOfElement * _WriterCursor);
        T ** link = (T **)charLink;
        *link = newElement;

        // If writer was so fast that caught up reader from behind, we must stop the writer position and loose the elements;
        // It might happen writer is at the end of channel and reader is at zero. In that case we do not move
        // writer position as well;
        int currentError = CHANNEL_NOERROR;
        int maxCyclicIndex = NumberOfElements - 1;
        if(_WriterCursor + 1 == _ReaderCursor ||
        ( (_WriterCursor == maxCyclicIndex) && _ReaderCursor == 0) ) {
            currentError = CHANNEL_BUFFER_OVERFLOW;
        } else {
            IncreaseCyclicIndex(&_WriterCursor, maxCyclicIndex);
        }
        pthread_cond_signal(&CrossCondition);
        pthread_mutex_unlock(&_BufferWriterLock);
        return currentError;
    }

    int Pop(T ** result, int WaitForData) {
        pthread_mutex_lock(&_BufferReaderLock);
        int currentError = CHANNEL_NOERROR;
        //let's check if we have anything to read
        if(WaitForData && _ReaderCursor == _WriterCursor) {
            _ReaderLockedWithNoData = 1;
            pthread_cond_wait(&CrossCondition, &_BufferReaderLock);
        }
        _ReaderLockedWithNoData = 0;
        if(_ReaderCursor != _WriterCursor) {
            char * charLink = (char *)_Buffer + (SizeOfElement * _ReaderCursor);
            T ** link = (T **)charLink;
            *result = *link;
            *link = NULL;
            int maxCyclicIndex = NumberOfElements - 1;
            IncreaseCyclicIndex(&_ReaderCursor, maxCyclicIndex);
        } else {
            currentError = CHANNEL_BUFFER_EMPTY;
            *result = NULL;
        }
        pthread_mutex_unlock(&_BufferReaderLock);
        return currentError;
    }

    void IncreaseCyclicIndex (int * index, int maxValue) {
        *index = *index + 1;
        if(*index > maxValue) {
            *index = 0;
        }
    }

    DiaChannel(int numberOfElements) {
        SizeOfElement = sizeof(void *);
        _Buffer = (T *)malloc(numberOfElements * SizeOfElement);
        NumberOfElements = numberOfElements;
        _ReaderCursor = 0;
        _WriterCursor = 0;
    }

    ~DiaChannel() {
        pthread_cond_signal(&CrossCondition);
        if(_Buffer!=NULL) {
            free(_Buffer);
        }
    }
};

#endif
