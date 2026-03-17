#include "stdio.h"

#include "constexpr.hpp"

// simple print function to print contents of generated LUTs
void print_constexpr_LUT(const char * name, const unsigned char * array, int length, int sizeX, int sizeY) {
	printf("array: %s\n", name);
	int y = 0;
	for(int i = 0; i < length; i++) {
		printf("%3u ", array[i]);
		if(!((i + 1) % sizeX)) {
			printf("\n");
			y++;
		}
		if(y == sizeY) {
			printf("\n");
			y = 0;
		}
	}
}

//	EXAMPLE function calls to put in main:
//
//	test whether the LUTs were generated correctly by printing them
//	printf("Generating our %dx%d constexpr LUTs ...\n", LEDNUMX, LEDNUMY);
//	print_constexpr_LUT("distctr"    , distctr.data()    , LEDNUMX * LEDNUMY, LEDNUMX, LEDNUMY);
//	print_constexpr_LUT("invdistctr" , invdistctr.data() , LEDNUMX * LEDNUMY, LEDNUMX, LEDNUMY);
//	print_constexpr_LUT("distcornerr", distcornerr.data(), 4*LEDNUMX*LEDNUMY, LEDNUMX, LEDNUMY);
//	print_constexpr_LUT("distQuadctr", distQuadctr.data(), 4*LEDNUMX*4*LEDNUMY, 4*LEDNUMX, 4*LEDNUMY);
