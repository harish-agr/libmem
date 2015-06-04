#ifndef __MEM_TESTING_H
#define __MEM_TESTING_H

#include <stdio.h>

#define TEST_REQUIRE( exp ) if ( !( exp ) ) \
{ \
	printf( "%s:%d - test '%s' failed\n", __FILE__, __LINE__, #exp ); \
	exit( 1 ); \
}

#endif /* __MEM_TESTING_H */
