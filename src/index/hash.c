#include "hash.h"

/* ===============================================
                       hash

    Add c to the hash h_val. h_val is mulitplied by
    32 (5 bit to the right), then added to himself
    (same thing as multiplied by 33), then added
    to c.

    This function dosen't have the repartition of
    md5 or sha1, but its enough sure to be used
    in diff. Plus its really fast, and the result
    can be stored in an unsigned long.
    ----------------------------------------------
    hash h_val : previous hash
    char c     : char to add to the hash
    ----------------------------------------------
    Return the new hash value.
   =============================================== */
hash_t hash(hash_t h_val, char c) {
    return (((h_val << 5) + h_val) + c); // hash * 33 + c
}


/* ===============================================
                       sdbm

    sdbm hash function. Not used.
    ----------------------------------------------
    hash h_val : previous hash
    char c     : char to add to the hash
    ----------------------------------------------
    Return the new hash value.
   =============================================== */
hash_t sdbm(hash_t h_val, char c) {
    return c + (h_val << 6) + (h_val << 16) - h_val;
}
