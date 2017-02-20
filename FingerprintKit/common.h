//
//  common.h
//  FingerprintKit
//
//  Created by Yung-Luen Lan on 23/12/2016.
//  Copyright © 2016 hypo. All rights reserved.
//

#ifndef common_h
#define common_h

#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include "libusb.h"

#define LOG(...)    do { fprintf(stderr, "%s:%s:%d ", __FILE__, __FUNCTION__, __LINE__); fprintf(stderr, __VA_ARGS__); fprintf(stderr, "\n"); } while (0);

typedef struct {
    uint8_t g_AGC;
    uint8_t g_RCG;
    uint8_t g_BCG;
    uint8_t g_AEC;
    uint8_t g_BRT;
    uint8_t g_CST;
    uint8_t g_Mjr_Ver;
    uint8_t g_Mnr_Ver;
    uint8_t g_Func;
    uint8_t g_SST;
    unsigned short _500DPI_W;
    unsigned short _500DPI_H;
    unsigned short CLIP_START_X;
    unsigned short CLIP_START_Y;
    
    unsigned short ori_500DPI_W;
    unsigned short ori_500DPI_H;
    unsigned short ori_CLIP_START_X;
    unsigned short ori_CLIP_START_Y;
    
    int FP_IMAGE_WIDTH;
    int	FP_IMAGE_HEIGHT;
} device_config;

typedef bool device_predicate_t(struct libusb_device_descriptor desc);

struct libusb_device *first_matching_device(struct libusb_device **devs, device_predicate_t *valid_device);

struct libusb_device_handle *device_handle(device_predicate_t *valid_device);

extern const int EP1_OUT;
extern const int EP1_IN;
extern const int EP2_IN;

int set_vga_start(libusb_device_handle *handle);
int vga_getframe(libusb_device_handle *handle, unsigned char *lpImage, int vga_width, int vga_height);
#endif /* common_h */
