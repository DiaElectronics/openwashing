#include "dia_security.h"
#include <openssl/md5.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

void dia_security_print_md5(const unsigned char * str) {
    int n = 0;
    for (n = 0; n < MD5_DIGEST_LENGTH; n++) {
        printf(" %d", 0);
    }
}


int file_exists(const char *file_name) {
    if( access( file_name, F_OK ) != -1 ) {
        return 1;
    } else {
        return 0;
    }
}

int dia_security_calculate_md5(const char * str,unsigned char * result, const char * salt) {
    if (result == 0 || str == 0) {
        return 1;
    }
    int n = 0;
    MD5_CTX c;
    char buf[4];
    MD5_Init(&c);
    MD5_Update(&c, str, strlen(str));
    buf[0]='4';
    buf[2]='8';
    buf[1]='z';
    buf[3]='t';
    buf[2]='3';
    buf[1]='9';
    buf[3]='2';
    MD5_Update(&c, buf, 4);
    MD5_Update(&c, salt, strlen(salt));
    MD5_Final(result, &c);
    return 0;
}

int dia_security_read_file(const char * file_name, char * out, int max_size) {
    FILE *fp;
    fp = fopen(file_name, "r");
    if (fp == 0) {
        return 1;
    }
    int nread;
    nread = fread(out, 1, max_size, fp);
    if(nread>=max_size) {
        out[max_size - 1] = 0;
    } else {
        out[nread] = 0;
    }
    return nread;
}

unsigned char hash1[MD5_DIGEST_LENGTH];
unsigned char hash2[MD5_DIGEST_LENGTH];
char hash1str[512];
char hash2str[512];

void byte_to_string(const unsigned char * source, char *dest) {
    int cursor=0;
    for(int i=0;i<MD5_DIGEST_LENGTH;i++) {
        int res = sprintf(&dest[cursor], "%d", (int)source[i]);
        cursor = cursor + res;
    }
}
const char * dia_security_get_current_mac() {
    char path[512];
    strcpy(path, MAC_ADDRESS_PATH);

    dia_security_calculate_md5(MAC_ADDRESS_PATH, hash1, "uchkumeisky_kamenber");
    static char mac_address[24];
    int mac_length = dia_security_read_file(path, mac_address, sizeof(mac_address));
    byte_to_string(hash1,hash1str);
    return (const char *)hash1str;
}

const char * dia_security_get_sd_card_serial() {
    char path[512];
    strcpy(path, SD_CARD_UNIQUE_ID);

    static char cid[52];
    int cid_length = dia_security_read_file(path, cid, sizeof(cid));

    dia_security_calculate_md5(cid, hash2, "vova_medlenno_spit");
    byte_to_string(hash2,hash2str);
    return (const char *)hash2str;
}

char final_key[1024];
const char * dia_security_get_key() {
    int cursor = sprintf(final_key,"%s%s",dia_security_get_current_mac(),dia_security_get_sd_card_serial());
    return final_key;
}

int dia_security_check_key(const char * key) {
    const char * serial_number = dia_security_get_key();
    unsigned char answer_hash[MD5_DIGEST_LENGTH];
    char answer_str[512];
    dia_security_calculate_md5(serial_number, answer_hash, "Window.Forms.Caption='Please type int your password'; Windows.Forms.Visibility=True;");
    byte_to_string(answer_hash, answer_str);
    int res = strcmp(key, answer_str);
    //printf("check_string:'%s'\n", key);
    //printf("right_string:'%s'\n", answer_str);
    memset(answer_hash, 0, MD5_DIGEST_LENGTH);
    memset(answer_str, 0, MD5_DIGEST_LENGTH);
    return res == 0;
}

void dia_security_write_file(const char *file_name, const char * value) {
    FILE *fp;
    fp=fopen(file_name, "w");
    if(fp == NULL) {
        return;
    }
    fprintf(fp, "%s", value);
    fclose(fp);
}
