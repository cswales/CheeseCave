CC = gcc
#CFLAGS =  -std=c99 -I. -lbcm2835
CFLAGS =  -std=gnu99 -I. -lbcm2835
DEPS = 

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)


%: %.c 
	$(CC) -o $@ $^ $(CFLAGS)

all: relay temp_log_DHT
