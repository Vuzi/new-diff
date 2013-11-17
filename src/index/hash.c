#include "hash.h"

// djb2
hash_t hash(hash_t h_val, char c) {
    return (((h_val << 5) + h_val) + c); /* hash * 33 + c */
}

// sdbm
hash_t sdbm(hash_t h_val, char c) {

    return c + (h_val << 6) + (h_val << 16) - h_val;
}
