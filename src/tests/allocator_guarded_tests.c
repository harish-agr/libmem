#include <stdio.h>
#include <string.h>

#include "../mem/allocator.h"
#include "../mem/internal/unused.h"
#include "testing.h"


static void ensure_allocator_guarded_init_sets_correct_parent_allocator( void )
{
	allocator_guarded_t alloc;
	allocator_t * parent = allocator_default( );
	allocator_guarded_init( &alloc, parent );
	TEST_REQUIRE( alloc.parent == parent );
}


static void ensure_allocator_guarded_init_sets_allocation_functions( void )
{
	allocator_guarded_t alloc;
	allocator_guarded_init( &alloc, allocator_default( ) );
	TEST_REQUIRE( alloc.alloc.alloc_fn );
	TEST_REQUIRE( alloc.alloc.free_fn );
}


static void ensure_allocator_guarded_init_default_sets_default_allocator( void )
{
	allocator_guarded_t alloc;
	allocator_guarded_init_default( &alloc );
	TEST_REQUIRE( alloc.parent == allocator_default( ) );
}


static void ensure_allocator_guarded_init_default_sets_allocation_functions( void )
{
	allocator_guarded_t alloc;
	allocator_guarded_init_default( &alloc );
	TEST_REQUIRE( alloc.alloc.alloc_fn );
	TEST_REQUIRE( alloc.alloc.free_fn );
}


static void ensure_allocator_guarded_get_returns_internal_allocator( void )
{
	allocator_guarded_t alloc;
	allocator_guarded_init_default( &alloc );
	TEST_REQUIRE( allocator_guarded_get( &alloc ) == &alloc.alloc );
}


static void ensure_allocator_guarded_alloc_returns_null_for_empty_allocation( void )
{
	allocator_guarded_t alloc;
	allocator_guarded_init_default( &alloc );
	TEST_REQUIRE( allocator_alloc( 0, allocator_guarded_get( &alloc ) ) == 0 );
}


static void ensure_allocator_guarded_alloc_returns_null_when_parent_allocator_null( void )
{
	allocator_guarded_t alloc;
	allocator_guarded_init( &alloc, 0 );
	TEST_REQUIRE( allocator_alloc( 1024, allocator_guarded_get( &alloc ) ) == 0 );
}


static void ensure_allocator_guarded_alloc_returns_null_when_parent_allocator_fails( void )
{
	allocator_guarded_t alloc;
	allocator_guarded_init( &alloc, allocator_always_fail( ) );
	TEST_REQUIRE( allocator_alloc( 1024, allocator_guarded_get( &alloc ) ) == 0 );
}


static void ensure_allocator_guarded_alloc_returns_valid_memory_for_nonempty_allocation( void )
{
	void * mem;
	allocator_guarded_t alloc;
	allocator_guarded_init_default( &alloc );
	mem = allocator_alloc( 1024, allocator_guarded_get( &alloc ) );
	TEST_REQUIRE( mem != 0 );
	allocator_free( mem, allocator_guarded_get( &alloc ) );
}


static void ensure_allocator_guarded_free_copes_with_null_parent_allocator( void )
{
	int x = 0;
	allocator_guarded_t alloc;
	allocator_guarded_init( &alloc, 0 );
	allocator_free( &x, allocator_guarded_get( &alloc ) );
}


static void ensure_allocator_guarded_length_returns_correct_value_for_successful_allocation( void )
{
	void * mem;
	allocator_guarded_t alloc;
	allocator_guarded_init_default( &alloc );
	mem = allocator_alloc( 1024, allocator_guarded_get( &alloc ) );
	TEST_REQUIRE( mem != 0 );
	TEST_REQUIRE( allocator_guarded_length( mem ) == 1024 );
	allocator_free( mem, allocator_guarded_get( &alloc ) );
}


static void ensure_allocator_guarded_length_returns_zero_when_address_not_guarded( void )
{
	char buffer[128];
	memset( buffer, 0, sizeof( buffer ) );
	TEST_REQUIRE( allocator_guarded_length( buffer + 64 ) == 0 );
}


static void ensure_allocator_guarded_length_returns_zero_when_start_guard_corrupted( void )
{
	size_t * buffer;
	size_t old_guard_value;
	allocator_guarded_t alloc;

	allocator_guarded_init_default( &alloc );
	buffer = ( size_t * ) allocator_alloc(
		sizeof( size_t ),
		allocator_guarded_get( &alloc )
	);
	TEST_REQUIRE( buffer );
	old_guard_value = *( buffer - 1 );
	*( buffer - 1 ) = 0;
	TEST_REQUIRE( allocator_guarded_length( buffer ) == 0 );

	/* Now put the guard value back so that we can release the underlying memory */
	*( buffer - 1 ) = old_guard_value;
	allocator_free( buffer, allocator_guarded_get( &alloc ) );
}


static void ensure_allocator_guarded_length_returns_zero_when_end_guard_corrupted( void )
{
	size_t * buffer;
	size_t old_guard_value;
	allocator_guarded_t alloc;

	allocator_guarded_init_default( &alloc );
	buffer = ( size_t * ) allocator_alloc(
		sizeof( size_t ),
		allocator_guarded_get( &alloc )
	);
	TEST_REQUIRE( buffer );
	old_guard_value = *( buffer + 1 );
	*( buffer + 1 ) = 0;
	TEST_REQUIRE( allocator_guarded_length( buffer ) == 0 );

	/* Now put the guard value back so that we can release the underlying memory */
	*( buffer + 1 ) = old_guard_value;
	allocator_free( buffer, allocator_guarded_get( &alloc ) );
}


int main( int argc, char * argv[] )
{
	UNUSED( argc );
	UNUSED( argv );
	ensure_allocator_guarded_init_sets_correct_parent_allocator( );
	ensure_allocator_guarded_init_sets_allocation_functions( );
	ensure_allocator_guarded_init_default_sets_default_allocator( );
	ensure_allocator_guarded_init_default_sets_allocation_functions( );
	ensure_allocator_guarded_get_returns_internal_allocator( );
	ensure_allocator_guarded_alloc_returns_null_for_empty_allocation( );
	ensure_allocator_guarded_alloc_returns_null_when_parent_allocator_null( );
	ensure_allocator_guarded_alloc_returns_null_when_parent_allocator_fails( );
	ensure_allocator_guarded_alloc_returns_valid_memory_for_nonempty_allocation( );
	ensure_allocator_guarded_free_copes_with_null_parent_allocator( );
	ensure_allocator_guarded_length_returns_correct_value_for_successful_allocation( );
	ensure_allocator_guarded_length_returns_zero_when_address_not_guarded( );
	ensure_allocator_guarded_length_returns_zero_when_start_guard_corrupted( );
	ensure_allocator_guarded_length_returns_zero_when_end_guard_corrupted( );
	return 0;
}
