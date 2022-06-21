#include <pthread.h>

struct file{
    char* fn;
    pthread_mutex_t transfer_lock;
    struct file* next;
};

struct filesys{
    struct file** buckets;
    int n_buckets;
};

void init_fs(struct filesys* fs, int n_buckets);
struct file* add_file(struct filesys* fs, char* fn);
struct file* lookup_file(struct filesys* fs, char* fn, _Bool exact);
void free_fs(struct filesys* fs);
