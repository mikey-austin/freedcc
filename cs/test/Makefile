CC			= gcc
CFLAGS		= -g -Wall -Wstrict-prototypes

TARGET		= hash_test_1
SRC		= hash.c hash_test_1.c
OBJ		= $(SRC:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJ)

hash.o: hash.c hash.h
hash_test_1.o: hash_test_1.c hash.h
