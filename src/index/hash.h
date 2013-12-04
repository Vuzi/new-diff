#ifndef _H_HASH
#define _H_HASH

// For djb2
#define HASH_START 5381

/* == Inlcudes == */
#include "../constant.h"

/* == Structures/Enum == */
#define hash_t size_t

/* == Prototypes == */
hash_t hash(hash_t h_val, char c);
hash_t sdbm(hash_t h_val, char c);

#endif
