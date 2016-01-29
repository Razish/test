#include <stdio.h>

int main( int argc, char **argv ) {
	signed char sc = -127;
	unsigned char uc = (unsigned char)sc;
	signed int si = (signed int)sc;
	unsigned int ui = (unsigned int)uc;
	printf( "unsigned %u/%u signed %i/%i\n", si, ui, si, ui );
	return 0;
}

