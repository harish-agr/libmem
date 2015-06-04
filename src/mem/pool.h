#ifndef __MEM_POOL_H
#define __MEM_POOL_H

#include <stdint.h>
#include "allocator.h"

#if defined(__cplusplus)
extern "C" {
#endif

/* A fixed-size pool of fixed-size, fixed-address objects */
typedef struct pool_t
{
	/* A pointer to the beginning of the buffer */
	int8_t * buffer;

	/* A pointer to the next free element in the buffer */
	int8_t * next;

	/* The total size of the buffer in bytes */
	size_t size;

	/* A pointer to the allocator that was used to allocate the above buffer */
	allocator_t * allocator;

} pool_t;

/* Allocates a pool_t structure and initialises it with capacity for the given
 * number of element of the given size, using the given allocator. The returned
 * pool should be passed to pool_delete once it is no longer needed */
pool_t * pool_new( size_t element_size, size_t num_elements, allocator_t * allocator );

/* Releases the given pool_t structure and the resources it consumes. This function
 * should be used to release a pool returned by pool_new */
void pool_delete( pool_t * pool );

/* Initialises the given pool_t object with the given number of elements of the
 * given size, using the given allocator to allocate the underlying storage. The
 * pool should be passed to pool_cleanup once it is no longer required.
 *
 * Note that this function assumes the given pool is uninitialised, and does not
 * attempt to clean up automatically, as this would prevent people from passing
 * an uninitialised (potentially garbage) pool to this function in order to initialise
 * it. Memory leakage will therefore occur if this function is passed an already
 * initialised pool (in short, there is no current method by which this this function
 * can distinguish between a legitimately initialised pool and a garbage pool).
 */
void pool_init( pool_t * pool, size_t element_size, size_t num_elements, allocator_t * allocator );

/* Releases the underlying memory used by the given pool_t object. Use this function
 * to release the underlying resources for a pool initialised by pool_init */
void pool_cleanup( pool_t * pool );

/* Take an unused element from the pool and returns a pointer to it */
void * pool_take( pool_t * pool );

/* Return an element to the pool */
void pool_return( pool_t * pool, void * address );

/* Return 1 if there are no more free elements in the pool to return, or 0 otherwise */
int pool_is_empty( pool_t * pool );

#if defined(__cplusplus)
} /* extern "C" */
#endif

#endif /* __MEM_POOL_H */
