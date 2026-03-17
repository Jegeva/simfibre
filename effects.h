#ifndef __SIMFIBRE_EFFECTS_H
#define __SIMFIBRE_EFFECTS_H

#include "simfibre_config.h"
#include "stdint.h"
#include "fast_hsv2rgb.h"

typedef struct {
  uint8_t r;
  uint8_t g;
  uint8_t b;
} RGBcolor_t;

void getRGB(unsigned int x,unsigned int y,RGBcolor_t * r);
void incrementt();
void flipAltLines();
void tableupdate();
void applyglobalsatval();

#endif