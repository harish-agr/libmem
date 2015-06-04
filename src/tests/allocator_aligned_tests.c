#include <stdio.h>
#include <string.h>

#include "../mem/allocator.h"
#include "../mem/internal/unused.h"
#include "testing.h"


static void _ensure_allocator_aligned_init_sets_correct_parent_allocator( )
{
	allocator_aligned_t alloc;
	allocator_t * parent = allocator_default( );
	allocator_aligned_init( &alloc, parent, 8 );
	TEST_REQUIRE( alloc.parent == parent );
}


static void _ensure_allocator_aligned_init_sets_allocation_functions( )
{
	allocator_aligned_t alloc;
	allocator_aligned_init( &alloc, allocator_default( ), 8 );
	TEST_REQUIRE( alloc.alloc.alloc_fn );
	TEST_REQUIRE( alloc.alloc.free_fn );
}


static void _ensure_allocator_aligned_init_sets_internal_alignment( )
{
	allocator_aligned_t alloc;
	allocator_aligned_init( &alloc, allocator_default( ), 8 );
	TEST_REQUIRE( alloc.alignment == 8 );
}


static void _ensure_allocator_aligned_init_default_sets_default_allocator( )
{
	allocator_aligned_t alloc;
	allocator_aligned_init_default( &alloc, 8 );
	TEST_REQUIRE( alloc.parent == allocator_default( ) );
}


static void _ensure_allocator_aligned_init_default_sets_allocation_functions( )
{
	allocator_aligned_t alloc;
	allocator_aligned_init_default( &alloc, 8 );
	TEST_REQUIRE( alloc.alloc.alloc_fn );
	TEST_REQUIRE( alloc.alloc.free_fn );
}


static void _ensure_allocator_aligned_init_default_sets_internal_alignment( )
{
	allocator_aligned_t alloc;
	allocator_aligned_init_default( &alloc, 8 );
	TEST_REQUIRE( alloc.alignment == 8 );
}


static void _ensure_allocator_aligned_get_returns_internal_allocator( )
{
	allocator_aligned_t alloc;
	allocator_aligned_init_default( &alloc, 8 );
	TEST_REQUIRE( allocator_aligned_get( &alloc ) == &alloc.alloc );
}


static void _ensure_allocator_aligned_alloc_returns_null_for_empty_allocation( )
{
	allocator_aligned_t alloc;
	allocator_aligned_init_default( &alloc, 8 );
	TEST_REQUIRE( allocator_alloc( 0, allocator_aligned_get( &alloc ) ) == 0 );
}


static void _ensure_allocator_aligned_alloc_returns_null_when_parent_allocator_null( )
{
	allocator_aligned_t alloc;
	allocator_aligned_init( &alloc, 0, 8 );
	TEST_REQUIRE( allocator_alloc( 1024, allocator_aligned_get( &alloc ) ) == 0 );
}


static void _ensure_allocator_aligned_alloc_returns_null_when_parent_allocator_fails( )
{
	allocator_aligned_t alloc;
	allocator_aligned_init( &alloc, allocator_always_fail( ), 8 );
	TEST_REQUIRE( allocator_alloc( 1024, allocator_aligned_get( &alloc ) ) == 0 );
}


static void _ensure_allocator_aligned_alloc_returns_valid_memory_for_nonempty_allocation( )
{
	void * mem;
	allocator_aligned_t alloc;
	allocator_aligned_init_default( &alloc, 8 );
	mem = allocator_alloc( 1024, allocator_aligned_get( &alloc ) );
	TEST_REQUIRE( mem != 0 );
	allocator_free( mem, allocator_aligned_get( &alloc ) );
}


static void _ensure_allocator_aligned_alloc_returns_correctly_aligned_address( )
{
	void * mem;
	allocator_aligned_t alloc;
	allocator_aligned_init_default( &alloc, 7 );
	mem = allocator_alloc( 1024, allocator_aligned_get( &alloc ) );
	TEST_REQUIRE( ( ( size_t ) mem ) % 7 == 0 );
	allocator_free( mem, allocator_aligned_get( &alloc ) );
}


static void _ensure_allocator_aligned_free_copes_with_null_parent_allocator( )
{
	int x = 0;
	allocator_aligned_t alloc;
	allocator_aligned_init( &alloc, 0, 8 );
	allocator_free( &x, allocator_aligned_get( &alloc ) );
}


static void _ensure_allocator_aligned_copes_with_alignment_value_of_zero( )
{
	void * mem;
	allocator_aligned_t alloc;
	allocator_aligned_init_default( &alloc, 0 );
	mem = allocator_alloc( 1024, allocator_aligned_get( &alloc ) );
	TEST_REQUIRE( mem != 0 );
	allocator_free( mem, allocator_aligned_get( &alloc ) );
}


static void _ensure_allocator_aligned_copes_with_alignment_value_of_one( )
{
	void * mem;
	allocator_aligned_t alloc;
	allocator_aligned_init_default( &alloc, 1 );
	mem = allocator_alloc( 1024, allocator_aligned_get( &alloc ) );
	TEST_REQUIRE( mem != 0 );
	allocator_free( mem, allocator_aligned_get( &alloc ) );
}


int main( int argc, char * argv[] )
{
	UNUSED( argc );
	UNUSED( argv );

	_ensure_allocator_aligned_init_sets_correct_parent_allocator( );
	_ensure_allocator_aligned_init_sets_allocation_functions( );
	_ensure_allocator_aligned_init_sets_internal_alignment( );
	_ensure_allocator_aligned_init_default_sets_default_allocator( );
	_ensure_allocator_aligned_init_default_sets_allocation_functions( );
	_ensure_allocator_aligned_init_default_sets_internal_alignment( );
	_ensure_allocator_aligned_get_returns_internal_allocator( );
	_ensure_allocator_aligned_alloc_returns_null_for_empty_allocation( );
	_ensure_allocator_aligned_alloc_returns_null_when_parent_allocator_null( );
	_ensure_allocator_aligned_alloc_returns_null_when_parent_allocator_fails( );
	_ensure_allocator_aligned_alloc_returns_valid_memory_for_nonempty_allocation( );
	_ensure_allocator_aligned_alloc_returns_correctly_aligned_address( );
	_ensure_allocator_aligned_free_copes_with_null_parent_allocator( );
	_ensure_allocator_aligned_copes_with_alignment_value_of_zero( );
	_ensure_allocator_aligned_copes_with_alignment_value_of_one( );
	return 0;
}
