#ifndef CONSTEXPR_HPP
#define CONSTEXPR_HPP

#include <array>
#include <cmath>

#include "simfibre_config.h"
#include "sluts.h"

//////////////////////////////////////////////////
// SIZE OF ARRAY BLOCKS							//
//////////////////////////////////////////////////
constexpr int NX = LEDNUMX;
constexpr int NY = LEDNUMY;

namespace lut {
	typedef std::array<unsigned char,     NX     * NY> array_2D;
	typedef std::array<unsigned char, 2 * NX * 2 * NY> array_3D;
	typedef std::array<unsigned char, 4 * NX * 4 * NY> array_4D;
}

//////////////////////////////////////////////////
// MATH FUNCTIONS								//
//////////////////////////////////////////////////
constexpr int clamp(int v, int min, int max) {
	return (v < min) ? min : (v > max) ? max : v;
}
constexpr double sqr(double x) {
	return x*x;
}

constexpr double sin(int angle) {
	return SIN_LUT[angle];
}
constexpr double cos(int angle) {
	return COS_LUT[angle];
}
constexpr double tan(int angle) {
	return TAN_LUT[angle];
}

//////////////////////////////////////////////////
// LUT PATTERN FUNCTIONS						//
//////////////////////////////////////////////////
constexpr double CENTER_X = (NX - 1.0) / 2.0;
constexpr double CENTER_Y = (NY - 1.0) / 2.0;

constexpr double dist_center(double y, double x, double centerY, double centerX) {
	return std::floor(std::sqrt( sqr(abs(y-centerY)) + sqr(abs(x-centerY)) ));
}

constexpr double dist_corner(double y, double x) {
	return std::sqrt( sqr(abs(y)) + sqr(abs(x)) );
}

//////////////////////////////////////////////////
// CONSTEXPR LUT GENERATION FUNCTIONS			//
//////////////////////////////////////////////////
constexpr lut::array_2D generate_distctr() {
	lut::array_2D arr {};
	double mapMax = dist_center(0, 0, CENTER_Y, CENTER_X);
	for(int y = 0; y < NY; y++) {
		for(int x = 0; x < NX; x++) {
			arr[y*NX + x] = dist_center(y, x, CENTER_Y, CENTER_X) * (255.0/mapMax);
		}
	}
	return arr;
}

constexpr lut::array_4D generate_distqctr() {
	lut::array_4D arr {};
	double centerQ[2] = { 
		(4.0 * NY - 1.0) / 2.0, 
		(4.0 * NX - 1.0) / 2.0 
	};
	double mapMax = dist_center(0, 0, centerQ[0], centerQ[1]);
	for(int y = 0; y < (NY*4); y++) {
		for(int x = 0; x < (NX*4); x++) {
			arr[y*(NX*4) + x] = dist_center(y, x, centerQ[0], centerQ[1]) * (255.0/mapMax);
		}
	}
	return arr;
}

constexpr lut::array_2D generate_invdistctr(lut::array_2D distctr) {
	lut::array_2D arr {};
	for(int y = 0; y < NY; y++) {
		for(int x = 0; x < NX; x++) {
			arr[y*NX + x] = 255.0 - distctr[y*NX + x];
		}
	}
	return arr;
}

constexpr lut::array_3D generate_distcorners() {
	lut::array_3D arr {};
	double mapMax = std::floor(dist_corner(NY,NX));
	for(int y = 0; y < NY; y++) {
		for(int x = 0; x < NX; x++) {
			int offset = y * NX + x;
			arr[0 * NX * NY + offset] = std::ceil(dist_corner(          y,          x)) * 255.0/mapMax;
			arr[1 * NX * NY + offset] = std::ceil(dist_corner(          y, (NX-1) - x)) * 255.0/mapMax;
			arr[2 * NX * NY + offset] = std::ceil(dist_corner( (NY-1) - y, (NX-1) - x)) * 255.0/mapMax;
			arr[3 * NX * NY + offset] = std::ceil(dist_corner( (NY-1) - y,          x)) * 255.0/mapMax;
		}
	}
   return arr;
}

// simple print function to print contents of generated LUTs
void print_constexpr_LUT(const char * name, const unsigned char * array, int length, int sizeX, int sizeY);

// GENERATE THE LUTS
constexpr lut::array_2D distctr     = generate_distctr    ();
constexpr lut::array_2D invdistctr  = generate_invdistctr ( distctr );
constexpr lut::array_3D distcornerr = generate_distcorners();
constexpr lut::array_4D distQuadctr = generate_distqctr   ();

#endif // CONSTEXPR_HPP