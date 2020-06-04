#ifndef _DIA_FUNCTIONS_H
#define _DIA_FUNCTIONS_H

int base64_encode(const unsigned char *data, size_t input_length, char *encoded_data, size_t buf_size);
int base64_decode(const char *data, size_t input_length, char *decoded_data, size_t buf_size);
void build_decoding_table();
void base64_cleanup();
#endif
