#include "buffer.h"

#include <string.h>

/**
 * buffer_new
 *
 * Allocate a new buffer_t object using the given allocator, and initialise the
 * buffer as an empty buffer. Should call buffer_delete to release the buffer_t
 * object, and its underlying memory buffer.
 */
buffer_t * buffer_new( allocator_t * allocator )
{
	buffer_t * buffer = ( buffer_t * ) allocator_alloc( sizeof( buffer_t ), allocator );
	if ( buffer )
	{
		buffer_init( buffer, allocator );
	}
	return buffer;
}


/**
 * buffer_delete
 *
 * Releases the given buffer_t object and its underlying memory buffer. Use this
 * function to release a buffer_t object created with buffer_new.
 *
 */
void buffer_delete( buffer_t * buffer )
{
	if ( buffer )
	{
		allocator_t * allocator = buffer->allocator;
		buffer_cleanup( buffer );
		allocator_free( buffer, allocator );
	}
}


/**
 * buffer_init
 *
 * Initialises the underlying memory buffer in the given buffer_t object as an
 * empty buffer (with zero capacity). Will use the given allocator for all
 * subsequent allocations. Should call buffer_cleanup to release the underlying memory.
 *
 */
void buffer_init( buffer_t * buffer, allocator_t * allocator )
{
	if ( buffer )
	{
		buffer->allocator = allocator;
		buffer->begin = buffer->pos = 0;
		buffer->capacity = 0;
	}
}


/**
 * buffer_cleanup
 *
 * Releases the underlying memory buffer in the given buffer_t object (the buffer_t
 * object itself is not released). Use this function to clean up a buffer_t object that
 * was initialised with the buffer_init function.
 *
 */
void buffer_cleanup( buffer_t * buffer )
{
	if ( buffer )
	{
		if ( buffer->begin && buffer->allocator )
		{
			allocator_free( buffer->begin, buffer->allocator );
		}

		buffer->begin = 0;
		buffer->pos = 0;
		buffer->capacity = 0;

		/* Note that we deliberately do not reset the allocator pointer here,
		 * such that it is retained for buffer_delete, should buffer_cleanup
		 * be called on the buffer first
		 */
	}
}


/**
 * buffer_capacity
 *
 * Returns the number of bytes allocated for this buffer.
 *
 */
size_t buffer_capacity( buffer_t * buffer )
{
	if ( buffer && buffer->begin )
	{
		return buffer->capacity;
	}
	return 0;
}


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
size_t buffer_grow( buffer_t * buffer, size_t amount_in_bytes )
{
	if ( buffer && buffer->allocator )
	{
		if ( amount_in_bytes == 0 )
		{
			return buffer->capacity;
		}
		else if ( buffer->begin )
		{
			size_t new_capacity, data_length;
			void * new_buffer;

			/* Compute new capacity - return if overflow */
			new_capacity = buffer->capacity + amount_in_bytes;
			if ( new_capacity < buffer->capacity )
			{
				return buffer->capacity;
			}

			/* Allocate new buffer */
			new_buffer = allocator_alloc( new_capacity, buffer->allocator );
			if ( !new_buffer )
			{
				return 0;
			}

			/* Copy original data into new buffer. */
			data_length = buffer_data_length( buffer );
			memcpy( new_buffer, buffer->begin, data_length );

			/* Release the old buffer */
			allocator_free( buffer->begin, buffer->allocator );

			/* Update the buffer structure */
			buffer->capacity = new_capacity;
			buffer->begin = ( int8_t * ) new_buffer;
			buffer->pos = buffer->begin + data_length;

			/* Return the new total capacity */
			return new_capacity;
		}
		else
		{
			/* This is the first allocation */
			buffer->capacity = amount_in_bytes;
			buffer->begin = allocator_alloc( amount_in_bytes, buffer->allocator );
			buffer->pos = buffer->begin;
			return amount_in_bytes;
		}
	}

	return 0;
}


/**
 * buffer_data_length
 *
 * Returns the number of bytes that have been written to the buffer.
 *
 */
size_t buffer_data_length( buffer_t * buffer )
{
	if ( buffer && buffer->begin && buffer->pos > buffer->begin )
	{
		return ( size_t )( buffer->pos - buffer->begin );
	}

	return 0;
}


/**
 * buffer_data_pointer
 *
 * Returns a pointer to the underlying data buffer. This could return null
 * if the buffer has zero capacity.
 *
 */
void * buffer_data_pointer( buffer_t * buffer )
{
	if ( buffer && buffer->begin )
	{
		return buffer->begin;
	}
	return 0;
}


/**
 * buffer_rewind
 *
 * Rewinds the current buffer position to the beginning of the buffer.
 *
 */
void buffer_rewind( buffer_t * buffer )
{
	if ( buffer )
	{
		buffer->pos = buffer->begin;
	}
}


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
size_t buffer_append( buffer_t * buffer, size_t length, void * data )
{
	void * pos;

	/* Validate arguments */
	if ( !buffer || !length || !data )
	{
		return 0;
	}

	/* Reserve space at the current write position for the given number of bytes */
	pos = buffer_reserve( buffer, length );
	if ( !pos )
	{
		return 0;
	}

	/* Copy the given data into the buffer */
	memcpy( pos, data, length );
	return length;
}


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
void * buffer_reserve( buffer_t * buffer, size_t length )
{
	size_t current_size, new_size;
	void * result;

	/* Validate arguments */
	if ( !buffer || !length || !buffer->allocator )
	{
		return 0;
	}

	/* Compute new buffer size required - return if overflow */
	current_size = buffer_data_length( buffer );
	new_size = current_size + length;
	if ( new_size < current_size )
	{
		return 0;
	}

	/* Do we have enough space in the current buffer? */
	if ( new_size > buffer->capacity )
	{
		/* Nope - grow the buffer to accommodate the new bytes */
		if ( buffer_grow( buffer, new_size - buffer->capacity ) < new_size )
		{
			return 0;
		}
	}

	/* Now append the data at the current buffer position */
	result = buffer->pos;
	buffer->pos += length;
	return result;
}
