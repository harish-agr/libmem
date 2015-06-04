#ifndef __MEM_BUFFER_H
#define __MEM_BUFFER_H

#include "allocator.h"

#if defined(__cplusplus)
extern "C" {
#endif

/**
 * buffer_t
 *
 * A growable memory buffer.
 *
 */
typedef struct buffer_t
{
	/* The current capacity of this buffer (in bytes) */
	size_t capacity;

	/* A pointer to the beginning of the buffer */
	int8_t * begin;

	/* The current position within the buffer */
	int8_t * pos;

	/* The allocator to use when growing the buffer */
	allocator_t * allocator;

} buffer_t;


/**
 * buffer_new
 *
 * Allocate a new buffer_t object using the given allocator, and initialise the
 * buffer as an empty buffer. Should call buffer_delete to release the buffer_t
 * object, and its underlying memory buffer.
 */
buffer_t * buffer_new( allocator_t * allocator );


/**
 * buffer_delete
 *
 * Releases the given buffer_t object and its underlying memory buffer. Use this
 * function to release a buffer_t object created with buffer_new.
 *
 */
void buffer_delete( buffer_t * buffer );


/**
 * buffer_init
 *
 * Initialises the underlying memory buffer in the given buffer_t object as an
 * empty buffer (with zero capacity). Will use the given allocator for all
 * subsequent allocations. Should call buffer_cleanup to release the underlying memory.
 *
 */
void buffer_init( buffer_t * buffer, allocator_t * allocator );


/**
 * buffer_cleanup
 *
 * Releases the underlying memory buffer in the given buffer_t object (the buffer_t
 * object itself is not released). Use this function to clean up a buffer_t object that
 * was initialised with the buffer_init function.
 *
 */
void buffer_cleanup( buffer_t * buffer );


/**
 * buffer_capacity
 *
 * Returns the number of bytes allocated for this buffer.
 *
 */
size_t buffer_capacity( buffer_t * buffer );


/**
 * buffer_grow
 *
 * Grows the buffer's capacity by the given number of bytes. Returns
 * the new total capacity of the buffer.
 *
 * Note that calling this function after buffer_cleanup will have the
 * effect of reinitialising the buffer, and must therefore be cleaned
 * up again with buffer_cleanup once the buffer is no longer required.
 *
 */
size_t buffer_grow( buffer_t * buffer, size_t amount_in_bytes );


/**
 * buffer_data_length
 *
 * Returns the number of bytes that have been written to the buffer.
 *
 */
size_t buffer_data_length( buffer_t * buffer );


/**
 * buffer_data_pointer
 *
 * Returns a pointer to the underlying data buffer. This could return null
 * if the buffer has zero capacity.
 *
 */
void * buffer_data_pointer( buffer_t * buffer );


/**
 * buffer_rewind
 *
 * Rewinds the current buffer position to the beginning of the buffer.
 * Does not affect the capacity of the buffer.
 *
 */
void buffer_rewind( buffer_t * buffer );


/**
 * buffer_append
 *
 * Copies the given number of bytes to the end of the given buffer_t object.
 * Returns the number of bytes that were successfully appended. Grows the
 * buffer to accommodate the given data if necessary.
 *
 * Note that calling this function after buffer_cleanup will have the
 * effect of reinitialising the buffer, and must therefore be cleaned
 * up again with buffer_cleanup once the buffer is no longer required.
 *
 */
size_t buffer_append( buffer_t * buffer, size_t length, void * data );


/**
 * buffer_reserve
 *
 * Reserves the given number of bytes starting at the current write position
 * in the buffer. Grows the buffer to accommodate this reserved block of memory
 * if necessary. Returns a pointer to this reserved block of memory such that it
 * can be initialised at some later point. This is particularly useful as an
 * alternative to buffer_append, when you do not yet have any data to initialise
 * this block of memory with.
 *
 * Note that the returned block of memory will be invalidated if the buffer
 * is deleted, cleaned up, or grown.
 *
 * Note that calling this function after buffer_cleanup will have the
 * effect of reinitialising the buffer, and must therefore be cleaned
 * up again with buffer_cleanup once the buffer is no longer required.
 *
 */
void * buffer_reserve( buffer_t * buffer, size_t length );


/**
 * buffer_allocator_t
 *
 * An allocator_t adapter that can be used to allocate memory from a
 * buffer_t object
 *
 */
typedef struct buffer_allocator_t
{
	/* The allocation functions for this allocator. */
	allocator_t alloc;

	/* The buffer from which all allocated memory will be taken from */
	buffer_t * buffer;

} buffer_allocator_t;


/**
 * buffer_allocator_init
 *
 * Initialises the given buffer allocator allocator.
 *
 */
void buffer_allocator_init(
	buffer_allocator_t * allocator,
	buffer_t * buffer
);


/**
 * buffer_allocator_get
 *
 * Returns the given buffer allocator as an allocator_t pointer.
 *
 */
allocator_t * buffer_allocator_get(
	buffer_allocator_t * allocator
);


#if defined(__cplusplus)
} /* extern "C" */
#endif

#endif /* __MEM_BUFFER_H */
