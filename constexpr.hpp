#ifndef CONSTEXPR_HPP
#define CONSTEXPR_HPP

#include <cstdio>
#include <array>

#include "sluts.h"

//////////////////////////////////////////////////
// SIZE OF ARRAY BLOCKS							//
//////////////////////////////////////////////////
constexpr int NX = 12;
constexpr int NY = 12;

//////////////////////////////////////////////////
// MATH FUNCTIONS								//
//////////////////////////////////////////////////
constexpr int PI = 3.14159265358979323846;

constexpr int
abs(int x)
{
	return (x < 0) ? -x : x;
}

constexpr int
sqr(int x)
{
	return x * x;
}

constexpr int
clamp(int v, int min, int max)
{
	return (v < min) ? min : (v > max) ? max : v;
}

// sqrt via 5 Newton iterations
constexpr double
sqrt(unsigned int x)
{
	// prevent division by zero
	if(x == 0)
	{
		return 0;
	}	
	double g = x;	
	for(int i = 0; i < 5; ++i)
	{
		g = (g + x / g) / 2;
	}
	return g;
}

constexpr double
sin(int angle)
{
	return SIN_LUT[angle];
}
constexpr double
cos(int angle) 
{
	return COS_LUT[angle];
}
constexpr double
tan(int angle)
{
	return TAN_LUT[angle];
}

//////////////////////////////////////////////////
// LUT PATTERN FUNCTIONS						//
//////////////////////////////////////////////////
constexpr int CENTER_X = (NX - 1) / 2;
constexpr int CENTER_Y = (NY - 1) / 2;

constexpr unsigned char 
dist_center(int x, int y)
{
	int dx = x - CENTER_X;
	int dy = y - CENTER_Y;
	unsigned int v = static_cast<unsigned int>(dx*dx + dy*dy);
	return static_cast<unsigned char>(sqrt(v));
}

constexpr unsigned char 
inv_dist_center(int x, int y)
{
	int d = static_cast<int>(dist_center(x, y));
	int v = 7 - d;
	if(v < 0)
	{
		return static_cast<unsigned char>(0);
	}
	return static_cast<unsigned char>(v);
}

constexpr unsigned char 
dist_corner(int x, int y)
{
	unsigned int v = static_cast<unsigned int>(x*x + y*y);
	return static_cast<unsigned char>(sqrt(v));
}

//////////////////////////////////////////////////
// CONSTEXPR LUT GENERATION FUNCTIONS			//
//////////////////////////////////////////////////
constexpr std::array<unsigned char, NX * NY> 
generate_distctr()
{
	std::array<unsigned char, NX * NY> arr {};
	for(int y = 0; y < NY; ++y)
	{
		for(int x = 0; x < NX; ++x)
		{
			arr[y * NX + x] = dist_center(x, y);
		}
	}
	return arr;
}

constexpr std::array<unsigned char, NX * NY> 
generate_invdistctr()
{
	std::array<unsigned char, NX * NY> arr {};
	for(int y = 0; y < NY; ++y)
	{
		for(int x = 0; x < NX; ++x)
		{
			arr[y*NX + x] = inv_dist_center(x, y);
		}
	}
	return arr;
}

constexpr std::array<unsigned char, 4 * NX * NY> 
generate_distcorners()
{
	std::array<unsigned char, 4 * NX * NY> arr {};
	for(int y = 0; y < NY; ++y)
	{
		for(int x = 0; x < NX; ++x)
		{
			int base = y * NX + x;
			arr[0 * NX * NY + base] = dist_corner(           x,            y);
			arr[1 * NX * NY + base] = dist_corner((NX - 1) - x,            y);
			arr[2 * NX * NY + base] = dist_corner((NX - 1) - x, (NY - 1) - y);
			arr[3 * NX * NY + base] = dist_corner(           x, (NY - 1) - y);
		}
	}
   return arr;
}

// simple print function to print contents of generated LUTs
void 
print_array(const char * name, const unsigned char * array, int length)
{
	printf("array: %s\n", name);
	int y = 0;
	for(int i = 0; i < length; i++)
	{
		printf("%2u ", array[i]);
		if(!((i + 1) % NX))
		{
			printf("\n");
			y++;
		}
		if(y == NY)
		{
			printf("\n");
			y = 0;
		}
	}
}

// GENERATE THE LUTS
constexpr std::array<unsigned char,     NX * NY> distctr     = generate_distctr    ();
constexpr std::array<unsigned char,     NX * NY> invdistctr  = generate_invdistctr ();
constexpr std::array<unsigned char, 4 * NX * NY> distcornerr = generate_distcorners();

#endif // CONSTEXPR_HPP