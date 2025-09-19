#ifndef _COMMON_H_
#define _COMMON_H_

#include <string>
#include <chrono>
#include <cstring>

enum IPCType {
    PIPE,
    FIFO
};

enum RequestType {
    QUIT,
    DEPOSIT,
    WITHDRAW,
    BALANCE,
    UPLOAD_FILE,
    DOWNLOAD_FILE,
    LOGIN,
    LOGOUT,
    FAILURE
};

struct Request {
    RequestType type;
    int user_id;
    double amount;
    char filename[256];
    char data[1024];

    Request(RequestType t = QUIT, int uid = 0, double amt = 0.0, 
            const std::string& fname = "", const std::string& d = "") : 
            type(t), user_id(uid), amount(amt) {
        strncpy(filename, fname.c_str(), sizeof(filename)-1);
        strncpy(data, d.c_str(), sizeof(data)-1);
        filename[sizeof(filename)-1] = '\0';
        data[sizeof(data)-1] = '\0';
    }
};

struct Response {
    bool success;
    double balance;
    char data[1024];
    char message[256];

    Response(bool s = false, double b = 0.0, 
            const std::string& d = "", const std::string& m = "") :
            success(s), balance(b) {
        strncpy(data, d.c_str(), sizeof(data)-1);
        strncpy(message, m.c_str(), sizeof(message)-1);
        data[sizeof(data)-1] = '\0';
        message[sizeof(message)-1] = '\0';
    }
};

#endif