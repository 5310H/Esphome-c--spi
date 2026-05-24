CC = gcc
CFLAGS = -Wall -Wextra -I./src/proto -I./include
LDFLAGS = -lsodium

SRC = \
    src/main.c \
    src/proto/api.pb.c \
    src/proto/api_encryption.pb.c \
    src/proto/api_service.pb.c \
    src/proto/api_options.pb.c \
    src/proto/pb_common.c \
    src/proto/pb_encode.c \
    src/proto/pb_decode.c
    
OBJ = $(SRC:.c=.o)

TARGET = esphome_client

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(OBJ) -o $(TARGET) $(LDFLAGS)

clean:
	rm -f $(OBJ) $(TARGET)