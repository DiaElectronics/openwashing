#ifndef _DIA_SECURITY_H
#define _DIA_SECURITY_H

#define MAC_ADDRESS_PATH "/sys/class/net/eth0/address"
#define SD_CARD_UNIQUE_ID "/sys/block/mmcblk0/device/cid"

#include <openssl/md5.h>

void dia_security_print_md5(const unsigned char * str);
int dia_security_calculate_md5(const char * str, unsigned char * result, int max_size);

const char * dia_security_get_current_mac();

int read_file(const char * file_name, char * out);

const char * dia_security_get_sd_card_serial();

const char * dia_security_get_key();
int dia_security_check_key(const char * key);

int file_exists(const char *file_name);
void dia_security_write_file(const char *file_name, const char * value);
int dia_security_read_file(const char * file_name, char * out, int max_size);

void dia_security_generate_public_key(char *out, int max_size);
#endif
