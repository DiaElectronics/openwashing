#include "dia_storage_interface.h"

#include <stdlib.h>
#include <stdio.h>

storage_interface_t * CreateEmptyInterface() {
    storage_interface_t * res = (storage_interface_t *)calloc(1, sizeof(storage_interface_t)); 
    res->next_object = 0;
    res->object = 0;
    res->save = storage_interface_empty_save;
    res->load = storage_interface_empty_load;
    res->is_real = 0;
    return res;
}

int storage_interface_empty_save(void * object, const char *key, const void *data, size_t length) {
    printf("empty save [%s]\n", key);
    return 1;
}
int storage_interface_empty_load(void * object, const char *key, void *data, size_t length) {
    printf("empty load [%s]\n", key);
    return 1;
}
