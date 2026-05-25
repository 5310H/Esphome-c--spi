CC = gcc
TIMESTAMP := $(shell TZ="America/New_York" date +"%Y-%m-%d %H:%M EST")
MD_FILES := README.md CONTRIBUTING.md ROADMAP.md

CFLAGS = -Wall -Wextra -O2 -Isrc/proto -Isrc
LIBS = -lsodium

# NanoPB and API Source Files
PROTO_DIR = src/proto
SRC_DIR = src

PROTO_SRCS = $(PROTO_DIR)/api_encryption.pb.c \
             $(PROTO_DIR)/api_message.pb.c \
             $(PROTO_DIR)/api_service.pb.c \
             $(PROTO_DIR)/api_options.pb.c \
             $(PROTO_DIR)/pb_common.c \
             $(PROTO_DIR)/pb_encode.c \
             $(PROTO_DIR)/pb_decode.c
PROTO_OBJS = $(PROTO_SRCS:.c=.o)
CLIENT_MAIN_OBJ = $(SRC_DIR)/main.o
FAKE_MAIN_OBJ = $(SRC_DIR)/fake_device.o

TARGET = esphome_client
FAKE_DEVICE = fake_esphome_device

CLIENT_OBJS = $(CLIENT_MAIN_OBJ) $(PROTO_OBJS)
FAKE_OBJS = $(FAKE_MAIN_OBJ) $(PROTO_OBJS)

all: $(TARGET) $(FAKE_DEVICE)

$(TARGET): $(CLIENT_OBJS)
	$(CC) $(CLIENT_OBJS) -o $(TARGET) $(LIBS)

$(FAKE_DEVICE): $(FAKE_OBJS)
	$(CC) $(FAKE_OBJS) -o $(FAKE_DEVICE) $(LIBS)

update_metadata:
	@echo "Updating timestamps to $(TIMESTAMP)..."
	@sed -i 's/\*Last Updated: .* .*\*/\*Last Updated: $(TIMESTAMP)\*/' $(MD_FILES)

%.o: %.c $(PROTO_HDRS)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f *.o src/*.o src/proto/*.o $(TARGET) $(FAKE_DEVICE)

.PHONY: all clean update_metadata