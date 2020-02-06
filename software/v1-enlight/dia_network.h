#ifndef _DIA_NETWORK_LIB
#define _DIA_NETWORK_LIB

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <assert.h>
#include <curl/curl.h>
#include <unistd.h>
#include "dia_channel.h"
#include <new>
#include <list>
#include <map>
// for ubuntu use sudo apt-get install libjson-c-dev
//#include <json-c/json.h>
#include <jansson.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <arpa/inet.h>

#define MAX_RELAY_NUM 6
#define CHANNEL_SIZE 8192
#define RETRY_DELAY 5

#define MODE_NO_LOGIN 0
#define MODE_HAVE_LOGIN 1
#define MODE_HAVE_TOKEN 2

#define SERVER_UNAVAILABLE 2

class NetworkMessage {
    public:
    uint8_t resolved;
    unsigned long long entry_id;
    std::string json_request;
    std::string type_request;
    std::string time_stamp;
    uint64_t stime;
};

typedef struct curl_answer {
    char * data;
    size_t length;
} curl_answer_t;

typedef struct create_token {
  std::string login;
  std::string password;
} create_token_t;

typedef struct create_company {
  std::string name;
  std::string login;
  std::string password;
  std::string role;
} create_company_t;

typedef struct create_washstation {
  std::string name;
  int company_id;
} create_washstation_t;

typedef struct create_station_post {
  std::string name;
  std::string serialid;
  std::string login;
  int washstation_id;
} create_station_post_t;

typedef struct create_money_report {
  int id;
  int transaction_id;
  std::string station_post_id;
  std::string time_stamp;
  int cars_total;
  int coins_total;
  int banknotes_total;
  int cashless_total;
  int test_total;
} create_money_report_t;

typedef struct ping_report {
    std::string hash;
} ping_report_t

typedef struct RelayStat {
  int switched_count;
  int total_time_on;
} RelayStat_t;

typedef struct create_relay_report {
	int id;
  int transaction_id;
  std::string station_post_id;
  std::string time_stamp;
  RelayStat_t RelayStats[MAX_RELAY_NUM];
} create_relay_report_t;

class Promotion {
    public:
  int current_promotion_id;
  std::string code;
  std::map <std::string,std::string> required_details;
  std::string time_stamp_start;
  std::string time_stamp_end;
  std::string info;
} ;

class Registries {
    public:
  std::map <std::string,std::string> registries;
} ;

class DiaNetwork {
    public:
    
    DiaNetwork() {
        _Host = "";

        _OnlineCashRegister = "";
        _PublicKey = "";

        pthread_create(&entry_processing_thread, NULL, DiaNetwork::process_extract , this);
    }

    ~DiaNetwork() {
        printf("Destroying DiaNetwork\n");
        StopTheWorld();
        int status = pthread_join(entry_processing_thread,NULL);
        if (status != 0) {
            printf("Main error: can't join thread, status = %d\n", status);
        }
    }

    // Base function for sending a POST request.
    // Parameters: gets pre-created HTTP body, modifies answer from server, gets address of host (URL).
    int SendRequest(std::string *body, std::string *answer, std::string host_addr) {
        assert(body);
        assert(answer);
        CURL *curl;
        CURLcode res;
        curl_answer_t raw_answer;
        InitCurlAnswer(&raw_answer);

        curl_global_init(CURL_GLOBAL_ALL);
        curl = curl_easy_init();
        if (curl == NULL) {
            return 1;
        }

        struct curl_slist *headers = NULL;
        headers = curl_slist_append(headers, "Accept: application/json");
        headers = curl_slist_append(headers, "Content-Type: application/json");
        headers = curl_slist_append(headers, "charsets: utf-8");

        curl_easy_setopt(curl, CURLOPT_URL, host_addr.c_str());
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "POST");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body->c_str());
        curl_easy_setopt(curl, CURLOPT_USERAGENT, "diae/0.1");
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, this->_Writefunc);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &raw_answer);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, 10);

        res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            fprintf(stderr, "curl_easy_perform() failed: %s\n",curl_easy_strerror(res));
            if (res == CURLE_COULDNT_CONNECT) {
                return SERVER_UNAVAILABLE;
            }
            return 1;
        }
        *answer = raw_answer.data;
        fprintf(stderr, "Answer from server: %s\n",raw_answer.data);
        DestructCurlAnswer(&raw_answer);
        curl_easy_cleanup(curl);
        curl_slist_free_all(headers);
        curl_global_cleanup();
        return 0;
    }

    // Central server searching, if it's IP address is unknown.
    // Gets local machine's IP and starts pinging every address in block.
    // Modifies IP address.
    int SearchCentralServer(std::string *ip) {
	    char* tmpUrl = new char[16];            	    
        int fd;
        struct ifreq ifr;

        fd = socket(AF_INET, SOCK_DGRAM, 0);
        ifr.ifr_addr.sa_family = AF_INET;
        strncpy(ifr.ifr_name, "eth0", IFNAMSIZ-1);

        ioctl(fd, SIOCGIFADDR, &ifr);
        close(fd);

        sscanf(inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr), "%s", tmpUrl);

	    std::string baseIP(tmpUrl);
        std::string reqIP;

	    // Truncate baseIP to prepare for scan
        int dotCount = 0;
        for (int j = 0; j < 12; j++) {
            if (baseIP[j] == '.')
                dotCount++;
            reqIP += baseIP[j];
            if (dotCount == 3)
                break;
        }

        int err = 0;
        // Scan whole block
        for (int i = 1; i <= 255; i++) {
            std::string reqUrl = reqIP + std::to_string(i) + ":8020/ping";
            err = this->PingRequest(reqUrl);

            if (!err) {
                // We found it!
                *ip = reqIP + std::to_string(i);
                curl_easy_cleanup(curl);
                curl_global_cleanup();
                return 0;
            }
            curl_easy_cleanup(curl);            	
        }   	

	    delete[] tmpUrl;
        curl_global_cleanup();
        return SERVER_UNAVAILABLE;
    }

    // Returns local machine's MAC address of eth0 interface.
    // Modfifes out parameter == MAC address without ":" symbols.
    void GetMacAddress(char * out) {
   	    int fd;
	
	    struct ifreq ifr;
	    const char *iface = "eth0";
	    char *mac;
	
	    fd = socket(AF_INET, SOCK_DGRAM, 0);
	    ifr.ifr_addr.sa_family = AF_INET;
	    strncpy((char *)ifr.ifr_name , (const char *)iface , IFNAMSIZ-1);
	    ioctl(fd, SIOCGIFHWADDR, &ifr);

	    close(fd);
	
	    mac = (char *)ifr.ifr_hwaddr.sa_data;

	    sprintf((char *)out,(const char *)"%.2x%.2x%.2x%.2x%.2x%.2x" , mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    }

    // Just key setter.
    int SetPublicKey(std::string publicKey) {
        _PublicKey = publicKey;
        return 0;
    }

    // Sets Central Server host name.
    // Also sets same host name to Online Cash Register, which is located on the same server, as Central.
    int SetHostName(std::string hostName) {
        _Host = hostName;
        _OnlineCashRegister = hostName;
        return 0;
    }

    // Returns Central Server IP address.
    std::string GetCentralServerAddress() {
        std::string serverIP = "localhost";
        int res = -1;

	    printf("Looking for Central-wash server ...\n");
        res = this->SearchCentralServer(&serverIP);
        
        if (res == 0) {
            printf("Server located on:\n%s\n", serverIP.c_str());
        }
        else {
            printf("Failed: no server found...\n");
        }
    }

    // PING request to specified URL. 
    // Returns 0, if request was OK, other value - in case of failure.
    int PingRequest(std::string url) {
        std::string answer;

        ping_report_t ping_data;
        ping_data.hash = _PublicKey;

        int result;
        std::string json_ping_request = json_create_ping(&ping_data);
        result = SendRequest(&json_ping_request, &answer, url);

        if (result == 2) {
            printf("No connection to server, PING aborted...\n");
            return 3;
        }
        if (result) {
            return 1;
        }

        json_t *object;
        json_error_t error;

        int err = 0;
        object = json_loads(answer.c_str(), 0, &error);
        do {
            if (!object) {
                printf("Error in PING: %d: %s\n", error.line, error.text );
                err = 1;
                break;
            }

            if(!json_is_object(object)) {
                break;
            }

            json_t *obj_service_amount;
            obj_service_amount = json_object_get(object, "serviceAmount");
            if(json_is_object(obj_service_amount)) {
                int serviceMoney = (int)json_integer_value(obj_service_amount);
                
                // TODO: trasfer money somehow
                break;
            }
            
        } while (0);
        json_decref(object);
        return err;
    }

    // Sends receipt report directly to Online Cash Register.
    int ReceiptRequest(int postPosition, int isCard, int amount) {
        int res = 0;
        
        res = SendReceiptRequest(postPosition, isCard, amount);
        
        if (res > 0) {
            printf("No connection to server\n");
            return 1;
        }
        return 0;
    }

    // Base function for receipt sending to Online Cash Register.
    int SendReceiptRequest(int postPosition, int isCard, int amount) {
        CURL *curl;
        CURLcode res;

        curl_global_init(CURL_GLOBAL_ALL);
        
        curl = curl_easy_init();
        if (curl == NULL) {
            return 1;
        }

        std::string reqUrl;	    
        reqUrl = "https://" + _OnlineCashRegister + ":8443/";
        reqUrl += std::to_string(postPosition) + "/" + std::to_string(amount) + "/" + std::to_string(isCard);

        curl_easy_setopt(curl, CURLOPT_URL, reqUrl.c_str());
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PUT");
        curl_easy_setopt(curl, CURLOPT_USERAGENT, "diae/0.1");
	    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
	    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0);

        res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
	        printf("%s", curl_easy_strerror(res));
	        printf("\n");
            curl_easy_cleanup(curl);
            curl_global_cleanup();
            return SERVER_UNAVAILABLE;
        }

        curl_easy_cleanup(curl);
        curl_global_cleanup();
        return 0;
    }

    int create_money_report(int transaction_id,std::string station_post_id,std::string time_stamp,int cars_total,int coins_total,int banknotes_total,int cashless_total,int test_total) {
        std::string answer;
        // Create classes and constructors
        create_money_report_t money_report_data={0,0,"","",0,0,0,0};
        //int res=0;
        money_report_data.transaction_id = transaction_id;
        money_report_data.station_post_id = station_post_id;
        money_report_data.time_stamp = time_stamp;
        money_report_data.cars_total = cars_total;
        money_report_data.coins_total=coins_total;
        money_report_data.banknotes_total=banknotes_total;
        money_report_data.cashless_total=cashless_total;
        money_report_data.test_total=test_total;

        std::string json_money_report_request = json_create_money_report(&money_report_data);
        CreateAndPushEntry(json_money_report_request);
        return 0;
    }

    int create_relay_report(int transaction_id,std::string station_post_id,std::string time_stamp,struct RelayStat RelayStats[MAX_RELAY_NUM]) {
        std::string answer;
        create_relay_report_t relay_report_data={0,0,"","",{{0,0},{0,0},{0,0},{0,0},{0,0},{0,0}}};
        relay_report_data.transaction_id = transaction_id;
        relay_report_data.station_post_id = station_post_id;
        relay_report_data.time_stamp = time_stamp;

        for(int i = 0; i < MAX_RELAY_NUM; i ++) {
            relay_report_data.RelayStats[i].switched_count = RelayStats[i].switched_count;
            relay_report_data.RelayStats[i].total_time_on = RelayStats[i].total_time_on;
        }

        std::string json_relay_report_request = json_create_relay_report(&relay_report_data);
        CreateAndPushEntry(json_relay_report_request);
        return 0;
    }

    int get_last_money_report(create_money_report_t *money_report_data) {
        std::string answer;
        int attempts = 10;
        while(_Mode!=MODE_HAVE_TOKEN && attempts>0) {
            usleep(1000000);
            printf("waiting... current mode is %d\n", _Mode);
            attempts--;
        }
        std::string json_get_last_money_report_request = json_get_last_money_report();
        int res=0;
        if (!_Token.empty()) {
            res=SendRequest(&json_get_last_money_report_request, &answer);
        }
        else {
            printf("Token empty\n");
            return 1;
        }
        if (res>0) {
            printf("No connection to server\n");
            return 1;
        }

        std::size_t found = answer.find("{\"errors\":[{\"message\":");
        if (found!=std::string::npos) {
            return 2;
        }
        json_t *root;
        json_error_t error;
        root = json_loads(answer.c_str(), 0, &error);
        int err = 0;

        do {
            if ( !root ) {
                printf("error in get_last_money_report: on line %d: %s\n", error.line, error.text );
                err = 1;
                break;
            }
            if(!json_is_object(root)) {
                err = 1;
                break;
            }
            json_t *obj_data;
            obj_data = json_object_get(root, "data" );
            if(!json_is_object(obj_data)) {
                err = 1;
                break;
            }
            json_t *obj_post;
            obj_post = json_object_get(obj_data, "GetLastMoneyReport" );
            if(!json_is_object(obj_post)) {
                err = 1;
                break;
            }

            json_t *obj_tek;
            obj_tek = json_object_get(obj_post, "ID" );
            if(!json_is_integer(obj_tek)) {
                err = 1;
                break;
            }
            money_report_data->id = json_integer_value(obj_tek);

            obj_tek = json_object_get(obj_post, "TransactionID" );
            if(!json_is_integer(obj_tek)) {
                err = 1;
                break;
            }
            money_report_data->transaction_id = json_integer_value(obj_tek);

            obj_tek = json_object_get(obj_post, "StationPostID" );
            if(!json_is_integer(obj_tek)) {
                err = 1;
                break;
            }
            money_report_data->station_post_id = std::to_string( json_integer_value(obj_tek));

            obj_tek = json_object_get(obj_post, "TimeStamp" );
            if(!json_is_integer(obj_tek)) {
                err = 1;
                break;
            }
            money_report_data->time_stamp =std::to_string( json_integer_value(obj_tek));

            obj_tek = json_object_get(obj_post, "CarsTotal" );
            if(!json_is_integer(obj_tek)) {
                err = 1;
                break;
            }
            money_report_data->cars_total = json_integer_value(obj_tek);

            obj_tek = json_object_get(obj_post, "CoinsTotal" );
            if(!json_is_integer(obj_tek)) {
                err = 1;
                break;
            }
            money_report_data->coins_total = json_integer_value(obj_tek);

            obj_tek = json_object_get(obj_post, "BanknotesTotal" );
            if(!json_is_integer(obj_tek)) {
                err = 1;
                break;
            }
            money_report_data->banknotes_total = json_integer_value(obj_tek);

            obj_tek = json_object_get(obj_post, "CashlessTotal" );
            if(!json_is_integer(obj_tek)) {
                err = 1;
                break;
            }
            money_report_data->cashless_total = json_integer_value(obj_tek);

            obj_tek = json_object_get(obj_post, "TestTotal" );
            if(!json_is_integer(obj_tek)) {
                err = 1;
                break;
            }
            money_report_data->test_total = json_integer_value(obj_tek);
        } while(0);
        json_decref(root);
        return err;
    }

    int get_last_relay_report(create_relay_report_t *relay_report_data) {
        std::string answer;
        int attempts = 10;
        while(_Mode!=MODE_HAVE_TOKEN && attempts>0) {
            usleep(1000000);
            printf("waiting... current mode is %d\n", _Mode);
            attempts--;
        }
        std::string json_get_last_relay_report_request = json_get_last_relay_report();
        int res=0;
        if (!_Token.empty()) {
            res=SendRequest(&json_get_last_relay_report_request, &answer);
        }
        else {
            printf("Token empty\n");
            return 1;
        }
        if (res>0) {
            printf("No connection to server\n");
            return 1;
        }

        std::size_t found = answer.find("{\"errors\":[{\"message\":");
        if (found!=std::string::npos) {
            return 2;
        }
        fprintf(stderr,"%s",answer.c_str());
        json_t *root;
        json_error_t error;
        root = json_loads(answer.c_str(), 0, &error);
        int err = 0;

        do {
            if ( !root ) {
                printf("error in get_last_relay_report: on line %d: %s\n", error.line, error.text );
                err = 1;
                break;
            }
            if(!json_is_object(root)) {
                err = 1;
                break;
            }
            json_t *obj_data;
            obj_data = json_object_get(root, "data" );
            if(!json_is_object(obj_data)) {
                err = 1;
                break;
            }
            json_t *obj_post;
            obj_post = json_object_get(obj_data, "GetLastRelayReport" );
            if(!json_is_object(obj_post)) {
                err = 1;
                break;
            }

            json_t *obj_tek;
            obj_tek = json_object_get(obj_post, "ID" );
            if(!json_is_integer(obj_tek)) {
                err = 1;
                break;
            }
            relay_report_data->id = json_integer_value(obj_tek);

            obj_tek = json_object_get(obj_post, "TransactionID" );
            if(!json_is_integer(obj_tek)) {
                err = 1;
                break;
            }
            relay_report_data->transaction_id = json_integer_value(obj_tek);

            obj_tek = json_object_get(obj_post, "StationPostID" );
            if(!json_is_integer(obj_tek)) {
                err = 1;
                break;
            }
            relay_report_data->station_post_id = std::to_string( json_integer_value(obj_tek));

            obj_tek = json_object_get(obj_post, "TimeStamp" );
            if(!json_is_integer(obj_tek)) {
                err = 1;
                break;
            }
            relay_report_data->time_stamp =std::to_string( json_integer_value(obj_tek));


            obj_tek = json_object_get(obj_post, "RelayStats" );
            if(!json_is_array(obj_tek)) {
                err = 1;
                break;
            }
            json_t *element;
            json_t *obj_for_array;
            int i,relay_id,switched_count,total_time_on;


            json_array_foreach(obj_tek,i, element){
                obj_for_array = json_object_get(element, "RelayID" );
                if(!json_is_integer(obj_for_array)) {
                    err = 1;
                    break;
                }
                relay_id = json_integer_value(obj_for_array);

                obj_for_array = json_object_get(element, "SwitchedCount" );
                if(!json_is_integer(obj_for_array)) {
                    err = 1;
                    break;
                }
               switched_count = json_integer_value(obj_for_array);

                obj_for_array = json_object_get(element, "TotalTimeOn" );
                if(!json_is_integer(obj_for_array)) {
                    err = 1;
                    break;
                }
               total_time_on = json_integer_value(obj_for_array);
               relay_report_data->RelayStats[relay_id-1].switched_count =switched_count;
               relay_report_data->RelayStats[relay_id-1].total_time_on =total_time_on;
            }
        } while(0);
        json_decref(root);
        return err;
    }

    int create_error_report(std::string message) {
        std::string answer;
        int res=0;
        std::string mymessage=message;

        std::string json_error_report_request = json_create_error(mymessage);
         if (!_Token.empty()) {
            res=SendRequest(&json_error_report_request, &answer);
        }
        else {
            printf("Token empty\n");
            CreateAndPushEntry(json_error_report_request);
            return 1;
        }
        if (res>0) {
            printf("No connection to server\n");
            CreateAndPushEntry(json_error_report_request);
            return 1;
        }

        return 0;
    }

    int RegisterPostWash(std::string name, std::string serialid, std::string *password, std::string *keystr) {
        assert(password);
        *password = "";
        assert(keystr);
        *keystr = "";
        std::string answer;
        create_station_post_t station_post_data={"","","",0};
        int res=0;
        station_post_data.name = name;
        station_post_data.serialid = serialid;
        station_post_data.login = "";
        station_post_data.washstation_id=0;

        int attempts = 10;
        while(_Mode!=MODE_HAVE_TOKEN && attempts>0) {
            usleep(1000000);
            printf("waiting... current mode is %d\n", _Mode);
            attempts--;
        }

        std::string create_station_post_request = json_create_station_post(&station_post_data);

        if (!_Token.empty()) {
            res=SendRequest(&create_station_post_request, &answer);
        }
        else {
            printf("Token empty\n");
            return 1;
        }
        if (res>0) {
            printf("No connection to server\n");
            return 1;
        }

        // XXX че, серьезно???? ты привязываешься к строгому порядку байт текущего ответа???
        std::size_t found = answer.find("{\"errors\":[{\"message\":");
        if (found!=std::string::npos) {
            return 2;
        }
        json_t *root;
        json_error_t error;
        root = json_loads(answer.c_str(), 0, &error);
        int err = 0;
        do {
            if ( !root ) {
                printf("error in CreateStationPost: on line %d: %s\n", error.line, error.text );
                err = 1;
                break;
            }
            if(!json_is_object(root)) {
                err = 1;
                break;
            }
            json_t *obj_data;
            obj_data = json_object_get(root, "data" );
            if(!json_is_object(obj_data)) {
                err = 1;
                break;
            }
            json_t *obj_post;
            obj_post = json_object_get(obj_data, "CreateStationPost" );
            if(!json_is_object(obj_post)) {
                err = 1;
                break;
            }

            json_t *obj_password;
            obj_password = json_object_get(obj_post, "Password" );
            if(!json_is_string(obj_password)) {
                err = 1;
                break;
            }
            const char* data_password = json_string_value(obj_password);

            json_t *obj_keystr;
            obj_keystr = json_object_get(obj_post, "KeyStr" );
            if(!json_is_string(obj_keystr)) {
                err = 1;
                break;
            }
            const char* data_keystr = json_string_value(obj_keystr);

            *password =  data_password;
            *keystr = data_keystr;
        } while(0);
        json_decref(root);

        return err;
    }

int MyPromotions(std::list<Promotion> *PromotionList) {
        std::string answer;
        int res=0;
        int attempts = 10;
        while(_Mode!=MODE_HAVE_TOKEN && attempts>0) {
            usleep(1000000);
            printf("waiting... current mode is %d\n", _Mode);
            attempts--;
        }
        std::string get_my_promotions = json_get_my_promotions();
        if (!_Token.empty()) {
            res=SendRequest(&get_my_promotions, &answer);
        }
        else {
            printf("Token empty\n");
            return 1;
        }
        if (res>0) {
            printf("No connection to server\n");
            return 1;
        }

        std::list<Promotion> prom_list;
        std::size_t found = answer.find("{\"errors\":[{\"message\":");
        if (found!=std::string::npos) {
            return 2;
        }
        json_t *root,*req;
        json_error_t error;
        root = json_loads(answer.c_str(), 0, &error);
        int err = 0;
        do {
            if ( !root ) {
                printf("error in MyPromotion: on line %d: %s\n", error.line, error.text );
                err = 1;
                break;
            }
            if(!json_is_object(root)) {
                err = 1;
                break;
            }
            json_t *obj_data;
            obj_data = json_object_get(root, "data" );
            if(!json_is_object(obj_data)) {
                err = 1;
                break;
            }
            json_t *obj_post;
            obj_post = json_object_get(obj_data, "GetMyPromotions" );
            if(!json_is_object(obj_post)) {
                err = 1;
                break;
            }

            json_t *obj_my_promotions;
            obj_my_promotions = json_object_get(obj_post, "MyPromotions" );
            if(!json_is_array(obj_my_promotions)) {
                err = 1;
                break;
            }
            json_t *element;
            json_t *obj_for_array,*value;
            int i;


            json_array_foreach(obj_my_promotions,i, element){
                Promotion* prom=new Promotion;
                obj_for_array = json_object_get(element, "CurrentPromotionID" );
                if(!json_is_integer(obj_for_array)) {
                    err = 1;
                    break;
                }
                prom->current_promotion_id = json_integer_value(obj_for_array);

                obj_for_array = json_object_get(element, "Code" );
                if(!json_is_string(obj_for_array)) {
                    err = 1;
                    break;
                }
               prom->code = json_string_value(obj_for_array);

                obj_for_array = json_object_get(element, "RequiredDetails" );
                if(!json_is_string(obj_for_array)) {
                    err = 1;
                    break;
                }
                req = json_loads(json_string_value(obj_for_array), 0, &error);
                if ( !req ) {
                    printf("error in MyPromotion: on line %d: %s\n", error.line, error.text );
                    err = 1;
                    break;
                }
                if(!json_is_object(req)) {
                    err = 1;
                    break;
                }
                const char* key;
                json_object_foreach(req, key, value) {
                        if(!json_is_string(value)) {
                            err = 1;
                            break;
                        }
                        prom->required_details.insert(std::pair<std::string, std::string>(key,json_string_value(value)));
                 };
                 json_decref(req);

                obj_for_array = json_object_get(element, "TimeStampStart" );
                if(!json_is_integer(obj_for_array)) {
                    err = 1;
                    break;
                }
                prom->time_stamp_start = std::to_string(json_integer_value(obj_for_array));

                obj_for_array = json_object_get(element, "TimeStampEnd" );
                if(!json_is_integer(obj_for_array)) {
                    err = 1;
                    break;
                }
                prom->time_stamp_end = std::to_string(json_integer_value(obj_for_array));

                obj_for_array = json_object_get(element, "Info" );
                if(!json_is_string(obj_for_array)) {
                    err = 1;
                    break;
                }
                prom->info = json_string_value(obj_for_array);
                PromotionList->push_back(*prom);
                delete prom;
            }


        } while(0);
        json_decref(root);

        return err;
    }

int MyRegistry(Registries *MyRegistries) {
        std::string answer;
        int res=0;
        int attempts = 10;
        while(_Mode!=MODE_HAVE_TOKEN && attempts>0) {
            usleep(1000000);
            printf("waiting... current mode is %d\n", _Mode);
            attempts--;
        }
        std::string get_my_registry = json_get_my_registry();
        if (!_Token.empty()) {
            res=SendRequest(&get_my_registry, &answer);
        }
        else {
            printf("Token empty\n");
            return 1;
        }
        if (res>0) {
            printf("No connection to server\n");
            return 1;
        }
        int err = 0;
        std::size_t found = answer.find("{\"errors\":[{\"message\":");
        if (found!=std::string::npos) {
            return 2;
        }
        json_t *root;
        json_error_t error;
        root = json_loads(answer.c_str(), 0, &error);
        do {
            if ( !root ) {
                printf("error in MyRegistry: on line %d: %s\n", error.line, error.text );
                err = 1;
                break;
            }
            if(!json_is_object(root)) {
                err = 1;
                break;
            }
            json_t *obj_data;
            obj_data = json_object_get(root, "data" );
            if(!json_is_object(obj_data)) {
                err = 1;
                break;
            }
            json_t *obj_post;
            obj_post = json_object_get(obj_data, "GetMyRegistry" );
            if(!json_is_object(obj_post)) {
                err = 1;
                break;
            }

            json_t *obj_my_promotions;
            obj_my_promotions = json_object_get(obj_post, "MyRegistrys" );
            if(!json_is_array(obj_my_promotions)) {
                err = 1;
                break;
            }
            json_t *element;
            json_t *obj_for_array;
            int i;
            std::string key,value;

            json_array_foreach(obj_my_promotions,i, element){
                obj_for_array = json_object_get(element, "Key" );
                if(!json_is_string(obj_for_array)) {
                    err = 1;
                    break;
                }
                key = json_string_value(obj_for_array);

                obj_for_array = json_object_get(element, "Value" );
                if(!json_is_string(obj_for_array)) {
                    err = 1;
                    break;
                }
               value = json_string_value(obj_for_array);
                MyRegistries->registries.insert(std::pair<std::string, std::string>(key,value));
            }
        } while(0);
        json_decref(root);
        return err;
    }

    int interrupted = 0;

    private:

    std::string _PublicKey;
    std::string _OnlineCashRegister;
    std::string _Host;

    int _Mode;
    DiaChannel<NetworkMessage> channel;
    pthread_t entry_processing_thread;
    pthread_mutex_t nfct_entries_mutex = PTHREAD_MUTEX_INITIALIZER;


    static void *process_extract(void *arg) {
        DiaNetwork *Dia = (DiaNetwork*) arg;

        while(!Dia->interrupted) {
            if (Dia->_Mode==MODE_HAVE_TOKEN) {
                int res = Dia->ModeHaveToken();
                if (res == SERVER_UNAVAILABLE) {
                    usleep(5000000); //let's sleep for 5 second before the next attempt
                }
            } else if (Dia->_Mode==MODE_HAVE_LOGIN) {
                int res = Dia->ModeHaveLogin();
                if (res) {
                    usleep(5000000);
                }
            } else if (Dia->_Mode==MODE_NO_LOGIN) {
                usleep(1000000);
            }
        }
        pthread_exit(NULL);
        return NULL;
    }

    int StopTheWorld() {
        interrupted = 1;
        return 0;
    }

    int ModeHaveLogin() {
        printf("000 MODE_HAVE_LOGIN\n");
        int err=LoginRequest();
        if (err==1) {
            printf("000 another Error login to server\n");
        } else if (err==2) {
            printf("000 Incorrect login or password\n");
            _Mode=MODE_NO_LOGIN;
            _Login="";
            _Password="";
        } else if (err==3) {
            printf("000 login No connection to server\n");
        } else {
            printf("000 logged in properly\n");
        }
        return err;
    }
    int ModeHaveToken() {
        NetworkMessage * message;
        int err = channel.Pop(&message);
        if (!err) {
            if(message && message->json_request!="") {
                std::string answer;
                int res=SendRequest(&(message->json_request), &answer);
                //let's ignore the answer for now;
                if (res==SERVER_UNAVAILABLE) { // server is not answering
                    printf("ERR: SERVER IS NOT AVAILABLE\n");
                    return SERVER_UNAVAILABLE;
                }
                free(message);
                return res;
            } else {
                printf("ERROR: somethings wrong with the channel");
            }
        } else {
            sleep(1);
            //no elements in channel
        }
        return err;
    }

    int CreateAndPushEntry(std::string json_string){
        NetworkMessage * entry = new NetworkMessage();
        if(!entry) {
            return 1;
        }
        entry->stime = time(NULL);
        entry->json_request = json_string;
        if (channel.Push(entry)==CHANNEL_BUFFER_OVERFLOW) {
            printf("CHANNEL BUFFER OVERFLOW\n");
            return 1;
        }
        else {
            return 0;
        }
    }

    std::string json_create_token(struct create_token *s) {
        json_t *root = json_object();
        json_t *myVar = json_object();
        json_t *variables = json_object();

        json_object_set_new(myVar, "Login", json_string(s->login.c_str()));
        json_object_set_new(myVar, "Password",json_string(s->password.c_str()));
        json_object_set_new(variables, "myVar",myVar);
        json_object_set_new(root, "operationName", json_null());
        json_object_set_new(root, "variables",variables);
        json_object_set_new(root, "query",json_string("query($myVar: TokenCreateRequest!){\n  CreateToken(Request: $myVar)\n}\n"));
        char *str = json_dumps(root, 0);
        std::string res = str;
        free(str);
        json_decref(root);
        return res;
    }

    std::string json_create_error(std::string s) {
        json_t *root = json_object();
        json_t *myVar = json_object();
        json_t *variables = json_object();

        json_object_set_new(myVar, "Message", json_string(s.c_str()));
        json_object_set_new(variables, "myVar",myVar);
        json_object_set_new(root, "operationName", json_null());
        json_object_set_new(root, "variables",variables);
        json_object_set_new(root, "query",json_string("mutation($myVar: ErrorReportCreateRequest!){\n  CreateErrorReport(Request: $myVar)\n{ID Message TimeStamp} }\n"));
        char *str = json_dumps(root, 0);
        std::string res = str;
        free(str);
        json_decref(root);
        return res;
    }

    std::string json_create_company(struct create_company *s) {
        char jsonObj[500];
        const char* pattern="{\"operationName\":null,\"variables\":{\"myVar\":{\"Name\": \"%s\",\"Login\": \"%s\", \"Password\": \"%s\", \"Role\": \"%s\"}},\"query\":\"mutation($myVar: CompanysCreateRequest!){\\n  CreateCompany(Request: $myVar)\\n{ID}}\\n\"}";
        sprintf(jsonObj,pattern,s->name.c_str(), s->login.c_str() ,s->password.c_str(),s->role.c_str());
        return jsonObj;
    }

    std::string json_create_washstation(struct create_washstation *s) {
        char jsonObj[500];
        const char* pattern="{\"operationName\":null,\"variables\":{\"myVar\":{\"Name\": \"%s\",\"CompanyID\": %d}},\"query\":\"mutation($myVar: WashStationsCreateRequest!){\\n  CreateWashStation(Request: $myVar)\\n{ID}}\\n\"}";
        sprintf(jsonObj,pattern,s->name.c_str(), s->company_id);
        return jsonObj;
    }

    std::string json_get_my_registry() {
        char jsonObj[500];
        const char* pattern="{\"operationName\":null,\"variables\":{\"myVar\":{\"Key\": \"%s\"}},\"query\":\"query($myVar: MyRegistryRequest!){\\n  GetMyRegistry(Request: $myVar)\\n {MyRegistrys{Key Value}}}\\n\"}";
        sprintf(jsonObj,pattern,"1");
        return jsonObj;
    }

    std::string json_create_station_post(struct create_station_post *s) {
        json_t *root = json_object();
        json_t *myVar = json_object();
        json_t *variables = json_object();

        json_object_set_new(myVar, "Name", json_string(s->name.c_str()));
        json_object_set_new(myVar, "SerialID",json_string(s->serialid.c_str()));
        json_object_set_new(myVar, "Login", json_string(s->login.c_str()));
        json_object_set_new(myVar, "WashStationID",json_integer(s->washstation_id));
        json_object_set_new(variables, "myVar",myVar);
        json_object_set_new(root, "operationName", json_null());
        json_object_set_new(root, "variables",variables);
        json_object_set_new(root, "query",json_string("mutation($myVar: StationPostsCreateRequest!){\n  CreateStationPost(Request: $myVar)\n{ID Name SerialID CompanyID WashStationID Password KeyStr}}\n"));
        char *str = json_dumps(root, 0);
        std::string res = str;
        free(str);
        json_decref(root);
        return res;
    }

    std::string json_create_money_report(struct create_money_report *s) {
        json_t *root = json_object();
        json_t *myVar = json_object();
        json_t *variables = json_object();

        json_object_set_new(myVar, "TransactionID", json_integer(s->transaction_id));
        json_object_set_new(myVar, "StationPostID", json_string(s->station_post_id.c_str()));
        json_object_set_new(myVar, "TimeStamp",json_integer(std::stoi( s->time_stamp)));
        json_object_set_new(myVar, "CarsTotal", json_integer(s->cars_total));
        json_object_set_new(myVar, "CoinsTotal", json_integer(s->coins_total));
        json_object_set_new(myVar, "BanknotesTotal",json_integer(s->banknotes_total));
        json_object_set_new(myVar, "CashlessTotal",json_integer(s->cashless_total));
        json_object_set_new(myVar, "TestTotal",json_integer(s->test_total));
        json_object_set_new(variables, "myVar",myVar);
        json_object_set_new(root, "operationName", json_null());
        json_object_set_new(root, "variables",variables);
        json_object_set_new(root, "query",json_string("mutation($myVar:MoneyReportsCreateRequest!) {\n  CreateMoneyReport(Request: $myVar) {\n    ID\n TransactionID\n StationPostID\n TimeStamp\n CarsTotal\n CoinsTotal\n BanknotesTotal\n CashlessTotal\n TestTotal\n    }\n}\n"));
        char *str = json_dumps(root, 0);
        std::string res = str;
        free(str);
        json_decref(root);
        return res;
    }

    std::string json_create_relay_report(struct create_relay_report *s) {
        json_t *root = json_object();
        json_t *myVar = json_object();
        json_t *variables = json_object();
        json_t *relayarr = json_array();
        json_t *relayobj[MAX_RELAY_NUM];

        for(int i = 0; i < MAX_RELAY_NUM; i ++) {
            relayobj[i]=json_object();
            json_object_set_new(relayobj[i], "RelayID",json_integer(i+1));
            json_object_set_new(relayobj[i], "SwitchedCount",json_integer(s->RelayStats[i].switched_count));
            json_object_set_new(relayobj[i], "TotalTimeOn",json_integer(s->RelayStats[i].total_time_on));
            json_array_append_new(relayarr, relayobj[i]);
        }
        json_object_set_new(myVar, "TransactionID", json_integer(s->transaction_id));
        json_object_set_new(myVar, "StationPostID", json_string(s->station_post_id.c_str()));
        json_object_set_new(myVar, "TimeStamp",json_integer(std::stoi( s->time_stamp)));
        json_object_set_new(myVar, "StationPostID", json_string(s->station_post_id.c_str()));
        json_object_set_new(myVar, "RelayStats", relayarr);
        json_object_set_new(variables, "myVar",myVar);
        json_object_set_new(root, "operationName", json_null());
        json_object_set_new(root, "variables",variables);
        json_object_set_new(root, "query",json_string("mutation($myVar:RelayReportsCreateRequest!) {\n  CreateRelayReport(Request:$myVar) {\n    ID\n TransactionID\n    StationPostID\n    TimeStamp\n    RelayStats {\n      RelayID\n      SwitchedCount\n      TotalTimeOn\n    }\n  }\n}\n"));
        char *str = json_dumps(root, 0);
        std::string res = str;
        free(str);
        json_decref(root);
        return res;
    }

    std::string json_get_last_money_report() {
        json_t *root = json_object();
        json_t *myVar = json_object();
        json_t *variables = json_object();

        json_object_set_new(myVar, "Total",json_integer(1));
        json_object_set_new(variables, "myVar",myVar);
        json_object_set_new(root, "operationName", json_null());
        json_object_set_new(root, "variables",variables);
        json_object_set_new(root, "query",json_string("query($myVar:EmptyRequest!) {\n  GetLastMoneyReport(Request: $myVar) {\n    ID\n TransactionID\n StationPostID\n TimeStamp\n CarsTotal\n CoinsTotal\n BanknotesTotal\n CashlessTotal\n TestTotal\n    }\n}\n"));
        char *str = json_dumps(root, 0);
        std::string res = str;
        free(str);
        json_decref(root);
        return res;
    }

    std::string json_get_last_relay_report() {
        json_t *root = json_object();
        json_t *myVar = json_object();
        json_t *variables = json_object();
        json_object_set_new(myVar, "Total", json_integer(1));
        json_object_set_new(variables, "myVar",myVar);
        json_object_set_new(root, "operationName", json_null());
        json_object_set_new(root, "variables",variables);
        json_object_set_new(root, "query",json_string("query($myVar:EmptyRequest!) {\n  GetLastRelayReport(Request:$myVar) {\n    ID\n TransactionID\n   StationPostID\n    TimeStamp\n    RelayStats {\n      RelayID\n      SwitchedCount\n      TotalTimeOn\n    }\n  }\n}\n"));
        char *str = json_dumps(root, 0);
        std::string res = str;
        free(str);
        json_decref(root);
        return res;
    }

    std::string json_get_my_promotions() {
        char jsonObj[500];
        const char* pattern="{\"operationName\":null,\"variables\":{\"myVar\":{\"StationPostID\": %s}},\"query\":\"query($myVar: MyPromotionRequest!){\\n  GetMyPromotions(Request: $myVar)\\n {MyPromotions{CurrentPromotionID Code RequiredDetails TimeStampStart TimeStampEnd Info}}}\\n\"}";
        sprintf(jsonObj,pattern,"1");
        return jsonObj;
    }

    static size_t _Writefunc(void *ptr, size_t size, size_t nmemb, curl_answer_t *answer) {
        assert(answer);
         size_t new_len = answer->length + size*nmemb;
        answer->data = (char*)realloc(answer->data, new_len+1);
        assert(answer->data);
        memcpy(answer->data+answer->length, ptr, size*nmemb);
        answer->data[new_len] = '\0';
        answer->length = new_len;

        return size*nmemb;
    }

    void InitCurlAnswer(curl_answer_t * raw_answer) {
        raw_answer->data = (char *) malloc(1);
        raw_answer->length = 0;
        raw_answer->data[0] = 0;
    }

    void DestructCurlAnswer(curl_answer_t * raw_answer) {
        free(raw_answer->data);
    }
};

#endif
