TARGET = ihatemylife
CC = gcc
ARGS = -Wall -Wextra -g
SRC = main.c

all: $(TARGET)

$(TARGET): $(TARGET).o
	$(CC) $(LDARGS) -o $(TARGET) $(TARGET).o

$(TARGET).o: $(SRC)
	$(CC) $(ARGS) -c $(SRC) -o $(TARGET).o

clean:
	rm -f $(TARGET) $(TARGET).o
run: 
	./$(TARGET)