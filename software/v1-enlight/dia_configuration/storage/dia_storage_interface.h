#ifndef dia_storage_interface_h
#define dia_storage_interface_h

#include <stdlib.h>

typedef struct storage_interface {
    void * object;
    int (*save)(void * object, const char *key, const void *data, size_t length);
    int (*load)(void * object, const char *key, void *data, size_t length);
    int is_real;
    void * next_object;
} storage_interface_t;

storage_interface_t * CreateEmptyInterface();
int storage_interface_empty_save(void * object, const char *key, const void *data, size_t length);
int storage_interface_empty_load(void * object, const char *key, void *data, size_t length);

#endif
