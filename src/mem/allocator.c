#include "allocator.h"
#include "internal/unused.h"


/**
 * allocator_alloc
 *
 * Use the given allocator to allocate the given number of bytes.
 *
 */
void * allocator_alloc( size_t length, allocator_t * allocator )
{
	if ( !allocator || !allocator->alloc_fn )
	{
		return 0;
	}
	return allocator->alloc_fn( length, allocator );
}


/**
 * allocator_free
 *
 * Use the given allocator to free the given memory address.
 *
 */
void allocator_free( void * address, allocator_t * allocator )
{
	if ( allocator && allocator->free_fn )
	{
		allocator->free_fn( address, allocator );
	}
}


/**
 * _allocator_default_alloc
 *
 * Default allocation function that calls through to stdlib malloc
 *
 */
static void * _allocator_default_alloc( size_t length, allocator_t * allocator )
{
	UNUSED( allocator );
	if ( !length )
	{
		return 0;
	}
	else
	{
		return malloc( length );
	}
}


/**
 * _allocator_default_free
 *
 * Default free function that calls through to stdlib free
 *
 */
static void _allocator_default_free( void * address, allocator_t * allocator )
{
	UNUSED( allocator );

	if ( address )
	{
		free( address );
	}
}


/**
 * allocator_default
 *
 * Returns a pointer to the default allocator, which calls through to stdlib malloc / free
 *
 */
allocator_t * allocator_default( void )
{
	static allocator_t result = {
		&_allocator_default_alloc,
		&_allocator_default_free
	};

	return &result;
}


/**
 * _allocator_always_fail_alloc
 *
 * Always returns NULL - useful for testing.
 *
 */
static void * _allocator_always_fail_alloc( size_t length, allocator_t * allocator )
{
	UNUSED( length );
	UNUSED( allocator );
	return 0;
}


/**
 * _allocator_always_fail_free
 *
 * Does nothing.
 *
 */
static void _allocator_always_fail_free( void * address, allocator_t * allocator )
{
	UNUSED( address );
	UNUSED( allocator );
}


/**
 * allocator_always_fail
 *
 * Returns a pointer to an allocator that always returns NULL - useful for testing
 *
 */
allocator_t * allocator_always_fail( void )
{
	static allocator_t result = {
		&_allocator_always_fail_alloc,
		&_allocator_always_fail_free
	};

	return &result;
}


/**
 * _allocator_aligned_alloc
 *
 * Allocates memory aligned to the given number of bytes.
 *
 */
static void * _allocator_aligned_alloc( size_t length, allocator_t * allocator )
{
	allocator_aligned_t * alloc;
	char * block, * unaligned, * aligned;

	if ( !length || !allocator )
	{
		return 0;
	}

	alloc = ( allocator_aligned_t * ) allocator;

	if ( !alloc->alignment )
	{
		return allocator_alloc( length, alloc->parent );
	}

	/* When we allocate a block of aligned memory, we'll allocate enough
	 * space to accommodate the amount of bytes requested, plus enough
	 * padding to align the buffer if necessary, and additional space
	 * to store a pointer to the original unaligned buffer. */
	block = allocator_alloc(
		length + alloc->alignment + sizeof( void* ),
		alloc->parent
	);

	if ( !block )
	{
		return 0;
	}

	/* The aligned address that we will return to the user will sit at the end
	 * of the memory block we allocated above. This address could potentially
	 * be unaligned... */
	unaligned = block + alloc->alignment + sizeof( void* );

	/* ... if it is, use the additional space we allocated to adjust the address
	 * backwards, such that it is aligned. */
	aligned = unaligned - ( ( size_t ) unaligned % alloc->alignment );

	/* As the aligned address that we will return back to the user is not the
	 * original address returned my 'malloc', passing this to 'free' will result
	 * in an error. So, we will store a pointer to the original buffer in the bytes
	 * immediately preceeding the aligned block of memory. */
	*( ( char ** )( aligned - sizeof( void * ) ) ) = block;

	/* Finally, return the aligned block of memory back to the user. */
	return aligned;
}


/**
 * _allocator_aligned_free
 *
 * Frees memory that was allocated using an aligned memory allocator.
 *
 */
static void _allocator_aligned_free( void * address, allocator_t * allocator )
{
	allocator_aligned_t * alloc;
	char * block;

	if ( !address || !allocator )
	{
		return;
	}

	alloc = ( allocator_aligned_t * ) allocator;

	if ( !alloc->alignment )
	{
		allocator_free( address, alloc->parent );
		return;
	}

	/* If the user performed an aligned allocation, a pointer to the original
	 * (e.g. unaligned) block of memory will be stored in the bytes immediately
	 * preceeding the aligned block of memory. Obtain this original pointer... */
	block = *( void ** ) ( ( char * ) address - sizeof( void * ) );

	/* ... and release it. */
	allocator_free( block, alloc->parent );
}


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
)
{
	allocator->alloc.alloc_fn = &_allocator_aligned_alloc;
	allocator->alloc.free_fn = &_allocator_aligned_free;
	allocator->parent = parent;
	allocator->alignment = alignment;
}


/**
 * allocator_aligned_init_default
 *
 * Initialises the given aligned memory allocator, using the default allocator for
 * underlying memory allocations.
 *
 */
void allocator_aligned_init_default( allocator_aligned_t * allocator, size_t alignment )
{
	allocator_aligned_init( allocator, allocator_default( ), alignment );
}


/**
 * allocator_aligned_get
 *
 * Returns the given aligned allocator as an allocator_t pointer.
 *
 */
allocator_t * allocator_aligned_get(
	allocator_aligned_t * allocator
)
{
	return &allocator->alloc;
}


/**
 * _allocator_guarded_value
 *
 * The hardcoded value that surrounds a guarded allocation to ensure the
 * bounds are not overwritten
 *
 */
static size_t _allocator_guarded_value = 0xbaadf00d;


/**
 * _allocator_guarded_alloc
 *
 * Allocates the given amount of memory, guarded by an additional padding value
 * that is initialised to a special value on allocation (this function) and validated
 * when the memory is free'd (see below)
 *
 */
static void * _allocator_guarded_alloc( size_t length, allocator_t * allocator )
{
	size_t total;
	char * begin, * end;

	/* Obtain the underlying guarded allocator */
	allocator_guarded_t * guarded = ( allocator_guarded_t * ) allocator;

	/* Ensure requested block of memory is not zero-length */
	if ( !length )
	{
		return 0;
	}

	/* Allocate memory, requesting space for 2 additonal size_t at the
	 * beginning of the block (size + guard), and 2 additional size_t at
	 * the end of the block (guard + size). */
	total = length + ( 4 * sizeof( size_t ) );
	begin = allocator_alloc( total, guarded->parent );
	if ( !begin )
	{
		return 0;
	}
	end = ( ( char * ) begin ) + total;

	/* Populate guard structures */
	*( ( ( size_t * ) begin ) + 0 ) = length;
	*( ( ( size_t * ) begin ) + 1 ) = _allocator_guarded_value;
	*( ( ( size_t * ) end ) - 1 ) = length;
	*( ( ( size_t * ) end ) - 2 ) = _allocator_guarded_value;

	/* Return address of user memory */
	return ( ( size_t * ) begin ) + 2;
}


/**
 * _allocator_guarded_free
 *
 * Frees the given amount of memory that was allocated with a guarded allocator.
 * Verifies that the special padding value that surrounds the requested chunk of
 * memory remains unchanged.
 *
 */
static void _allocator_guarded_free( void * address, allocator_t * allocator )
{
	size_t length;
	void * begin, * end;

	/* Obtain the underlying guarded allocator */
	allocator_guarded_t * guarded = ( allocator_guarded_t * ) allocator;
	if ( !guarded->parent )
	{
		return;
	}

	/* Ensure the guarded block of memory is valid. If not, then it is
	 * not safe to proceed... */
	length = allocator_guarded_length( address );
	if ( !length )
	{
		return;
	}

	/* Clean up guard structure */
	begin = ( ( size_t * ) address ) - 2;
	end = ( ( char * ) address ) + length + ( 2 * sizeof( size_t ) );

	*( ( ( size_t * ) begin ) + 0 ) = 0;
	*( ( ( size_t * ) begin ) + 1 ) = 0;
	*( ( ( size_t * ) end ) - 1 ) = 0;
	*( ( ( size_t * ) end ) - 2 ) = 0;

	/* Release block of memory (including guard data) */
	allocator_free( begin, guarded->parent );
}


/**
 * allocator_guarded_init
 *
 * Initialises the given guarded allocator.
 *
 */
void allocator_guarded_init(
	allocator_guarded_t * allocator,
	allocator_t * parent
)
{
	allocator->alloc.alloc_fn = &_allocator_guarded_alloc;
	allocator->alloc.free_fn = &_allocator_guarded_free;
	allocator->parent = parent;
}


/**
 * allocator_guarded_init_default
 *
 * Initialises the given guarded allocator, using the default allocator for underlying
 * memory allocations.
 *
 */
void allocator_guarded_init_default(
	allocator_guarded_t * allocator
)
{
	allocator_guarded_init( allocator, allocator_default( ) );
}


/**
 * allocator_guarded_get
 *
 * Returns the given guarded allocator as an allocator_t pointer.
 *
 */
allocator_t * allocator_guarded_get(
	allocator_guarded_t * allocator
)
{
	return &allocator->alloc;
}


/**
 * allocator_guarded_length
 *
 * Returns the length in bytes of a guarded block of memory (as requested by the
 * user, excluding additional memory required to manage the block of memory).
 * Returns 0 if the given memory block is not valid (e.g. has been released,
 * was not allocated with a guarded allocator, or has invalidated guard blocks)
 *
 */
size_t allocator_guarded_length( void * address )
{
	size_t * begin, * end;

	if ( !address )
	{
		return 0;
	}

	begin = ( ( size_t * ) address ) - 2;
	if ( !*begin || *( begin + 1 ) != _allocator_guarded_value )
	{
		return 0;
	}

	end = ( size_t * )( ( ( char * ) address ) + *begin ) + 2;
	if ( !*( end - 1 ) || *( end - 2 ) != _allocator_guarded_value || *begin != *( end - 1 ) )
	{
		return 0;
	}

	return *begin;
}


/**
 * _allocator_traced_alloc
 *
 * Allocates the given number of bytes and writes a trace message about this event to the
 * allocator's FILE descriptor
 *
 */
static void * _allocator_traced_alloc(
	size_t length,
	allocator_t * allocator
)
{
	allocator_traced_t * traced;
	void * result;

	if ( !allocator )
	{
		return 0;
	}

	traced = ( allocator_traced_t * ) allocator;
	result = allocator_alloc( length, traced->parent );
	fprintf( traced->fd, "Allocated %lu bytes in block %p\n", ( long unsigned ) length, result );
	return result;
}


/**
 * _allocator_traced_free
 *
 * Frees the given memory and writes a trace message about this event to the allocator's
 * FILE descriptor
 *
 */
static void _allocator_traced_free(
	void * address,
	allocator_t * allocator
)
{
	allocator_traced_t * traced = ( allocator_traced_t * ) allocator;
	allocator_free( address, traced->parent );
	fprintf( traced->fd, "Released block %p\n", address );
}


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
)
{
	if ( allocator )
	{
		allocator->alloc.alloc_fn = &_allocator_traced_alloc;
		allocator->alloc.free_fn = &_allocator_traced_free;
		allocator->parent = parent;
		allocator->fd = fd;
	}
}


/**
 * allocator_traced_init_stdout
 *
 * Initialises the given traced allocator, with trace messages being sent to stdout
 *
 */
void allocator_traced_init_stdout(
	allocator_traced_t * allocator,
	allocator_t * parent
)
{
	allocator_traced_init( allocator, parent, stdout );
}


/**
 * allocator_traced_init_stderr
 *
 * Initialises the given traced allocator, with trace messages being sent to stderr
 *
 */
void allocator_traced_init_stderr(
	allocator_traced_t * allocator,
	allocator_t * parent
)
{
	allocator_traced_init( allocator, parent, stderr );
}


/**
 * allocator_traced_get
 *
 * Returns the given traced allocator as an allocator_t pointer.
 *
 */
allocator_t * allocator_traced_get(
	allocator_traced_t * allocator
)
{
	return &allocator->alloc;
}


/**
 * _allocator_counted_alloc
 *
 * Allocates the given number of bytes and writes a trace message about this event to the
 * allocator's FILE descriptor
 *
 */
static void * _allocator_counted_alloc(
	size_t length,
	allocator_t * allocator
)
{
	allocator_counted_t * counted;
	void * result;

	if ( !allocator || !length )
	{
		return 0;
	}

	counted = ( allocator_counted_t * ) allocator;
	result = allocator_alloc( length + sizeof( size_t ), counted->parent );

	if ( result )
	{
		*( ( size_t * ) result ) = length;

		counted->current += length;
		if ( counted->current > counted->peak )
		{
			counted->peak = counted->current;
		}

		return ( ( size_t * ) result ) + 1;
	}
	else
	{
		return 0;
	}
}


/**
 * _allocator_counted_free
 *
 * Frees the given memory and writes a trace message about this event to the allocator's
 * FILE descriptor
 *
 */
static void _allocator_counted_free(
	void * address,
	allocator_t * allocator
)
{
	allocator_counted_t * counted;
	size_t * fixup;

	if ( !allocator || !address )
	{
		return;
	}

	counted = ( allocator_counted_t * ) allocator;
	fixup = ( ( size_t * ) address ) - 1;
	counted->current -= *fixup;
	allocator_free( fixup, counted->parent );
}


/**
 * allocator_counted_init
 *
 * Initialises the given counted allocator.
 *
 */
void allocator_counted_init(
	allocator_counted_t * allocator,
	allocator_t * parent
)
{
	if ( allocator )
	{
		allocator->alloc.alloc_fn = &_allocator_counted_alloc;
		allocator->alloc.free_fn = &_allocator_counted_free;
		allocator->parent = parent;
		allocator->current = 0;
		allocator->peak = 0;
	}
}


/**
 * allocator_counted_init_default
 *
 * Initialises the given counted allocator, using the default allocator for underlying
 * memory allocations.
 *
 */
void allocator_counted_init_default(
	allocator_counted_t * allocator
)
{
	allocator_counted_init( allocator, allocator_default( ) );
}


/**
 * allocator_counted_get
 *
 * Returns the given counted allocator as an allocator_t pointer.
 *
 */
allocator_t * allocator_counted_get(
	allocator_counted_t * allocator
)
{
	if ( allocator )
	{
		return &allocator->alloc;
	}
	else
	{
		return 0;
	}
}


/**
 * allocator_counted_get_current_count
 *
 * Returns the number of bytes currently consumed by this allocator.
 *
 */
size_t allocator_counted_get_current_count(
	allocator_counted_t * allocator
)
{
	if ( allocator )
	{
		return allocator->current;
	}
	else
	{
		return 0;
	}
}


/**
 * allocator_counted_get_peak_count
 *
 * Returns the maximum number of bytes ever to be consumed by this allocator.
 *
 */
size_t allocator_counted_get_peak_count(
	allocator_counted_t * allocator
)
{
	if ( allocator )
	{
		return allocator->peak;
	}
	else
	{
		return 0;
	}
}

