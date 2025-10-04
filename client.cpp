#include "common.h"
#include "channel.h"
#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include <fstream>
#include <vector>

using namespace std;

void print_menu() {
    cout << "\n=== Banking System Menu ===\n"
         << "1. Login\n"
         << "2. Deposit\n"
         << "3. Withdraw\n"
         << "4. View Balance\n"
         << "5. Upload File\n"
         << "6. Download File\n"
         << "7. Logout\n"
         << "0. Exit\n"
         << "Enter choice: ";
}

void clear_input() {
    cin.clear();
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
}

int main() {
    // Choose pipes or FIFO for IPC
    int ipc_choice = 0;
    while (ipc_choice != 1 && ipc_choice != 2) {
        cout << "Choose IPC type, 1 for PIPE, 2 for FIFO: ";
        cin >> ipc_choice;
    }
    IPCType ipc;
    switch(ipc_choice) {
        case 1:
            ipc = PIPE;
            break;
        case 2:
            ipc = FIFO;
            break;
        default:
            cout << "Defaulting to PIPE" << endl;
            ipc = PIPE;
    }

    // Start servers
    cout << "Starting servers..." << endl;

    // finance server
    int max_account;
    cout << "Enter the maximum account number: ";
    cin >> max_account;

    int finance_pipe1[2], finance_pipe2[2];
    int logging_pipe1[2], logging_pipe2[2]; 
    int file_pipe1[2], file_pipe2[2];
    // TODO: Create pipes before fork within the if statement
    if (ipc == PIPE) {
        if (pipe(finance_pipe1) < 0 || pipe(finance_pipe2) < 0 ||
            pipe(logging_pipe1) < 0 || pipe(logging_pipe2) < 0 ||
            pipe(file_pipe1) < 0 || pipe(file_pipe2) < 0) {
            perror("Pipe creation failed");
            exit(1);
        }
    }

    // Start finance server
    pid_t pid = fork();
    if (pid < 0) {
        perror("Fork failed");
        exit(1);
    }
    if (pid == 0) {  // Child process
        if (ipc == PIPE) {
            // TODO: close unused ends and dup2 the right ones
            // child: finance reads from finance_pipe1[0] (stdin), writes to finance_pipe2[1] (stdout)
            dup2(finance_pipe1[0], STDIN_FILENO);
            dup2(finance_pipe2[1], STDOUT_FILENO);
            // close all fds (both ends) in child after dup2
            close(finance_pipe1[0]); close(finance_pipe1[1]);
            close(finance_pipe2[0]); close(finance_pipe2[1]);
            // also close logging & file pipes since child doesn't use them
            close(logging_pipe1[0]); close(logging_pipe1[1]);
            close(logging_pipe2[0]); close(logging_pipe2[1]);
            close(file_pipe1[0]); close(file_pipe1[1]);
            close(file_pipe2[0]); close(file_pipe2[1]);
        }
        
        char* args[] = {(char*)"./finance", (char*)"-p", (char*)"-m", (char*)to_string(max_account).c_str(), nullptr};
        if (ipc == FIFO) {
            args[1] = (char*)"-f";
        }
        execvp(args[0], args);
        perror("Execvp failed");
        exit(1);
    } else{// parent
        if (ipc == PIPE) {
            close(finance_pipe1[0]); // parent will write to finance_pipe1[1]
            close(finance_pipe2[1]); // parent will read from finance_pipe2[0]
        }
    }

    // logging server
    string log_file_name;
    cout << "Enter the name of your logging file: ";
    cin >> log_file_name;

    // Start logging server
    pid = fork();
    if (pid < 0) {
        perror("Fork failed");
        exit(1);
    }
    if (pid == 0) { // Child process
        if (ipc == PIPE) {
            // TODO: close unused ends and dup2 the right ones
            // child: logging reads from logging_pipe1[0] (stdin), writes to logging_pipe2[1] (stdout)
            dup2(logging_pipe1[0], STDIN_FILENO);
            dup2(logging_pipe2[1], STDOUT_FILENO);
            // close all fds in child
            close(logging_pipe1[0]); close(logging_pipe1[1]);
            close(logging_pipe2[0]); close(logging_pipe2[1]);
            // close finance & file pipes
            close(finance_pipe1[0]); close(finance_pipe1[1]);
            close(finance_pipe2[0]); close(finance_pipe2[1]);
            close(file_pipe1[0]); close(file_pipe1[1]);
            close(file_pipe2[0]); close(file_pipe2[1]);
        }

        char* args[] = {(char*)"./logging", (char*)"-p", (char*)"-n", (char*)log_file_name.c_str(), nullptr};
        if (ipc == FIFO) {
            args[1] = (char*)"-f";
        }
        execvp(args[0], args);
        perror("Execvp failed");
        exit(1);
    } else { // parent
        if (ipc == PIPE) {
            close(logging_pipe1[0]); // parent will write to logging_pipe1[1]
            close(logging_pipe2[1]); // parent will read from logging_pipe2[0]
        }
    }

    // file server
    cout << "Enter number of allowed file extensions: ";
    int num_extensions;
    cin >> num_extensions;
    cin.ignore();

    // Store extensions in vector
    vector<string> extensions;
    cout << "Enter allowed extensions (including the dot, e.g. .txt):" << endl;
    string ext;
    for(int i = 0; i < num_extensions; i++) {
        cout << i+1 << ": ";
        getline(cin, ext);
        extensions.push_back(ext);
    }

    // Create argument array for file server
    char** file_args = new char*[num_extensions + 3]; // +3 for program name, ipc type, and NULL
    file_args[0] = (char*)"./fileserver";
    if (ipc == FIFO) {
        file_args[1] = (char*)"-f";
    } else {
        file_args[1] = (char*)"-p";
    }
    
    // Fill with pointers to the extension c strings
    for(int i = 0; i < num_extensions; i++) {
        file_args[i + 2] = (char*)extensions[i].c_str();
    }
    file_args[num_extensions + 2] = NULL;

    // Start file server
    pid_t file_pid = fork();
    if (file_pid < 0) {
        perror("Fork failed");
        exit(1);
    }
    if (file_pid == 0) {
        if (ipc == PIPE) {
            // child: file server reads from file_pipe1[0] (stdin), writes to file_pipe2[1] (stdout)
            dup2(file_pipe1[0], STDIN_FILENO);
            dup2(file_pipe2[1], STDOUT_FILENO);
            // close all fds in child
            close(file_pipe1[0]); close(file_pipe1[1]);
            close(file_pipe2[0]); close(file_pipe2[1]);
            // close finance & logging pipes
            close(finance_pipe1[0]); close(finance_pipe1[1]);
            close(finance_pipe2[0]); close(finance_pipe2[1]);
            close(logging_pipe1[0]); close(logging_pipe1[1]);
            close(logging_pipe2[0]); close(logging_pipe2[1]);
        }
        
        execvp(file_args[0], file_args);
        perror("File server exec failed");
        exit(1);
    } else { // parent
        if (ipc == PIPE) {
            close(file_pipe1[0]); // parent will write to file_pipe1[1]
            close(file_pipe2[1]); // parent will read from file_pipe2[0]
        }
    }

    delete[] file_args;
    
    // Give servers time to start
    usleep(500000);
    
    // Create the parent Request Channels
    RequestChannel* finance;
    RequestChannel* logging;
    RequestChannel* file;
    if (ipc == FIFO) {
        // TODO: Create channels in parent using FIFOChannel
        finance = new FIFOChannel("finance", RequestChannel::CLIENT_SIDE);
        logging = new FIFOChannel("logging", RequestChannel::CLIENT_SIDE);
        file = new FIFOChannel("files", RequestChannel::CLIENT_SIDE);
    } else {
        // TODO: Create channels in parent using PipeChannel
        // Parent reads from *_pipe2[0] and writes to *_pipe1[1]
        finance = new PipeChannel(finance_pipe2[0], finance_pipe1[1]);
        logging = new PipeChannel(logging_pipe2[0], logging_pipe1[1]);
        file = new PipeChannel(file_pipe2[0], file_pipe1[1]);
    }

    int current_user = -1;  // -1 means no user logged in
    bool running = true;
    
    while (running) {
        print_menu();
        
        int choice;
        while (!(cin >> choice)) {
            cout << "Invalid input. Please enter a number: ";
            clear_input();
        }
        clear_input();

        switch (choice) {
            case 0: {  // Exit
                running = false;
                break;
            }
            
            case 1: {  // Login
                if (current_user != -1) {
                    cout << "Already logged in! Please logout first.\n";
                    break;
                }
                
                cout << "Enter user ID: ";
                cin >> current_user;
                clear_input();
                
                Request login(LOGIN, current_user);
                logging->send_request(login);
                cout << "Logged in as user " << current_user << endl;
                break;
            }
            
            case 2: {  // Deposit
                if (current_user == -1) {
                    cout << "Please login first!\n";
                    break;
                }
                
                double amount;
                cout << "Enter amount to deposit: ";
                cin >> amount;
                clear_input();
                
                Request txn(DEPOSIT, current_user, amount);
                Response resp = finance->send_request(txn);
                
                if (resp.success) {
                    cout << "Deposit successful. New balance: " << resp.balance << endl;
                    Request audit(DEPOSIT, current_user, amount, "", "deposited " + to_string(amount));
                    logging->send_request(audit);
                } else {
                    cout << "Deposit failed: " << resp.message << endl;
                }
                break;
            }
            
            case 3: {  // Withdraw
                if (current_user == -1) {
                    cout << "Please login first!\n";
                    break;
                }
                
                double amount;
                cout << "Enter amount to withdraw: ";
                cin >> amount;
                clear_input();
                
                Request txn(WITHDRAW, current_user, amount);
                Response resp = finance->send_request(txn);
                
                if (resp.success) {
                    cout << "Withdrawal successful. New balance: " << resp.balance << endl;
                    Request audit(WITHDRAW, current_user, amount, "", "withdrew " + to_string(amount));
                    logging->send_request(audit);
                } else {
                    cout << "Withdrawal failed: " << resp.message << endl;
                }
                break;
            }
            
            case 4: {  // View Balance
                if (current_user == -1) {
                    cout << "Please login first!\n";
                    break;
                }
                
                Request txn(BALANCE, current_user);
                Response resp = finance->send_request(txn);
                
                if (resp.success) {
                    cout << "View balance successful. Your balance: " << resp.balance << endl;
                    Request audit(BALANCE, current_user, resp.balance, "", "viewed balance");
                    logging->send_request(audit);
                } else {
                    cout << "View balance failed: " << resp.message << endl;
                }
                break;
            }
            
            case 5: {  // Upload File
                if (current_user == -1) {
                    cout << "Please login first!\n";
                    break;
                }
                
                string filename;
                cout << "Enter filename to upload: ";
                getline(cin, filename);
                
                ifstream infile(filename);
                if (!infile) {
                    cout << "Error: Could not open file\n";
                    break;
                }
                
                string content((istreambuf_iterator<char>(infile)), {});
                infile.close();

                // NOTE: file uploads will truncate. If you want to upload a full file, use a file that has less content (less than 1024 chars)
                Request upload(UPLOAD_FILE, current_user, 0, filename, content);
                Response resp = file->send_request(upload);
                
                if (resp.success) {
                    cout << "File upload successful\n";
                    Request audit(UPLOAD_FILE, current_user, 0, filename, "uploaded file");
                    logging->send_request(audit);
                } else {
                    cout << "File upload failed: " << resp.message << endl;
                }
                break;
            }
            
            case 6: {  // Download File
                if (current_user == -1) {
                    cout << "Please login first!\n";
                    break;
                }
                
                string filename;
                cout << "Enter filename to download: ";
                getline(cin, filename);
                Request download(DOWNLOAD_FILE, current_user, 0, filename);
                Response resp = file->send_request(download);
                
                if (resp.success) {
                    ofstream outfile(filename);
                    outfile << resp.data;
                    outfile.close();
                    cout << "File downloaded successfully\n";
                    Request audit(DOWNLOAD_FILE, current_user, 0, filename, "downloaded file");
                    logging->send_request(audit);
                } else {
                    cout << "File download failed: " << resp.message << endl;
                }
                break;
            }
            
            case 7: {  // Logout
                if (current_user == -1) {
                    cout << "Not logged in!\n";
                    break;
                }
                Request logout(LOGOUT, current_user);
                logging->send_request(logout);
                current_user = -1;
                cout << "Logged out successfully\n";
                break;
            }
            
            default:
                cout << "Invalid choice. Please try again.\n";
        }
    }

    // Cleanup

    Request quit(QUIT);
    finance->send_request(quit);
    file->send_request(quit);
    logging->send_request(quit);

    // Wait for all child processes to finish
    while(wait(NULL) > 0);

    // TODO: Clean up channels
    delete finance;
    delete file;
    delete logging;
    
    return 0;
}