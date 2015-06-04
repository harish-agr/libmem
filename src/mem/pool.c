#include "pool.h"


/* Allocates a pool_t structure and initialises it with capacity for the given
 * number of element of the given size, using the given allocator. The returned
 * pool should be passed to pool_delete once it is no longer needed */
pool_t * pool_new( size_t element_size, size_t num_elements, allocator_t * allocator )
{
	pool_t * result = ( pool_t * ) allocator_alloc( sizeof( pool_t ), allocator );
	if ( result )
	{
		pool_init( result, element_size, num_elements, allocator );
	}
	return result;
}


/* Releases the given pool_t structure and the resources it consumes. This function
 * should be used to release a pool returned by pool_new */
void pool_delete( pool_t * pool )
{
	if ( pool )
	{
		allocator_t * allocator = pool->allocator;
		pool_cleanup( pool );
		allocator_free( pool, allocator );
	}
}


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
void pool_init( pool_t * pool, size_t element_size, size_t num_elements, allocator_t * allocator )
{
	if ( pool && allocator )
	{
		if ( element_size && num_elements )
		{
			if ( element_size < sizeof( int8_t * ) )
			{
				element_size = sizeof( int8_t * );
			}

			pool->size = element_size * num_elements;
			pool->buffer = allocator_alloc( pool->size, allocator );
			pool->next = pool->buffer;

			if ( pool->buffer )
			{
				int8_t * current = pool->buffer;
				int8_t * last = current + pool->size - element_size;

				while ( current < last )
				{
					*( ( int8_t ** ) current ) = current + element_size;
					current += element_size;
				}

				*( ( int8_t ** ) last ) = 0;
			}
		}
		else
		{
			pool->buffer = pool->next = 0;
			pool->size = 0;
		}

		pool->allocator = allocator;
	}
}


/* Releases the underlying memory used by the given pool_t object. Use this function
 * to release the underlying resources for a pool initialised by pool_init */
void pool_cleanup( pool_t * pool )
{
	if ( pool )
	{
		if ( pool->buffer && pool->allocator )
		{
			allocator_free( pool->buffer, pool->allocator );
		}

		pool->buffer = 0;
		pool->next = 0;
		pool->size = 0;

		/* Note we're deliberately not resetting pool->allocator here such
		 * that if pool_delete is called afterwards, the allocator is still
		 * available such that the memory consumed by the pool_t object can
		 * be released
		 */
	}
}


/* Take an unused element from the pool and returns a pointer to it */
void * pool_take( pool_t * pool )
{
	if ( pool && pool->next )
	{
		int8_t * result = pool->next;
		pool->next = *( ( int8_t ** ) pool->next );
		return result;
	}

	return 0;
}


/* Return an element to the pool */
void pool_return( pool_t * pool, void * address )
{
	if ( pool && address )
	{
		void * end = ( ( int8_t * ) pool->buffer ) + pool->size;
		if ( address >= ( ( void * ) pool->buffer ) && address < end )
		{
			*( ( int8_t ** ) address ) = pool->next;
			pool->next = ( int8_t * ) address;
		}
	}
}


/* Return 1 if there are no more free elements in the pool to return, or 0 otherwise */
int pool_is_empty( pool_t * pool )
{
	if ( pool )
	{
		if ( pool->next )
		{
			return 0;
		}
	}

	return 1;
}
