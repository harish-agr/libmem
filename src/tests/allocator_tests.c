#include <stdio.h>
#include <string.h>

#include "../mem/allocator.h"
#include "../mem/internal/unused.h"
#include "testing.h"


typedef struct _mock_allocator_t
{
	allocator_t alloc;
	unsigned alloc_count;
	unsigned free_count;
} _mock_allocator_t;


static void * _mock_allocator_alloc( size_t length, allocator_t * alloc )
{
	UNUSED( length );
	++( ( _mock_allocator_t * ) alloc )->alloc_count;
	return 0;
}


static void _mock_allocator_free( void * address, allocator_t * alloc )
{
	UNUSED( address );
	++( ( _mock_allocator_t * ) alloc )->free_count;
}


static void _mock_allocator_init( _mock_allocator_t * alloc )
{
	alloc->alloc.alloc_fn = &_mock_allocator_alloc;
	alloc->alloc.free_fn = &_mock_allocator_free;
	alloc->alloc_count = 0;
	alloc->free_count = 0;
}


static void _ensure_allocator_alloc_returns_null_when_passed_null_allocator( )
{
	TEST_REQUIRE( allocator_alloc( 1024, 0 ) == 0 );
}


static void _ensure_allocator_alloc_returns_null_when_allocator_fails( )
{
	TEST_REQUIRE( allocator_alloc( 1024, allocator_always_fail( ) ) == 0 );
}


static void _ensure_allocator_alloc_calls_alloc_fn( )
{
	_mock_allocator_t alloc;
	_mock_allocator_init( &alloc );
	TEST_REQUIRE( alloc.alloc_count == 0 );
	allocator_alloc( 1024, &alloc.alloc );
	TEST_REQUIRE( alloc.alloc_count == 1 );
}


static void _ensure_allocator_alloc_copes_with_null_alloc_fn( )
{
	allocator_t alloc = { 0, 0 };
	TEST_REQUIRE( allocator_alloc( 1024, &alloc ) == 0 );
}


static void _ensure_allocator_free_copes_with_null_address( )
{
	allocator_free( 0, allocator_always_fail( ) );
}


static void _ensure_allocator_free_copes_with_null_allocator( )
{
	int x = 0;
	allocator_free( &x, 0 );
}


static void _ensure_allocator_free_calls_free_fn( )
{
	_mock_allocator_t alloc;
	_mock_allocator_init( &alloc );
	TEST_REQUIRE( alloc.free_count == 0 );
	allocator_free( 0, &alloc.alloc );
	TEST_REQUIRE( alloc.free_count == 1 );
}


static void _ensure_allocator_free_copes_with_null_free_fn( )
{
	int x = 0;
	allocator_t alloc = { 0, 0 };
	allocator_free( &x, &alloc );
}


static void _ensure_allocator_default_returns_an_allocator( )
{
	TEST_REQUIRE( allocator_default( ) != 0 );
}


static void _ensure_allocator_default_can_allocate_and_release_memory( )
{
	void * mem = allocator_alloc( 1024, allocator_default( ) );
	TEST_REQUIRE( mem != 0 );
	allocator_free( mem, allocator_default( ) );
}


static void _ensure_allocator_default_returns_null_for_zero_length_allocation( )
{
	void * mem = allocator_alloc( 0, allocator_default( ) );
	TEST_REQUIRE( mem == 0 );
}


static void _ensure_allocator_default_free_copes_with_null_address( )
{
	allocator_free( 0, allocator_default( ) );
}


static void _ensure_allocator_always_fail_returns_an_allocator( )
{
	TEST_REQUIRE( allocator_always_fail( ) != 0 );
}


static void _ensure_allocator_always_fail_returns_null_memory( )
{
	TEST_REQUIRE( allocator_alloc( 1024, allocator_always_fail( ) ) == 0 );
}


static void _ensure_allocator_always_fail_free_copes_with_null_address( )
{
	allocator_free( 0, allocator_always_fail( ) );
}


int main( int argc, char * argv[] )
{
	UNUSED( argc );
	UNUSED( argv );

	_ensure_allocator_alloc_returns_null_when_passed_null_allocator( );
	_ensure_allocator_alloc_returns_null_when_allocator_fails( );
	_ensure_allocator_alloc_calls_alloc_fn( );
	_ensure_allocator_alloc_copes_with_null_alloc_fn( );
	_ensure_allocator_free_copes_with_null_address( );
	_ensure_allocator_free_copes_with_null_allocator( );
	_ensure_allocator_free_calls_free_fn( );
	_ensure_allocator_free_copes_with_null_free_fn( );
	_ensure_allocator_default_returns_an_allocator( );
	_ensure_allocator_default_can_allocate_and_release_memory( );
	_ensure_allocator_default_returns_null_for_zero_length_allocation( );
	_ensure_allocator_default_free_copes_with_null_address( );
	_ensure_allocator_always_fail_returns_an_allocator( );
	_ensure_allocator_always_fail_returns_null_memory( );
	_ensure_allocator_always_fail_free_copes_with_null_address( );
	return 0;
}
