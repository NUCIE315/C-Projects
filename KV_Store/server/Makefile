#gcc -o kvstore kvstore.c epoll_entry.c kvstore.h array_store.c -I .

CC = gcc
FLAGS =  -I . 
SRCS = kvstore.c epoll_entry.c kvstore.h array_store.c kvstore_rbtree.c
TARGET = kvstore

kvstore:
	$(CC) -o $(TARGET) $(SRCS) $(FLAGS)
clear:
	rm -rf $(TARGET)


