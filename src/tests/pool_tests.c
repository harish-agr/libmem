#include "testing.h"
#include "../mem/pool.h"
#include "../mem/internal/unused.h"

static void _ensure_pool_new_returns_initialised_pool( void )
{
	pool_t * pool;
	allocator_guarded_t alloc;
	allocator_guarded_init_default( &alloc );
	pool = pool_new( 128, 4, allocator_guarded_get( &alloc ) );
	TEST_REQUIRE( pool );
	TEST_REQUIRE( pool->buffer );
	TEST_REQUIRE( allocator_guarded_length( pool->buffer ) >= 128 * 4 );
	TEST_REQUIRE( pool->next );
	TEST_REQUIRE( pool->allocator == allocator_guarded_get( &alloc ) );
	pool_delete( pool );
}

static void _ensure_pool_new_returns_null_when_no_more_memory( void )
{
	pool_t * pool = pool_new( 128, 4, allocator_always_fail( ) );
	TEST_REQUIRE( pool == 0 );
}

static void _ensure_pool_delete_releases_all_allocated_memory( void )
{
	pool_t * pool;
	allocator_counted_t alloc;
	allocator_counted_init_default( &alloc );
	pool = pool_new( 128, 4, allocator_counted_get( &alloc ) );
	pool_delete( pool );
	TEST_REQUIRE( allocator_counted_get_current_count( &alloc ) == 0 );
}

static void _ensure_pool_delete_gracefully_handles_null_pool( void )
{
	pool_delete( 0 );
}

static void _ensure_pool_delete_gracefully_handles_cleaned_up_pool( void )
{
	pool_t * pool;
	allocator_counted_t alloc;
	allocator_counted_init_default( &alloc );
	pool = pool_new( 128, 4, allocator_counted_get( &alloc ) );
	pool_cleanup( pool );
	pool_delete( pool );
	TEST_REQUIRE( allocator_counted_get_current_count( &alloc ) == 0 );
}

static void _ensure_pool_init_gracefully_handles_null_pool( void )
{
	pool_init( 0, 4, 16, allocator_default( ) );
}

static void _ensure_pool_init_with_zero_element_size_constructs_valid_empty_pool( void )
{
	allocator_counted_t alloc;
	pool_t pool;

	allocator_counted_init_default( &alloc );
	pool_init( &pool, 0, 16, allocator_counted_get( &alloc ) );
	TEST_REQUIRE( pool_is_empty( &pool ) );
	pool_cleanup( &pool );
	TEST_REQUIRE( allocator_counted_get_current_count( &alloc ) == 0 );
}

static void _ensure_pool_init_with_zero_element_count_constructs_valid_empty_pool( void )
{
	allocator_counted_t alloc;
	pool_t pool;

	allocator_counted_init_default( &alloc );
	pool_init( &pool, 4, 0, allocator_counted_get( &alloc ) );
	TEST_REQUIRE( pool_is_empty( &pool ) );
	pool_cleanup( &pool );
	TEST_REQUIRE( allocator_counted_get_current_count( &alloc ) == 0 );
}

static void _ensure_pool_init_gracefully_copes_when_out_of_memory( void )
{
	pool_t pool;
	pool_init( &pool, 4, 16, allocator_always_fail( ) );
}

static void _ensure_pool_cleanup_releases_underlying_buffer( void )
{
	allocator_counted_t alloc;
	pool_t pool;

	allocator_counted_init_default( &alloc );
	pool_init( &pool, 4, 16, allocator_counted_get( &alloc ) );
	pool_cleanup( &pool );
	TEST_REQUIRE( allocator_counted_get_current_count( &alloc ) == 0 );
}

static void _ensure_pool_cleanup_gracefully_handles_null_pool( void )
{
	pool_cleanup( 0 );
}

static void _ensure_pool_cleanup_gracefully_handles_cleaned_up_pool( void )
{
	pool_t pool;
	pool_init( &pool, 4, 16, allocator_default( ) );
	pool_cleanup( &pool );
	pool_cleanup( &pool );
}

static void _ensure_pool_take_returns_object_when_not_empty( void )
{
	void * item;
	pool_t pool;
	pool_init( &pool, 4, 16, allocator_default( ) );
	item = pool_take( &pool );
	TEST_REQUIRE( item );
	item = 0;
	pool_cleanup( &pool );
}

static void _ensure_pool_take_returns_null_when_empty( void )
{
	void * item;
	pool_t pool;
	pool_init( &pool, 4, 1, allocator_default( ) );
	pool_take( &pool );
	item = pool_take( &pool );
	TEST_REQUIRE( item == 0 );
	pool_cleanup( &pool );
}

static void _ensure_pool_take_gracefully_handles_null_pool( void )
{
	TEST_REQUIRE( pool_take( 0 ) == 0 );
}

static void _ensure_pool_take_gracefully_handles_cleaned_up_pool( void )
{
	pool_t pool;
	pool_init( &pool, 4, 16, allocator_default( ) );
	pool_cleanup( &pool );
	TEST_REQUIRE( pool_take( &pool ) == 0 );
}

static void _ensure_pool_take_reuses_returned_elements( void )
{
	void * item;
	pool_t pool;
	pool_init( &pool, 4, 1, allocator_default( ) );
	item = pool_take( &pool );
	pool_return( &pool, item );
	TEST_REQUIRE( pool_take( &pool ) == item );
	pool_cleanup( &pool );
}

static void _ensure_pool_return_gracefully_handles_address_already_returned( void )
{
	void * item;
	pool_t pool;
	pool_init( &pool, 4, 1, allocator_default( ) );
	item = pool_take( &pool );
	pool_return( &pool, item );
	pool_return( &pool, item );
	pool_cleanup( &pool );
}

static void _ensure_pool_return_gracefully_handles_cleaned_up_pool( void )
{
	void * item;
	pool_t pool;
	pool_init( &pool, 4, 1, allocator_default( ) );
	item = pool_take( &pool );
	pool_cleanup( &pool );
	pool_return( &pool, item );
}

static void _ensure_pool_return_gracefully_handles_address_not_in_pool( void )
{
	unsigned item_not_in_pool;
	pool_t pool;
	pool_init( &pool, 4, 1, allocator_default( ) );
	pool_return( &pool, &item_not_in_pool );
	TEST_REQUIRE( pool_take( &pool ) != &item_not_in_pool );
	TEST_REQUIRE( pool_take( &pool ) == 0 );
	pool_cleanup( &pool );
}

static void _ensure_pool_is_empty_gracefully_handles_null_pool( void )
{
	TEST_REQUIRE( pool_is_empty( 0 ) != 0 );
}

static void _ensure_pool_is_empty_gracefully_handles_cleaned_up_pool( void )
{
	pool_t pool;
	pool_init( &pool, 4, 1, allocator_default( ) );
	pool_cleanup( &pool );
	TEST_REQUIRE( pool_is_empty( &pool ) != 0 );
}

static void _ensure_pool_is_empty_returns_non_zero_on_empty_legitimate_pool( void )
{
	pool_t pool;
	pool_init( &pool, 4, 1, allocator_default( ) );
	TEST_REQUIRE( pool_is_empty( &pool ) == 0 );
	pool_cleanup( &pool );
}

static void _ensure_pool_is_empty_returns_zero_on_non_empty_legitimate_pool( void )
{
	pool_t pool;
	pool_init( &pool, 4, 1, allocator_default( ) );
	pool_take( &pool );
	TEST_REQUIRE( pool_is_empty( &pool ) != 0 );
	pool_cleanup( &pool );
}

int main( int argc, char * argv[] )
{
	UNUSED( argc );
	UNUSED( argv );

	_ensure_pool_new_returns_initialised_pool( );
	_ensure_pool_new_returns_null_when_no_more_memory( );
	_ensure_pool_delete_releases_all_allocated_memory( );
	_ensure_pool_delete_gracefully_handles_null_pool( );
	_ensure_pool_delete_gracefully_handles_cleaned_up_pool( );
	_ensure_pool_init_gracefully_handles_null_pool( );
	_ensure_pool_init_with_zero_element_size_constructs_valid_empty_pool( );
	_ensure_pool_init_with_zero_element_count_constructs_valid_empty_pool( );
	_ensure_pool_init_gracefully_copes_when_out_of_memory( );
	_ensure_pool_cleanup_releases_underlying_buffer( );
	_ensure_pool_cleanup_gracefully_handles_null_pool( );
	_ensure_pool_cleanup_gracefully_handles_cleaned_up_pool( );
	_ensure_pool_take_returns_object_when_not_empty( );
	_ensure_pool_take_returns_null_when_empty( );
	_ensure_pool_take_gracefully_handles_null_pool( );
	_ensure_pool_take_gracefully_handles_cleaned_up_pool( );
	_ensure_pool_take_reuses_returned_elements( );
	_ensure_pool_return_gracefully_handles_address_already_returned( );
	_ensure_pool_return_gracefully_handles_cleaned_up_pool( );
	_ensure_pool_return_gracefully_handles_address_not_in_pool( );
	_ensure_pool_is_empty_gracefully_handles_null_pool( );
	_ensure_pool_is_empty_gracefully_handles_cleaned_up_pool( );
	_ensure_pool_is_empty_returns_non_zero_on_empty_legitimate_pool( );
	_ensure_pool_is_empty_returns_zero_on_non_empty_legitimate_pool( );
}

