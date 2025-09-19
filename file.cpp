#include "common.h"
#include "channel.h"
#include <fstream>
#include <iostream>
#include <unistd.h>
#include <sstream>
#include <vector>

using namespace std;

int main(int argc, char* argv[]) {
    vector<string> allowed_extensions;
    IPCType ipc = PIPE;
    
    // Get allowed extensions from command line arguments
    // No flag needed, all arguments are extensions
    for (int i = 1; i < argc; i++) {
        string arg = argv[i];
        if(arg == "-p") {
            ipc = PIPE;
        }
        else if(arg == "-f") {
            ipc = FIFO;
        }
        else {
            allowed_extensions.push_back(arg);
        }
    }

    RequestChannel* channel;
    if (ipc == FIFO) {
        // TODO: Create the RequestChannel using FIFOChannel
    } else {
        // TODO: Create the RequestChannel using PipeChannel
    }

    if (system("mkdir -p storage") != 0) {
        cout << "Error creating storage directory" << endl;
        return 1;
    }

    while (true) {
        Request req = channel->receive_request();

        if (req.type == QUIT) {
            Response resp(true, 0, "", "Server shutting down");
            channel->send_response(resp);
            delete channel;
            exit(0);
        }

        Response resp;
        resp.success = true;
        
        if (req.type == UPLOAD_FILE) {
            // Check file extension if extensions were provided
            if (!allowed_extensions.empty()) {
                string ext = req.filename;
                size_t dot_pos = ext.find_last_of(".");
                if (dot_pos == string::npos) {
                    resp = Response(false, 0, "", "File has no extension");
                    channel->send_response(resp);
                    continue;
                }

                ext = ext.substr(dot_pos);
                bool allowed = false;
                for (const string& allowed_ext : allowed_extensions) {
                    if (ext == allowed_ext) {
                        allowed = true;
                        break;
                    }
                }
                
                if (!allowed) {
                    resp = Response(false, 0, "", "File extension not allowed");
                    channel->send_response(resp);
                    continue;
                }
            }
            
            string filepath = "storage/" + string(req.filename);
            ofstream outfile(filepath);
            
            if (!outfile) {
                resp = Response(false, 0, "", "Failed to create file");
            } else {
                outfile << req.data;
                outfile.close();
                resp = Response(true, 0, "", "File uploaded successfully");
            }
        }
        else if (req.type == DOWNLOAD_FILE) {
            string filepath = "storage/" + string(req.filename);
            ifstream infile(filepath);
            
            if (!infile) {
                resp = Response(false, 0, "", "File not found");
            } else {
                stringstream buffer;
                buffer << infile.rdbuf();
                resp = Response(true, 0, buffer.str(), "File downloaded successfully");
                infile.close();
            }
        }
        else {
            resp = Response(false, 0, "", "Unknown RequestType");
        }

        channel->send_response(resp);
    }
    
    return 0;
}