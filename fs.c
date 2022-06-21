#include <string.h>
#include <stdlib.h>

#include "fs.h"

void init_fs(struct filesys* fs, int n_buckets){
    fs->n_buckets = n_buckets;
    fs->buckets = calloc(sizeof(struct file*), fs->n_buckets);
}

struct file* add_file(struct filesys* fs, char* fn){
    struct file* f = malloc(sizeof(struct file));
    int idx;
    for(char* i = fn; *i; ++i)
        idx += (i-fn+1)*(*i);
    idx %= fs->n_buckets;
    f->fn = fn;
    f->next = fs->buckets[idx];
    fs->buckets[idx] = f;
    return f; 
}

struct file* _iter_fs(struct filesys* fs, char* fn, size_t fnlen, _Bool exact){
    struct file* prev;
    for(int i = 0; i < fs->n_buckets; ++i){
        prev = NULL;
        for(struct file* f = fs->buckets[i]; f; f = f->next){
            if(!fn){
                if(prev)free(prev);
                prev = f;
                continue;
            }
            if(exact && strlen(f->fn) != fnlen)continue;
            if(!memcmp(fn, f->fn, fnlen))return f;
            if(!exact && strstr(f->fn, fn))return f;
        }
        if(prev)free(prev);
    }
    if(!fn)free(fs->buckets);
    return NULL;
}

struct file* lookup_file(struct filesys* fs, char* fn, _Bool exact){
    size_t fnlen = strlen(fn);
    return _iter_fs(fs, fn, fnlen, exact);
}

void free_fs(struct filesys* fs){
    _iter_fs(fs, NULL, -1, 0);
}
