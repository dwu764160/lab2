# Compiler and flags
CXX = g++
CXXFLAGS = -std=c++17 -g -Wall -Wextra

# File names
CHANNEL_SRC = channel.cpp
CLIENT_SRC = client.cpp
FINANCE_SRC = finance.cpp
FILE_SRC = file.cpp
LOGGING_SRC = logging.cpp

# Generate object file names
CHANNEL_OBJ = $(CHANNEL_SRC:.cpp=.o)
CLIENT_OBJ = $(CLIENT_SRC:.cpp=.o)
FINANCE_OBJ = $(FINANCE_SRC:.cpp=.o)
FILE_OBJ = $(FILE_SRC:.cpp=.o)
LOGGING_OBJ = $(LOGGING_SRC:.cpp=.o)

EXECUTABLES = client finance fileserver logging

# Default target
all: $(EXECUTABLES)

# Generic rule for object files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Build executables using object files
client: $(CLIENT_OBJ) $(CHANNEL_OBJ)
	$(CXX) $(CXXFLAGS) $(CLIENT_OBJ) $(CHANNEL_OBJ) -o $@

finance: $(FINANCE_OBJ) $(CHANNEL_OBJ)
	$(CXX) $(CXXFLAGS) $(FINANCE_OBJ) $(CHANNEL_OBJ) -o $@

fileserver: $(FILE_OBJ) $(CHANNEL_OBJ)
	$(CXX) $(CXXFLAGS) $(FILE_OBJ) $(CHANNEL_OBJ) -o $@

logging: $(LOGGING_OBJ) $(CHANNEL_OBJ)
	$(CXX) $(CXXFLAGS) $(LOGGING_OBJ) $(CHANNEL_OBJ) -o $@

# Clean up
clean:
	rm -f *.o $(EXECUTABLES)

distclean: clean
	rm -f *.log fifo_*
	rm -rf storage

# Phony targets
.PHONY: all clean distclean test private_test

# Dependencies
$(CLIENT_OBJ): channel.h
$(FINANCE_OBJ): channel.h
$(FILE_OBJ): channel.h
$(LOGGING_OBJ): channel.h

test: all
	chmod u+x lab2-tests.sh
	./lab2-tests.sh
