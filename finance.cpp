#include "common.h"
#include "channel.h"
#include <iostream>
#include <unistd.h>
#include <vector>

using namespace std;

class Account {
public:
    int id;
    double balance;
    bool active;
    Account() : id(-1), balance(0.0), active(false) {}  // Used only for array initialization
    Account(int _id) : id(_id), balance(0.0), active(true) {}
};

#define MAX_ACCOUNTS 100  // Or whatever reasonable limit we want to set

int main(int argc, char* argv[]) {
    // Default max accounts if not specified
    int max_accounts = 100;
    IPCType ipc = PIPE;
    
    // Parse command line arguments
    for(int i = 1; i < argc; i++) {
        string arg = argv[i];
        if(arg == "-m" && i + 1 < argc) {
            max_accounts = atoi(argv[++i]) + 1;
        }
        else if(arg == "-p") {
            ipc = PIPE;
        }
        else if(arg == "-f") {
            ipc = FIFO;
        }
    }

    RequestChannel* channel;
    if (ipc == FIFO) {
        // TODO: Create the RequestChannel using FIFOChannel
    } else {
        // TODO: Create the RequestChannel using PipeChannel
    }

    Account* accounts = new Account[max_accounts];
    
    while (true) {
        Request req = channel->receive_request();

        if (req.type == QUIT) {
            Response resp(true, 0, "", "Server shutting down");
            channel->send_response(resp);
            delete channel;
            delete[] accounts;
            exit(0);
        }

        Response resp;
        resp.success = true;

        if (req.user_id < 0 || req.user_id >= max_accounts) {
            resp = Response(false, 0, "", "Invalid account ID");
            channel->send_response(resp);
            continue;
        }

        // Create account if it doesn't exist
        if (!accounts[req.user_id].active) {
            accounts[req.user_id] = Account(req.user_id);
        }

        Account& acc = accounts[req.user_id];
        
        if (req.type == DEPOSIT) {
            acc.balance += req.amount;
            resp = Response(true, acc.balance, "", "Deposit successful");
        } 
        else if (req.type == WITHDRAW) {
            if (acc.balance >= req.amount) {
                acc.balance -= req.amount;
                resp = Response(true, acc.balance, "", "Withdrawal successful");
            } else {
                resp = Response(false, acc.balance, "", "Insufficient funds");
            }
        }
        else if (req.type == BALANCE) {
            resp = Response(true, acc.balance, "", "View balance successful");
        }
        else {
            resp = Response(false, 0, "", "Invalid request type");
        }

        channel->send_response(resp);
    }
    return 0;
}