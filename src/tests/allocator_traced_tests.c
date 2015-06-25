#include <stdio.h>
#include <string.h>

#include "../mem/allocator.h"
#include "../mem/internal/unused.h"
#include "testing.h"


static void _ensure_allocator_traced_init_copes_with_null_allocator( void )
{
	allocator_traced_init( 0, allocator_default( ), stdout );
}


static void _ensure_allocator_traced_init_sets_correct_parent_allocator( void )
{
	allocator_traced_t alloc;
	allocator_t * parent = allocator_default( );
	allocator_traced_init( &alloc, parent, stdout );
	TEST_REQUIRE( alloc.parent == parent );
}


static void _ensure_allocator_traced_init_sets_allocation_functions( void )
{
	allocator_traced_t alloc;
	allocator_traced_init( &alloc, allocator_default( ), stdout );
	TEST_REQUIRE( alloc.alloc.alloc_fn );
	TEST_REQUIRE( alloc.alloc.free_fn );
}


static void _ensure_allocator_traced_init_stdout_copes_with_null_allocator( void )
{
	allocator_traced_init_stdout( 0, allocator_default( ) );
}


static void _ensure_allocator_traced_init_stdout_sets_parent_allocator( void )
{
	allocator_traced_t alloc;
	allocator_traced_init_stdout( &alloc, allocator_default( ) );
	TEST_REQUIRE( alloc.parent == allocator_default( ) );
}


static void _ensure_allocator_traced_init_stderr_copes_with_null_allocator( void )
{
	allocator_traced_init_stderr( 0, allocator_default( ) );
}


static void _ensure_allocator_traced_init_stdout_sets_allocation_functions( void )
{
	allocator_traced_t alloc;
	allocator_traced_init_stdout( &alloc, allocator_default( ) );
	TEST_REQUIRE( alloc.alloc.alloc_fn );
	TEST_REQUIRE( alloc.alloc.free_fn );
}


static void _ensure_allocator_traced_init_stderr_sets_parent_allocator( void )
{
	allocator_traced_t alloc;
	allocator_traced_init_stderr( &alloc, allocator_default( ) );
	TEST_REQUIRE( alloc.parent == allocator_default( ) );
}


static void _ensure_allocator_traced_init_stderr_sets_allocation_functions( void )
{
	allocator_traced_t alloc;
	allocator_traced_init_stderr( &alloc, allocator_default( ) );
	TEST_REQUIRE( alloc.alloc.alloc_fn );
	TEST_REQUIRE( alloc.alloc.free_fn );
}


static void _ensure_allocator_traced_get_returns_internal_allocator( void )
{
	allocator_traced_t alloc;
	allocator_traced_init_stdout( &alloc, allocator_default( ) );
	TEST_REQUIRE( allocator_traced_get( &alloc ) == &alloc.alloc );
}


static void _ensure_allocator_traced_alloc_returns_null_for_empty_allocation( void )
{
	allocator_traced_t alloc;
	allocator_traced_init_stdout( &alloc, allocator_default( ) );
	TEST_REQUIRE( allocator_alloc( 0, allocator_traced_get( &alloc ) ) == 0 );
}


static void _ensure_allocator_traced_alloc_returns_null_when_parent_allocator_null( void )
{
	allocator_traced_t alloc;
	allocator_traced_init( &alloc, 0, stdout );
	TEST_REQUIRE( allocator_alloc( 1024, allocator_traced_get( &alloc ) ) == 0 );
}


static void _ensure_allocator_traced_alloc_returns_null_when_parent_allocator_fails( void )
{
	allocator_traced_t alloc;
	allocator_traced_init( &alloc, allocator_always_fail( ), stdout );
	TEST_REQUIRE( allocator_alloc( 1024, allocator_traced_get( &alloc ) ) == 0 );
}


static void _ensure_allocator_traced_alloc_returns_valid_memory_for_nonempty_allocation( void )
{
	void * mem;
	allocator_traced_t alloc;
	allocator_traced_init_stdout( &alloc, allocator_default( ) );
	mem = allocator_alloc( 1024, allocator_traced_get( &alloc ) );
	TEST_REQUIRE( mem != 0 );
	allocator_free( mem, allocator_traced_get( &alloc ) );
}


static void _ensure_allocator_traced_free_copes_with_null_parent_allocator( void )
{
	int x = 0;
	allocator_traced_t alloc;
	allocator_traced_init( &alloc, 0, stdout );
	allocator_free( &x, allocator_traced_get( &alloc ) );
}


int main( int argc, char * argv[] )
{
	UNUSED( argc );
	UNUSED( argv );

	_ensure_allocator_traced_init_copes_with_null_allocator( );
	_ensure_allocator_traced_init_sets_correct_parent_allocator( );
	_ensure_allocator_traced_init_sets_allocation_functions( );
	_ensure_allocator_traced_init_stdout_copes_with_null_allocator( );
	_ensure_allocator_traced_init_stdout_sets_parent_allocator( );
	_ensure_allocator_traced_init_stderr_copes_with_null_allocator( );
	_ensure_allocator_traced_init_stdout_sets_allocation_functions( );
	_ensure_allocator_traced_init_stderr_sets_parent_allocator( );
	_ensure_allocator_traced_init_stderr_sets_allocation_functions( );
	_ensure_allocator_traced_get_returns_internal_allocator( );
	_ensure_allocator_traced_alloc_returns_null_for_empty_allocation( );
	_ensure_allocator_traced_alloc_returns_null_when_parent_allocator_null( );
	_ensure_allocator_traced_alloc_returns_null_when_parent_allocator_fails( );
	_ensure_allocator_traced_alloc_returns_valid_memory_for_nonempty_allocation( );
	_ensure_allocator_traced_free_copes_with_null_parent_allocator( );
	return 0;
}
