cmake_minimum_required( VERSION 3.0 )
project( "libmem" )

set( LIBMEM_VERSION_MAJOR 1 )
set( LIBMEM_VERSION_MINOR 0 )
set( LIBMEM_VERSION_PATCH 0 )
set( LIBMEM_VERSION ${LIBMEM_VERSION_MAJOR}.${LIBMEM_VERSION_MINOR}.${LIBMEM_VERSION_PATCH} )
set( LIBMEM_ABI_VERSION ${LIBMEM_VERSION_MAJOR} )

set( VALGRIND_ENABLE True CACHE BOOL "Enable valgrind tests" )
set( VALGRIND_ARGS "--leak-check=full" "--error-exitcode=1" CACHE STRING "Valgrind arguments" )
find_program( VALGRIND valgrind DOC "Valgrind location (optional)" )

set( STRICT True CACHE BOOL "Enable strict mode (on by default)" )
if( STRICT )
	set( CMAKE_C_FLAGS "-std=c90 -Wall -Wextra -Werror -Wmissing-prototypes -Wstrict-prototypes -Wold-style-definition -pedantic-errors" )
endif( )

enable_testing( )
add_subdirectory( src )
