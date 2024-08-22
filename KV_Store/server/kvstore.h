



#ifndef __KV_STORE__
#define __KV_STORE__

#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>

#define KVSTORE_MAX_TOKENS 128
#define BUFFER_LENGTH		512

typedef int (*RCALLBACK)(int fd);

// conn, fd, buffer, callback
struct conn_item {
	int fd;
	
	char rbuffer[BUFFER_LENGTH];
	int rlen;
	char wbuffer[BUFFER_LENGTH];
	int wlen;

	union {
		RCALLBACK accept_callback;
		RCALLBACK recv_callback;
	} recv_t;
	RCALLBACK send_callback;
};


int kvstore_request(struct conn_item *item);
int epoll_entry(void);

void *kvstore_malloc(size_t size);
void kvstore_free(void *ptr);


#define NETWORK_EPOLL 0
#define NETWORK_NTYCO 1

#define ENABLE_NETWORK_SELECT NETWORK_EPOLL

#define ENABLE_ARRAY_KVENGINE 1
#define ENABLE_RBTREE_KVENGINE 1



#if ENABLE_ARRAY_KVENGINE

struct kvs_array_item
{
    char *key;
    char *value;
};

typedef struct array_s {
    struct kvs_array_item *array_table;
    int array_idx;
} array_t;

extern array_t Array;

int kvs_array_create(array_t *arr);
int kvs_array_destory(array_t *arr); 

int kvs_array_set(array_t* arr,char *key,char *value);
char *kvs_array_get(array_t* arr,char *key);
int kvs_array_delete(array_t *arr,char *key);
int kvs_array_modify(array_t *arr,char *key,char *value);
int kvs_array_count(array_t *arr);

#endif

#define KVS_ARRAY_SIZE 1024

#if ENABLE_RBTREE_KVENGINE
typedef struct _rbtree rbtree_t;

extern rbtree_t Tree;

int kvs_rbtree_create(rbtree_t *tree);
void kvse_rbtree_destory(rbtree_t *tree);
int kvs_rbtree_set(rbtree_t *tree,char *key, char *value);
char* kvs_rbtree_get(rbtree_t *tree,char *key);
int kvs_rbtree_delete(rbtree_t *tree,char *key);
int kvs_rbtree_modify(rbtree_t *tree,char *key,char *value);
int kvs_rbtree_count(rbtree_t *tree);
#endif


#endif

