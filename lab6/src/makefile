# Compiler
CC = gcc

# Compiler flags
CFLAGS = -lpthread

# Targets
TARGETS = client server socket_utils.o

# Source files
CLIENT_SRC = client.c
SERVER_SRC = server.c
UTILS_SRC = socket_utils.c

# Executable names
CLIENT_EXEC = client
SERVER_EXEC = server

# Default target
all: $(TARGETS)

# Build client executable
$(CLIENT_EXEC): $(CLIENT_SRC) $(UTILS_SRC)
	$(CC) $(CLIENT_SRC) $(UTILS_SRC) -o $(CLIENT_EXEC) $(CFLAGS)

# Build server executable
$(SERVER_EXEC): $(SERVER_SRC) $(UTILS_SRC)
	$(CC) $(SERVER_SRC) $(UTILS_SRC) -o $(SERVER_EXEC) $(CFLAGS)

# Clean up generated files
clean:
	rm -f $(CLIENT_EXEC) $(SERVER_EXEC)

.PHONY: all clean
