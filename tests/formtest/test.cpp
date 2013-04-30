#include <dlfcn.h>
#include <stdio.h>

int main( void )
{
	void *lib = dlopen( "libformtest.so", 1 );
	if( !lib ) {
		fprintf( stderr, "Load error: %s\n", dlerror( ) );
		return 1;
	}
	dlclose( lib );

	return 0;
}
