CC = g++
CFLAGS = -c -o
OBJS = main.o time.o config.o log.o msg.o data.o cnt.o jang.o socket.o 
TARGET = tcpip_commuication_single_process.out

$(TARGET): $(OBJS) 
	$(CC) -o $@ $^

main.o: main.h main.cpp
time.o: C_time.cpp
	$(CC) $(CFLAGS) $@ $^

config.o: C_config.cpp
	$(CC) $(CFLAGS) $@ $^

log.o: C_log.cpp
	$(CC) $(CFLAGS) $@ $^

msg.o: C_msg.cpp
	$(CC) $(CFLAGS) $@ $^

data.o: C_data.h
	$(CC) $(CFLAGS) $@ $^

cnt.o: C_cnt.cpp
	$(CC) $(CFLAGS) $@ $^

jang.o: C_jang.cpp
	$(CC) $(CFLAGS) $@ $^

socket.o: C_socket.cpp
	$(CC) $(CFLAGS) $@ $^

clean:
	rm -f *.o
	rm -f $(TARGET)
