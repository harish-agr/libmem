#include <stdio.h>
#include <string.h>
#include "../mem/buffer.h"
#include "../mem/internal/unused.h"
#include "testing.h"

static void _ensure_buffer_new_returns_null_when_given_null_allocator( )
{
	TEST_REQUIRE( buffer_new( 0 ) == 0 );
}

static void _ensure_buffer_new_returns_null_when_allocation_fails( )
{
	TEST_REQUIRE( buffer_new( allocator_always_fail( ) ) == 0 );
}

static void _ensure_buffer_new_initialises_buffer_with_zero_capacity( )
{
	buffer_t * buffer = buffer_new( allocator_default( ) );
	TEST_REQUIRE( buffer );
	TEST_REQUIRE( buffer_capacity( buffer ) == 0 );
	buffer_delete( buffer );
}

static void _ensure_buffer_new_initialises_buffer_with_zero_data_length( )
{
	buffer_t * buffer = buffer_new( allocator_default( ) );
	TEST_REQUIRE( buffer );
	TEST_REQUIRE( buffer_data_length( buffer ) == 0 );
	buffer_delete( buffer );
}

static void _ensure_buffer_delete_copes_with_null_buffer( )
{
	buffer_delete( 0 );
}

static void _ensure_buffer_delete_copes_with_cleaned_up_buffer( )
{
	buffer_t * buffer;
	allocator_counted_t allocator;
	allocator_counted_init( &allocator, allocator_default( ) );
	buffer = buffer_new( allocator_counted_get( &allocator ) );
	buffer_cleanup( buffer );
	buffer_delete( buffer );
	TEST_REQUIRE( allocator_counted_get_current_count( &allocator ) == 0 );
}

static void _ensure_buffer_delete_releases_all_memory_consumed_by_legitimate_buffer( )
{
	int data = 123;
	buffer_t * buffer;

	allocator_counted_t allocator;
	allocator_counted_init( &allocator, allocator_default( ) );
	buffer = buffer_new( allocator_counted_get( &allocator ) );
	buffer_append( buffer, sizeof( data ), &data );
	buffer_append( buffer, sizeof( data ), &data );
	buffer_append( buffer, sizeof( data ), &data );
	buffer_append( buffer, sizeof( data ), &data );
	buffer_delete( buffer );
	TEST_REQUIRE( allocator_counted_get_current_count( &allocator ) == 0 );
}

static void _ensure_buffer_init_initialises_buffer_with_zero_capacity( )
{
	buffer_t buffer;
	buffer_init( &buffer, allocator_default( ) );
	TEST_REQUIRE( buffer_capacity( &buffer ) == 0 );
	buffer_cleanup( &buffer );
}

static void _ensure_buffer_init_initialises_buffer_with_zero_data_length( )
{
	buffer_t buffer;
	buffer_init( &buffer, allocator_default( ) );
	TEST_REQUIRE( buffer_data_length( &buffer ) == 0 );
	buffer_cleanup( &buffer );
}

static void _ensure_buffer_init_copes_with_null_buffer( )
{
	buffer_init( 0, allocator_default( ) );
}

static void _ensure_buffer_init_copes_with_null_allocator( )
{
	buffer_t buffer;
	buffer_init( &buffer, 0 );
}

static void _ensure_buffer_init_copes_with_failed_allocation( )
{
	buffer_t buffer;
	buffer_init( &buffer, allocator_always_fail( ) );
}

static void _ensure_buffer_cleanup_copes_with_null_buffer( )
{
	buffer_cleanup( 0 );
}

static void _ensure_buffer_cleanup_copes_with_cleaned_up_buffer( )
{
	int data = 123;
	buffer_t buffer;
	buffer_init( &buffer, allocator_default( ) );
	buffer_append( &buffer, sizeof( data ), &data );
	buffer_cleanup( &buffer );
	buffer_cleanup( &buffer );
}

static void _ensure_buffer_cleanup_releases_all_memory_consumed_by_legitimate_buffer( )
{
	allocator_counted_t allocator;
	buffer_t buffer;
	int data = 123;

	allocator_counted_init( &allocator, allocator_default( ) );
	buffer_init( &buffer, allocator_counted_get( &allocator ) );
	buffer_append( &buffer, sizeof( data ), &data );
	buffer_append( &buffer, sizeof( data ), &data );
	buffer_append( &buffer, sizeof( data ), &data );
	buffer_append( &buffer, sizeof( data ), &data );
	buffer_cleanup( &buffer );
	TEST_REQUIRE( allocator_counted_get_current_count( &allocator ) == 0 );
}

static void _ensure_buffer_grow_increases_capacity_by_given_non_zero_amount( )
{
	allocator_counted_t allocator;
	buffer_t buffer;

	allocator_counted_init( &allocator, allocator_default( ) );
	buffer_init( &buffer, allocator_counted_get( &allocator ) );
	buffer_grow( &buffer, 256 );
	TEST_REQUIRE( allocator_counted_get_current_count( &allocator ) == 256 );
	buffer_cleanup( &buffer );
}

static void _ensure_buffer_grow_does_not_reallocate_when_given_zero_amount( )
{
	allocator_counted_t allocator;
	buffer_t buffer;

	allocator_counted_init( &allocator, allocator_default( ) );
	buffer_init( &buffer, allocator_counted_get( &allocator ) );
	buffer_grow( &buffer, 256 );
	buffer_grow( &buffer, 0 );
	TEST_REQUIRE( allocator_counted_get_current_count( &allocator ) == 256 );
	TEST_REQUIRE( allocator_counted_get_peak_count( &allocator ) == 256 );
	buffer_cleanup( &buffer );
}

static void _ensure_buffer_grow_returns_zero_when_given_length_of_zero_on_empty_buffer( )
{
	buffer_t buffer;
	buffer_init( &buffer, allocator_default( ) );
	TEST_REQUIRE( buffer_grow( &buffer, 0 ) == 0 );
	buffer_cleanup( &buffer );
}

static void _ensure_buffer_grow_returns_current_capacity_when_given_length_of_zero_on_nonempty_buffer( )
{
	buffer_t buffer;
	buffer_init( &buffer, allocator_default( ) );
	buffer_grow( &buffer, 128 );
	TEST_REQUIRE( buffer_grow( &buffer, 0 ) == 128 );
	buffer_cleanup( &buffer );
}

static void _ensure_buffer_grow_returns_correct_total_capacity_when_growing_empty_buffer( )
{
	buffer_t buffer;
	buffer_init( &buffer, allocator_default( ) );
	TEST_REQUIRE( buffer_grow( &buffer, 256 ) == 256 );
	buffer_cleanup( &buffer );
}

static void _ensure_buffer_grow_returns_correct_total_capacity_when_growing_non_empty_buffer( )
{
	buffer_t buffer;
	buffer_init( &buffer, allocator_default( ) );
	buffer_grow( &buffer, 256 );
	TEST_REQUIRE( buffer_grow( &buffer, 128 ) == 384 );
	buffer_cleanup( &buffer );
}

static void _ensure_buffer_grow_copes_with_null_buffer( )
{
	TEST_REQUIRE( buffer_grow( 0, 256 ) == 0 );
}

static void _ensure_buffer_grow_copes_with_cleaned_up_buffer( )
{
	allocator_counted_t allocator;
	buffer_t buffer;

	allocator_counted_init( &allocator, allocator_default( ) );
	buffer_init( &buffer, allocator_counted_get( &allocator ) );
	buffer_grow( &buffer, 128 );
	buffer_cleanup( &buffer );
	TEST_REQUIRE( buffer_grow( &buffer, 128 ) == 128 );
	buffer_cleanup( &buffer );
	TEST_REQUIRE( allocator_counted_get_current_count( &allocator ) == 0 );
}

static void _ensure_buffer_data_length_returns_exact_number_of_bytes_written_to_buffer( )
{
	int data = 123;
	buffer_t buffer;
	buffer_init( &buffer, allocator_default( ) );
	buffer_append( &buffer, sizeof( data ), &data );
	buffer_append( &buffer, sizeof( data ), &data );
	buffer_append( &buffer, sizeof( data ), &data );
	TEST_REQUIRE( buffer_data_length( &buffer ) == sizeof( data ) * 3 );
	buffer_cleanup( &buffer );
}

static void _ensure_buffer_data_length_copes_with_null_buffer( )
{
	TEST_REQUIRE( buffer_data_length( 0 ) == 0 );
}

static void _ensure_buffer_data_length_copes_with_cleaned_up_buffer( )
{
	buffer_t buffer;
	buffer_init( &buffer, allocator_default( ) );
	buffer_grow( &buffer, 4 );
	buffer_cleanup( &buffer );
	TEST_REQUIRE( buffer_data_length( &buffer ) == 0 );
}

static void _ensure_buffer_data_pointer_returns_null_for_empty_buffer( )
{
	buffer_t buffer;
	buffer_init( &buffer, allocator_default( ) );
	TEST_REQUIRE( buffer_data_pointer( &buffer ) == 0 );
	buffer_cleanup( &buffer );
}

static void _ensure_buffer_data_pointer_copes_with_null_buffer( )
{
	TEST_REQUIRE( buffer_data_pointer( 0 ) == 0 );
}

static void _ensure_buffer_data_pointer_copes_with_cleaned_up_buffer( )
{
	buffer_t buffer;
	buffer_init( &buffer, allocator_default( ) );
	buffer_grow( &buffer, 4 );
	buffer_cleanup( &buffer );
	TEST_REQUIRE( buffer_data_pointer( &buffer ) == 0 );
}

static void _ensure_buffer_rewind_reuses_existing_buffer_capacity( )
{
	int data1 = 123, data2 = 456;
	allocator_counted_t allocator;
	buffer_t buffer;

	allocator_counted_init( &allocator, allocator_default( ) );
	buffer_init( &buffer, allocator_counted_get( &allocator ) );
	buffer_append( &buffer, sizeof( data1 ), &data1 );
	TEST_REQUIRE( buffer_capacity( &buffer ) == sizeof( data1 ) );
	TEST_REQUIRE( allocator_counted_get_current_count( &allocator ) == sizeof( data1 ) );
	TEST_REQUIRE( allocator_counted_get_peak_count( &allocator ) == sizeof( data1 ) );
	buffer_rewind( &buffer );
	TEST_REQUIRE( buffer_capacity( &buffer ) == sizeof( data1 ) );
	TEST_REQUIRE( allocator_counted_get_current_count( &allocator ) == sizeof( data1 ) );
	TEST_REQUIRE( allocator_counted_get_peak_count( &allocator ) == sizeof( data1 ) );
	buffer_append( &buffer, sizeof( data2 ), &data2 );
	TEST_REQUIRE( buffer_capacity( &buffer ) == sizeof( data1 ) );
	TEST_REQUIRE( allocator_counted_get_current_count( &allocator ) == sizeof( data1 ) );
	TEST_REQUIRE( allocator_counted_get_peak_count( &allocator ) == sizeof( data1 ) );
	buffer_cleanup( &buffer );
}

static void _ensure_buffer_rewind_copes_with_empty_buffer( )
{
	buffer_t buffer;
	buffer_init( &buffer, allocator_default( ) );
	buffer_rewind( &buffer );
	buffer_cleanup( &buffer );
}

static void _ensure_buffer_rewind_copes_with_null_buffer( )
{
	buffer_rewind( 0 );
}

static void _ensure_buffer_rewind_copes_with_cleaned_up_buffer( )
{
	int data = 123;
	buffer_t buffer;
	buffer_init( &buffer, allocator_default( ) );
	buffer_append( &buffer, sizeof( data ), &data );
	buffer_cleanup( &buffer );
	buffer_rewind( &buffer );
}

static void _ensure_buffer_append_copies_given_data_into_buffer( )
{
	int data1 = 123, data2 = 456;
	buffer_t buffer;
	buffer_init( &buffer, allocator_default( ) );
	buffer_append( &buffer, sizeof( data1 ), &data1 );
	buffer_append( &buffer, sizeof( data2 ), &data2 );
	TEST_REQUIRE( ( ( int * ) buffer_data_pointer( &buffer ) )[0] == 123 );
	TEST_REQUIRE( ( ( int * ) buffer_data_pointer( &buffer ) )[1] == 456 );
	buffer_cleanup( &buffer );
}

static void _ensure_buffer_append_copes_with_null_buffer( )
{
	int data = 123;
	TEST_REQUIRE( buffer_append( 0, sizeof( data ), &data ) == 0 );
}

static void _ensure_buffer_append_copes_with_cleaned_up_buffer( )
{
	int data = 123;
	allocator_counted_t allocator;
	buffer_t buffer;

	allocator_counted_init( &allocator, allocator_default( ) );
	buffer_init( &buffer, allocator_counted_get( &allocator ) );
	TEST_REQUIRE( buffer_append( &buffer, sizeof( data ), &data ) == sizeof( data ) );
	buffer_cleanup( &buffer );
	TEST_REQUIRE( buffer_append( &buffer, sizeof( data ), &data ) == sizeof( data ) );
	buffer_cleanup( &buffer );
	TEST_REQUIRE( allocator_counted_get_current_count( &allocator ) == 0 );
}

static void _ensure_buffer_append_does_not_mutate_buffer_when_given_zero_data_length( )
{
	int data = 123;
	buffer_t buffer, snapshot;
	buffer_init( &buffer, allocator_default( ) );
	buffer_append( &buffer, sizeof( int ), &data );
	snapshot = buffer;
	TEST_REQUIRE( buffer_append( &buffer, 0, &data ) == 0 );
	TEST_REQUIRE( memcmp( &buffer, &snapshot, sizeof( buffer ) ) == 0 );
	buffer_cleanup( &buffer );
}

static void _ensure_buffer_append_does_not_mutate_buffer_when_given_null_data_pointer( )
{
	int data = 123;
	buffer_t buffer, snapshot;
	buffer_init( &buffer, allocator_default( ) );
	buffer_append( &buffer, sizeof( int ), &data );
	snapshot = buffer;
	TEST_REQUIRE( buffer_append( &buffer, sizeof( data ), 0 ) == 0 );
	TEST_REQUIRE( memcmp( &buffer, &snapshot, sizeof( buffer ) ) == 0 );
	buffer_cleanup( &buffer );
}

static void _ensure_buffer_append_returns_number_of_bytes_appended_to_buffer( )
{
	int data[] = { 1, 2, 3, 4, 5 };
	buffer_t buffer;
	buffer_init( &buffer, allocator_default( ) );
	TEST_REQUIRE( buffer_append( &buffer, sizeof( data ), data ) == sizeof( data ) );
	buffer_cleanup( &buffer );
}

static void _ensure_buffer_reserve_copes_with_null_buffer( )
{
	TEST_REQUIRE( buffer_reserve( 0, 1024 ) == 0 );
}

static void _ensure_buffer_reserve_copes_with_cleaned_up_buffer( )
{
	buffer_t buffer;
	buffer_init( &buffer, allocator_default( ) );
	buffer_cleanup( &buffer );
	TEST_REQUIRE( buffer_reserve( &buffer, 1024 ) );
	buffer_cleanup( &buffer );
}

static void _ensure_buffer_reserve_does_not_mutate_buffer_when_given_zero_data_length( )
{
	buffer_t buffer, snapshot;
	buffer_init( &buffer, allocator_default( ) );
	buffer_reserve( &buffer, sizeof( int ) );
	snapshot = buffer;
	TEST_REQUIRE( buffer_reserve( &buffer, 0 ) == 0 );
	TEST_REQUIRE( memcmp( &buffer, &snapshot, sizeof( buffer ) ) == 0 );
	buffer_cleanup( &buffer );
}

static void _ensure_buffer_reserve_returns_non_null_pointer_on_success( )
{
	buffer_t buffer;
	buffer_init( &buffer, allocator_default( ) );
	TEST_REQUIRE( buffer_reserve( &buffer, 1024 ) );
	buffer_cleanup( &buffer );
}

int main( int argc, char * argv[] )
{
	UNUSED( argc );
	UNUSED( argv );

	_ensure_buffer_new_returns_null_when_given_null_allocator( );
	_ensure_buffer_new_returns_null_when_allocation_fails( );
	_ensure_buffer_new_initialises_buffer_with_zero_capacity( );
	_ensure_buffer_new_initialises_buffer_with_zero_data_length( );
	_ensure_buffer_delete_copes_with_null_buffer( );
	_ensure_buffer_delete_copes_with_cleaned_up_buffer( );
	_ensure_buffer_delete_releases_all_memory_consumed_by_legitimate_buffer( );
	_ensure_buffer_init_initialises_buffer_with_zero_capacity( );
	_ensure_buffer_init_initialises_buffer_with_zero_data_length( );
	_ensure_buffer_init_copes_with_null_buffer( );
	_ensure_buffer_init_copes_with_null_allocator( );
	_ensure_buffer_init_copes_with_failed_allocation( );
	_ensure_buffer_cleanup_copes_with_null_buffer( );
	_ensure_buffer_cleanup_copes_with_cleaned_up_buffer( );
	_ensure_buffer_cleanup_releases_all_memory_consumed_by_legitimate_buffer( );
	_ensure_buffer_grow_increases_capacity_by_given_non_zero_amount( );
	_ensure_buffer_grow_does_not_reallocate_when_given_zero_amount( );
	_ensure_buffer_grow_returns_zero_when_given_length_of_zero_on_empty_buffer( );
	_ensure_buffer_grow_returns_current_capacity_when_given_length_of_zero_on_nonempty_buffer( );
	_ensure_buffer_grow_returns_correct_total_capacity_when_growing_empty_buffer( );
	_ensure_buffer_grow_returns_correct_total_capacity_when_growing_non_empty_buffer( );
	_ensure_buffer_grow_copes_with_null_buffer( );
	_ensure_buffer_grow_copes_with_cleaned_up_buffer( );
	_ensure_buffer_data_length_returns_exact_number_of_bytes_written_to_buffer( );
	_ensure_buffer_data_length_copes_with_null_buffer( );
	_ensure_buffer_data_length_copes_with_cleaned_up_buffer( );
	_ensure_buffer_data_pointer_returns_null_for_empty_buffer( );
	_ensure_buffer_data_pointer_copes_with_null_buffer( );
	_ensure_buffer_data_pointer_copes_with_cleaned_up_buffer( );
	_ensure_buffer_rewind_reuses_existing_buffer_capacity( );
	_ensure_buffer_rewind_copes_with_empty_buffer( );
	_ensure_buffer_rewind_copes_with_null_buffer( );
	_ensure_buffer_rewind_copes_with_cleaned_up_buffer( );
	_ensure_buffer_append_copies_given_data_into_buffer( );
	_ensure_buffer_append_copes_with_null_buffer( );
	_ensure_buffer_append_copes_with_cleaned_up_buffer( );
	_ensure_buffer_append_does_not_mutate_buffer_when_given_zero_data_length( );
	_ensure_buffer_append_does_not_mutate_buffer_when_given_null_data_pointer( );
	_ensure_buffer_append_returns_number_of_bytes_appended_to_buffer( );
	_ensure_buffer_reserve_copes_with_null_buffer( );
	_ensure_buffer_reserve_copes_with_cleaned_up_buffer( );
	_ensure_buffer_reserve_does_not_mutate_buffer_when_given_zero_data_length( );
	_ensure_buffer_reserve_returns_non_null_pointer_on_success( );
	return 0;
}
