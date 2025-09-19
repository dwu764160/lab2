#!/bin/bash

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m'

# Test directory setup
TEST_DIR="/tmp/lab2_blackbox_test"
# TEST_DIR="/~/CSCE313/lab-2-solution/pub_tests"
POINTS=0
MAX_POINTS=10

print_test_header() {
    echo -e "\n${YELLOW}=== $1 ===${NC}"
}

setup() {
    rm -rf $TEST_DIR
    mkdir -p $TEST_DIR
    cp -r ./* $TEST_DIR/
    cd $TEST_DIR
    make clean && make all
}

cleanup() {
    cd - >/dev/null
    rm -rf $TEST_DIR
    make clean
}

award_points() {
    local test_name=$1
    local points=$2
    local max_points=$3
    local details=$4
    
    POINTS=$((POINTS + points))
    if [ $points -eq $max_points ]; then
        echo -e "${GREEN}✓ $test_name test passed: $points/$max_points points${NC}"
        [ -n "$details" ] && echo -e "  Details: $details"
    else
        echo -e "${RED}✗ $test_name test failed: $points/$max_points points${NC}"
        [ -n "$details" ] && echo -e "  Error: $details"
    fi
}

# Test 1: Basic Functionality (3 points)
test_basic_functionality() {
    print_test_header "Basic Functionality Tests"
    local points=0
    
    # Test login/logout (1 points)
    echo -e "1\n1\ntest.log\n1\n.txt\n1\n1\n7\n0\n" | ./client >/dev/null 2>&1
    
    if [ -f "test.log" ] && grep -q "logged in" "test.log" && grep -q "logged out" "test.log"; then
        award_points "Login/Logout" 1 1 "Successfully logged in and out"
    else
        award_points "Login/Logout" 0 1 "Failed to log in or out properly"
    fi
    
    # Test deposit/withdraw/balance (1 points)
    echo -e "1\n1\ntest2.log\n1\n.txt\n1\n1\n2\n100\n3\n50\n4\n7\n0\n" | ./client >/dev/null 2>&1
    
    if [ -f "test2.log" ] && \
       grep -q "deposited 100" "test2.log" && \
       grep -q "withdrew 50" "test2.log" && \
       grep -q "viewed balance" "test2.log"; then
        award_points "Banking Operations" 1 1 "Successfully performed banking operations"
    else
        award_points "Banking Operations" 0 1 "Failed to perform banking operations"
    fi
    
    # Test file operations (1 points)
    echo "test upload content" > test_upload.txt
    echo "test download content" > storage/test_download.txt
    echo -e "1\n1\ntest3.log\n1\n.txt\n1\n1\n5\ntest_upload.txt\n6\ntest_download.txt\n7\n0\n" | ./client >/dev/null 2>&1
    
    if [ -f "test3.log" ] && \
       grep -q "uploaded file" "test3.log" && \
       grep -q "downloaded file" "test3.log" && \
       [ -f "test_download.txt" ] && \
       [ -f "storage/test_upload.txt" ]; then
        award_points "File Operations" 1 1 "Successfully performed file operations"
    else
        award_points "File Operations" 0 1 "Failed to perform file operations"
    fi
}

# Test 2: Error Handling (3 points)
test_error_handling() {
    print_test_header "Error Handling Tests"
    
    # Test invalid account access (1 points)
    echo -e "1\n1\ntest4.log\n1\n.txt\n1\n9999\n2\n100\n7\n0\n" | ./client >/dev/null 2>&1
    if ! grep -q "deposited 100" "test4.log"; then
        award_points "Invalid Account" 1 1 "Properly handled invalid account"
    else
        award_points "Invalid Account" 0 1 "Failed to handle invalid account"
    fi
    
    # Test insufficient funds (1points)
    echo -e "1\n1\ntest5.log\n1\n.txt\n1\n1\n3\n1000000\n7\n0\n" | ./client >/dev/null 2>&1
    if ! grep -q "withdrew 1000000" "test5.log"; then
        award_points "Insufficient Funds" 1 1 "Properly handled insufficient funds"
    else
        award_points "Insufficient Funds" 0 1 "Failed to handle insufficient funds"
    fi
    
    # Test invalid file operations (1 points)
    echo -e "1\n1\ntest6.log\n1\n.txt\n1\n1\n6\nnonexistent.txt\n7\n0\n" | ./client >/dev/null 2>&1
    if ! grep -q "downloaded file: nonexistent.txt" "test6.log"; then
        award_points "Invalid File" 1 1 "Properly handled nonexistent file"
    else
        award_points "Invalid File" 0 1 "Failed to handle nonexistent file"
    fi
}

# Test 3: Concurrency (2 points)
test_concurrency() {
    print_test_header "Concurrency Tests"
    
    # Test multiple clients (1 points)
    local success=true
    
    # Start 3 simultaneous clients
    (echo -e "1\n1\ntest8_1.log\n1\n.txt\n1\n1\n2\n100\n7\n0\n" | ./client >/dev/null 2>&1) &
    (echo -e "1\n2\ntest8_2.log\n1\n.txt\n1\n2\n2\n200\n7\n0\n" | ./client >/dev/null 2>&1) &
    (echo -e "1\n3\ntest8_3.log\n1\n.txt\n1\n3\n2\n300\n7\n0\n" | ./client >/dev/null 2>&1) &
    
    wait
    
    # Verify each client's operations were recorded correctly
    if [ -f "test8_1.log" ] && [ -f "test8_2.log" ] && [ -f "test8_3.log" ] && \
       grep -q "deposited 100" "test8_1.log" && \
       grep -q "deposited 200" "test8_2.log" && \
       grep -q "deposited 300" "test8_3.log"; then
        award_points "Multiple Clients" 1 1 "Successfully handled multiple clients"
    else
        award_points "Multiple Clients" 0 1 "Failed to handle multiple clients"
    fi
    
    # Test concurrent file operations (1 points)
    echo "test content 1" > test_upload1.txt
    echo "test content 2" > test_upload2.txt
    
    (echo -e "1\n1\ntest9_1.log\n1\n.txt\n1\n1\n5\ntest_upload1.txt\n7\n0\n" | ./client >/dev/null 2>&1) &
    (echo -e "1\n2\ntest9_2.log\n1\n.txt\n1\n2\n5\ntest_upload2.txt\n7\n0\n" | ./client >/dev/null 2>&1) &
    
    wait
    
    if [ -f "storage/test_upload1.txt" ] && [ -f "storage/test_upload2.txt" ] && \
       [ "$(cat storage/test_upload1.txt)" = "test content 1" ] && \
       [ "$(cat storage/test_upload2.txt)" = "test content 2" ]; then
        award_points "Concurrent File Operations" 1 1 "Successfully handled concurrent file operations"
    else
        award_points "Concurrent File Operations" 0 1 "Failed to handle concurrent file operations"
    fi
}

# Test 4: System Stability (2 points)
test_system_stability() {
    print_test_header "System Stability Tests"
    
    # Test proper shutdown (1 points)
    echo -e "1\n1\ntest10.log\n1\n.txt\n1\n1\n7\n0\n" | ./client >/dev/null 2>&1
    
    # Check for zombie processes
    local zombie_count=$(ps aux | grep -E "\./(finance|logging|fileserver)" | grep -v grep | wc -l)
    if [ "$zombie_count" -eq 0 ]; then
        award_points "Clean Shutdown" 1 1 "All processes terminated properly"
    else
        award_points "Clean Shutdown" 0 1 "Found zombie processes"
    fi
    
    # Test resource cleanup (1 points)
    local fd_count_before=$(ls -l /proc/$$/fd | wc -l)
    echo -e "1\n1\ntest11.log\n1\n.txt\n1\n1\n7\n0\n" | ./client >/dev/null 2>&1
    local fd_count_after=$(ls -l /proc/$$/fd | wc -l)
    
    # Check file descriptors
    if [ "$fd_count_before" -eq "$fd_count_after" ] && \
       ! ls -la /tmp/fifo_* >/dev/null 2>&1; then
        award_points "Resource Cleanup" 1 1 "All resources cleaned up properly"
    else
        award_points "Resource Cleanup" 0 1 "Resources not cleaned up properly"
    fi
}

main() {
    echo -e "${YELLOW}Starting Black Box Tests...${NC}"
    setup
    
    test_basic_functionality
    test_error_handling
    test_concurrency
    test_system_stability
    
    echo -e "\n${YELLOW}========== Final Score ==========${NC}"
    echo -e "${GREEN}Total Score: $POINTS/$MAX_POINTS${NC}"
    
    cleanup
}

main