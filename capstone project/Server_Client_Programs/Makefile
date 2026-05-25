CC = gcc

TARGET_SERVER = ems_server
TARGET_CLIENT = ems_client

BUILD_DIR = build
CFLAGS = -Wall -Wextra -pthread -g

SERVER_SRC = ems_server.c uart.c uart_processor.c network_socket.c crc16.c dashboard.c
CLIENT_SRC = ems_client.c network_socket.c

SERVER_OBJ = $(patsubst %.c,$(BUILD_DIR)/%.o,$(SERVER_SRC))
CLIENT_OBJ = $(patsubst %.c,$(BUILD_DIR)/%.o,$(CLIENT_SRC))

all: directories $(TARGET_SERVER) $(TARGET_CLIENT)

directories:
	@mkdir -p $(BUILD_DIR)
	@mkdir -p logs

$(TARGET_SERVER): $(SERVER_OBJ)
	$(CC) $(CFLAGS) $^ -o $@

$(TARGET_CLIENT): $(CLIENT_OBJ)
	$(CC) $(CFLAGS) $^ -o $@

$(BUILD_DIR)/%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILD_DIR) $(TARGET_SERVER) $(TARGET_CLIENT)
