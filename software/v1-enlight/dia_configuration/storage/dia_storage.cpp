#include "dia_storage.h"

int DiaStore_Load(void * object, const char *key, void *data, size_t length) {
    DiaStore * store = (DiaStore *) object;
    return store->Load(key, data, length);
}

int DiaStore_Save(void * object, const char *key, const void *data, size_t length) {
    DiaStore * store = (DiaStore *) object;
    return store->Save(key, data, length);
}
