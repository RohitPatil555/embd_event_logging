#include <compiler_req_apis.h>

void *memset( void *s, int c, size_t n ) {
	uint8_t *p	  = (uint8_t *)( s );
	uint8_t value = (uint8_t)( c );

	while ( n-- ) {
		*p++ = value;
	}
	return s;
}
