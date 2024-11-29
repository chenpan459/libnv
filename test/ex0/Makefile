CC = gcc
CFLAGS = -Wall -Wextra -I../../src
LDFLAGS = -L../../lib -lnv -pthread

TARGET = test_log

all: $(TARGET)

$(TARGET): benchmark_nv.c
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

clean:
	rm -f $(TARGET)

.PHONY: all clean
