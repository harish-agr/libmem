#include "../mem/allocator.h"
#include "../mem/internal/unused.h"
#include "testing.h"


static void _ensure_allocator_counted_init_copes_with_null_allocator( void )
{
	allocator_counted_init( 0, allocator_default( ) );
}


static void _ensure_allocator_counted_init_copes_with_null_parent_allocator( void )
{
	allocator_counted_t alloc;
	allocator_counted_init( &alloc, 0 );
}


static void _ensure_allocator_counted_init_sets_correct_parent_allocator( void )
{
	allocator_counted_t alloc;
	allocator_counted_init( &alloc, allocator_default( ) );
	TEST_REQUIRE( alloc.parent == allocator_default( ) );
}


static void _ensure_allocator_counted_init_sets_allocation_functions( void )
{
	allocator_counted_t alloc;
	allocator_counted_init( &alloc, allocator_default( ) );
	TEST_REQUIRE( alloc.alloc.alloc_fn );
	TEST_REQUIRE( alloc.alloc.free_fn );
}


static void _ensure_allocator_counted_init_default_copes_with_null_allocator( void )
{
	allocator_counted_init_default( 0 );
}


static void _ensure_allocator_counted_init_default_sets_correct_parent_allocator( void )
{
	allocator_counted_t alloc;
	allocator_counted_init_default( &alloc );
	TEST_REQUIRE( alloc.parent == allocator_default( ) );
}


static void _ensure_allocator_counted_init_default_sets_allocation_functions( void )
{
	allocator_counted_t alloc;
	allocator_counted_init_default( &alloc );
	TEST_REQUIRE( alloc.alloc.alloc_fn );
	TEST_REQUIRE( alloc.alloc.free_fn );
}


static void _ensure_allocator_counted_get_returns_internal_allocator( void )
{
	allocator_counted_t alloc;
	allocator_counted_init_default( &alloc );
	TEST_REQUIRE( allocator_counted_get( &alloc ) == &alloc.alloc );
}


static void _ensure_allocator_counted_alloc_returns_null_for_empty_allocation( void )
{
	allocator_counted_t alloc;
	allocator_counted_init_default( &alloc );
	TEST_REQUIRE( allocator_alloc( 0, allocator_counted_get( &alloc ) ) == 0 );
}


static void _ensure_allocator_counted_alloc_returns_null_when_parent_allocator_null( void )
{
	allocator_counted_t alloc;
	allocator_counted_init( &alloc, 0 );
	TEST_REQUIRE( allocator_alloc( 1024, allocator_counted_get( &alloc ) ) == 0 );
}


static void _ensure_allocator_counted_alloc_returns_null_when_parent_allocator_fails( void )
{
	allocator_counted_t alloc;
	allocator_counted_init( &alloc, allocator_always_fail() );
	TEST_REQUIRE( allocator_alloc( 1024, allocator_counted_get( &alloc ) ) == 0 );
}


static void _ensure_allocator_counted_alloc_updates_count_when_allocation_succeeds( void )
{
	void * a, * b;

	allocator_counted_t alloc;
	allocator_counted_init_default( &alloc );
	TEST_REQUIRE( allocator_counted_get_current_count( &alloc ) == 0 );
	a = allocator_alloc( 1024, allocator_counted_get( &alloc ) );
	TEST_REQUIRE( a );
	TEST_REQUIRE( allocator_counted_get_current_count( &alloc ) == 1024 );
	b = allocator_alloc( 1024, allocator_counted_get( &alloc ) );
	TEST_REQUIRE( b );
	TEST_REQUIRE( allocator_counted_get_current_count( &alloc ) == 2048 );
	allocator_free( a, allocator_counted_get( &alloc ) );
	TEST_REQUIRE( allocator_counted_get_current_count( &alloc ) == 1024 );
	allocator_free( b, allocator_counted_get( &alloc ) );
	TEST_REQUIRE( allocator_counted_get_current_count( &alloc ) == 0 );
}


static void _ensure_allocator_counted_alloc_doesnt_update_count_when_allocation_failed( void )
{
	void * a;
	allocator_counted_t alloc;
	allocator_counted_init( &alloc, allocator_always_fail( ) );
	TEST_REQUIRE( allocator_counted_get_current_count( &alloc ) == 0 );
	a = allocator_alloc( 1024, allocator_counted_get( &alloc ) );
	TEST_REQUIRE( a == 0 );
	TEST_REQUIRE( allocator_counted_get_current_count( &alloc ) == 0 );
}


static void _ensure_allocator_counted_alloc_maintains_correct_peak_count( void )
{
	void * a, * b;
	allocator_counted_t alloc;
	allocator_counted_init_default( &alloc );
	TEST_REQUIRE( allocator_counted_get_peak_count( &alloc ) == 0 );
	a = allocator_alloc( 1024, allocator_counted_get( &alloc ) );
	TEST_REQUIRE( a );
	TEST_REQUIRE( allocator_counted_get_peak_count( &alloc ) == 1024 );
	b = allocator_alloc( 1024, allocator_counted_get( &alloc ) );
	TEST_REQUIRE( b );
	TEST_REQUIRE( allocator_counted_get_peak_count( &alloc ) == 2048 );
	allocator_free( a, allocator_counted_get( &alloc ) );
	TEST_REQUIRE( allocator_counted_get_peak_count( &alloc ) == 2048 );
	allocator_free( b, allocator_counted_get( &alloc ) );
	TEST_REQUIRE( allocator_counted_get_peak_count( &alloc ) == 2048 );
}


static void _ensure_allocator_counted_free_copes_with_null_parent_allocator( void )
{
	int data = 0;
	allocator_counted_t alloc;
	allocator_counted_init( &alloc, 0 );
	allocator_free( &data, allocator_counted_get( &alloc ) );
}


static void _ensure_allocator_counted_free_copes_with_null_address( void )
{
	allocator_counted_t alloc;
	allocator_counted_init_default( &alloc );
	allocator_free( 0, allocator_counted_get( &alloc ) );
}


int main( int argc, char * argv[] )
{
	UNUSED( argc );
	UNUSED( argv );

	_ensure_allocator_counted_init_copes_with_null_allocator( );
	_ensure_allocator_counted_init_copes_with_null_parent_allocator( );
	_ensure_allocator_counted_init_sets_correct_parent_allocator( );
	_ensure_allocator_counted_init_sets_allocation_functions( );
	_ensure_allocator_counted_init_default_copes_with_null_allocator( );
	_ensure_allocator_counted_init_default_sets_correct_parent_allocator( );
	_ensure_allocator_counted_init_default_sets_allocation_functions( );
	_ensure_allocator_counted_get_returns_internal_allocator( );
	_ensure_allocator_counted_alloc_returns_null_for_empty_allocation( );
	_ensure_allocator_counted_alloc_returns_null_when_parent_allocator_null( );
	_ensure_allocator_counted_alloc_returns_null_when_parent_allocator_fails( );
	_ensure_allocator_counted_alloc_updates_count_when_allocation_succeeds( );
	_ensure_allocator_counted_alloc_doesnt_update_count_when_allocation_failed( );
	_ensure_allocator_counted_alloc_maintains_correct_peak_count( );
	_ensure_allocator_counted_free_copes_with_null_parent_allocator( );
	_ensure_allocator_counted_free_copes_with_null_address( );
	return 0;
}
