#include "common.h"
#include "channel.h"
#include <fstream>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <unistd.h>
#include <vector>

using namespace std;

string get_timestamp() {
    auto now = chrono::system_clock::now();
    time_t time = chrono::system_clock::to_time_t(now);
    stringstream ss;
    ss << put_time(localtime(&time), "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

int main(int argc, char* argv[]) {
    // Default log file if not specified
    string log_file = "system.log";
    IPCType ipc = PIPE;
    
    // Parse command line arguments
    for(int i = 1; i < argc; i++) {
        string arg = argv[i];
        if(arg == "-n" && i + 1 < argc) {
            log_file = argv[++i];
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
        channel = new FIFOChannel("logging", RequestChannel::SERVER_SIDE);
    } else {
        // TODO: Create the RequestChannel using PipeChannel
        channel = new PipeChannel(STDIN_FILENO, STDOUT_FILENO);
    }

    ofstream logfile(log_file, ios::app);

    while (true) {
        Request req = channel->receive_request();

        if (req.type == QUIT) {
            Response resp(true, 0, "", "Server shutting down");
            channel->send_response(resp);
            delete channel;
            exit(0);
        }

        logfile << "[" << req.user_id << "]: ";
        
        switch(req.type) {
            case LOGIN:
                logfile << "logged in";
                break;
            case LOGOUT:
                logfile << "logged out";
                break;
            case DEPOSIT:
                logfile << "deposited " << req.amount;
                break;
            case WITHDRAW:
                logfile << "withdrew " << req.amount;
                break;
            case BALANCE:
                logfile << "viewed balance: " << req.amount;
                break;
            case UPLOAD_FILE:
                logfile << "uploaded file: " << req.filename;
                break;
            case DOWNLOAD_FILE:
                logfile << "downloaded file: " << req.filename;
                break;
            default:
                logfile << "unknown action (type=" << req.type << ")";
        }
        logfile << endl;

        Response resp(true, 0, "", "Logged successfully");
        
        // Use the channel's send_response method instead of direct write
        channel->send_response(resp);
    }

    logfile.close();
    return 0;
}