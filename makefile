CC      = gcc
CFLAGS  = -Wall -Wextra -std=c99
TARGET  = scheduler
SRC     = scheduler.c
OBJ     = $(SRC:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f $(OBJ) $(TARGET)