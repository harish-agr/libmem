#ifndef __MEM_ALLOCATOR_H
#define __MEM_ALLOCATOR_H

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#if defined(__cplusplus)
extern "C" {
#endif

/**
 * allocator_t
 *
 * A swappable allocator abstraction.
 *
 */
typedef struct allocator_t
{
	/* Allocation function takes number of bytes to allocate plus a pointer to the allocator */
	void * ( *alloc_fn )( size_t, struct allocator_t * );

	/* Free function takes address to free plus a pointer to the allocator */
	void ( *free_fn )( void *, struct allocator_t * );

} allocator_t;


/**
 * allocator_alloc
 *
 * Use the given allocator to allocate the given number of bytes.
 *
 */
void * allocator_alloc( size_t length, allocator_t * allocator );


/**
 * allocator_free
 *
 * Use the given allocator to free the given memory address.
 *
 */
void allocator_free( void * address, allocator_t * allocator );


/**
 * allocator_default
 *
 * Returns a pointer to the default allocator, which calls through to stdlib malloc / free
 *
 */
allocator_t * allocator_default( void );


/**
 * allocator_always_fail
 *
 * Returns a pointer to an allocator that always returns NULL - useful for testing
 *
 */
allocator_t * allocator_always_fail( void );


/**
 * allocator_aligned_t
 *
 * An allocator whose allocations are aligned to a fixed byte boundary.
 *
 */
typedef struct allocator_aligned_t
{
	/* The allocation functions for this allocator. */
	allocator_t alloc;

	/* Parent allocator */
	allocator_t * parent;

	/* Number of bytes to align */
	size_t alignment;

} allocator_aligned_t;


/**
 * allocator_aligned_init
 *
 * Initialises the given aligned memory allocator.
 *
 */
void allocator_aligned_init(
	allocator_aligned_t * allocator,
	allocator_t * parent,
	size_t alignment
);


/**
 * allocator_aligned_init_default
 *
 * Initialises the given aligned memory allocator, using the default allocator for
 * underlying memory allocations.
 *
 */
void allocator_aligned_init_default(
	allocator_aligned_t * allocator,
	size_t alignment
);


/**
 * allocator_aligned_get
 *
 * Returns the given aligned allocator as an allocator_t pointer.
 *
 */
allocator_t * allocator_aligned_get(
	allocator_aligned_t * allocator
);


/**
 * allocator_guarded_t
 *
 * Pads the beginning and end of each allocation with an additional guard value, which
 * is set to a known value on allocation and validated when free'd. Blocks of memory with
 * invalidated guard blocks are not free'd by design, such that these blocks are made
 * visible to memory checkers (valgrind, etc).
 *
 */
typedef struct allocator_guarded_t
{
	/* The allocation functions for this allocator. */
	allocator_t alloc;

	/* The parent allocator to which all memory allocations will be forwarded. */
	allocator_t * parent;

} allocator_guarded_t;


/**
 * allocator_guarded_init
 *
 * Initialises the given guarded allocator.
 *
 */
void allocator_guarded_init(
	allocator_guarded_t * allocator,
	allocator_t * parent
);


/**
 * allocator_guarded_init_default
 *
 * Initialises the given guarded allocator, using the default allocator for underlying
 * memory allocations.
 *
 */
void allocator_guarded_init_default(
	allocator_guarded_t * allocator
);


/**
 * allocator_guarded_get
 *
 * Returns the given guarded allocator as an allocator_t pointer.
 *
 */
allocator_t * allocator_guarded_get(
	allocator_guarded_t * allocator
);


/**
 * allocator_guarded_length
 *
 * Returns the length in bytes of a guarded block of memory (as requested by the
 * user, excluding additional memory required to manage the block of memory).
 * Returns 0 if the given memory block is not valid (e.g. has been released,
 * was not allocated with a guarded allocator, or has invalidated guard blocks)
 *
 */
size_t allocator_guarded_length( void * address );


/**
 * allocator_traced_t
 *
 * Prints all allocations to a given FILE descriptor - useful for debugging.
 *
 */
typedef struct allocator_traced_t
{
	/* The allocation functions for this allocator */
	allocator_t alloc;

	/* The parent allocator to which all allocations will be forwarded */
	allocator_t * parent;

	/* The FILE descriptor to which the trace messages will be written to */
	FILE * fd;

} allocator_traced_t;


/**
 * allocator_traced_init
 *
 * Initialises the given traced allocator
 *
 */
void allocator_traced_init(
	allocator_traced_t * allocator,
	allocator_t * parent,
	FILE * fd
);


/**
 * allocator_traced_init_stdout
 *
 * Initialises the given traced allocator, with trace messages being sent to stdout
 *
 */
void allocator_traced_init_stdout(
	allocator_traced_t * allocator,
	allocator_t * parent
);


/**
 * allocator_traced_init_stderr
 *
 * Initialises the given traced allocator, with trace messages being sent to stderr
 *
 */
void allocator_traced_init_stderr(
	allocator_traced_t * allocator,
	allocator_t * parent
);


/**
 * allocator_traced_get
 *
 * Returns the given traced allocator as an allocator_t pointer.
 *
 */
allocator_t * allocator_traced_get(
	allocator_traced_t * allocator
);


/**
 * allocator_counted_t
 *
 * Maintains a count of the number of bytes currently consumed by the allocator,
 * and a peak number of bytes ever to be consumed by the allocator.
 *
 */
typedef struct allocator_counted_t
{
	/* The allocation functions for this allocator */
	allocator_t alloc;

	/* The parent allocator to which all allocations will be forwarded */
	allocator_t * parent;

	/* Keeps track of the current number of bytes consumed by the allocator */
	size_t current;

	/* Keeps track of the maximum number of bytes ever to be consumed by this
	 * allocator */
	size_t peak;

} allocator_counted_t;


/**
 * allocator_counted_init
 *
 * Initialises the given counted allocator.
 *
 */
void allocator_counted_init(
	allocator_counted_t * allocator,
	allocator_t * parent
);


/**
 * allocator_counted_init_default
 *
 * Initialises the given counted allocator, using the default allocator for underlying
 * memory allocations.
 *
 */
void allocator_counted_init_default(
	allocator_counted_t * allocator
);


/**
 * allocator_counted_get
 *
 * Returns the given counted allocator as an allocator_t pointer.
 *
 */
allocator_t * allocator_counted_get(
	allocator_counted_t * allocator
);


/**
 * allocator_counted_get_current_count
 *
 * Returns the number of bytes currently consumed by this allocator.
 *
 */
size_t allocator_counted_get_current_count(
	allocator_counted_t * allocator
);


/**
 * allocator_counted_get_peak_count
 *
 * Returns the maximum number of bytes ever to be consumed by this allocator.
 *
 */
size_t allocator_counted_get_peak_count(
	allocator_counted_t * allocator
);

#if defined(__cplusplus)
} /* extern "C" */
#endif

#endif /* __MEM_ALLOCATOR_H */
