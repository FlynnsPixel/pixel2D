#ifndef PXL_H
#define PXL_H

#include "PXL_Batch.h"
#include "PXL_ShaderUtils.h"

//screen info
extern int PXL_screen_width;
extern int PXL_screen_height;
extern int PXL_center_screen_x;
extern int PXL_center_screen_y;

/**
\*brief: initialises PXL
\*param [screen_width]: the window width of your program
\*param [screen_height]: the window height of your program
**/
extern void PXL_init(int screen_width, int screen_height);

#endif