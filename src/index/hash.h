#ifndef _H_HASH
#define _H_HASH

/* == Inlcudes == */
#include "../constant.h"

/* == Structures/Enum == */
#define hash_t size_t

// For djb2
#define HASH_START 5381

/* == Prototypes == */
hash_t hash(hash_t h_val, char c);
hash_t sdbm(hash_t h_val, char c);

#endif
