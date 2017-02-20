//
//  image.h
//  FingerprintKit
//
//  Created by Yung-Luen Lan on 23/12/2016.
//  Copyright © 2016 hypo. All rights reserved.
//

#ifndef gray_h
#define gray_h

#include <stdio.h>

// copy the rect: {origin: (x_str, y_str), size: (x_disp, y_disp)} from src to dst 
int clip(int src_w, int src_h,
         uint8_t *src, uint8_t *dst,
         unsigned short x_str, unsigned short y_str,
         unsigned short x_disp, unsigned short y_disp);
int resize_cubic_v6(uint8_t *InImg, uint8_t *OutImg, int src_w, int src_h, int tgt_w, int tgt_h);


void gray_enhance(uint8_t *g_lpTempBuf, int w, int h);

int rgb_bayer_to_gray(unsigned char *in, unsigned char *out, int width, int height);

void write_bmp(char *filename, uint8_t *red_buf, uint8_t *green_buf, uint8_t *blue_buf, int WIDTH, int HEIGHT);
#endif /* gray_h */
