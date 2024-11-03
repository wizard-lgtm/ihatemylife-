TARGET = ihatemylife
CC = gcc
ARGS = -Wall -Wextra -g -I./include -I/usr/local/include
SRC = main.c db.c types.c http.c
LDFLAGS = -ldotenv

# Include the pkg-config command to get the flags for the MongoDB C driver
MONGOC_CFLAGS = $(shell pkg-config --cflags libmongoc-1.0)
MONGOC_LIBS = $(shell pkg-config --libs libmongoc-1.0)

# Generate object file names from source files
OBJ = $(SRC:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) -o $(TARGET) $(OBJ) $(MONGOC_LIBS) $(LDFLAGS)

%.o: %.c
	$(CC) $(ARGS) $(MONGOC_CFLAGS) -c $< -o $@

clean:
	rm -f $(TARGET) $(OBJ)

run: 
	./$(TARGET)
