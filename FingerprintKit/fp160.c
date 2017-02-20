//
//  fp160.c
//  FingerprintKit
//
//  Created by Yung-Luen Lan on 23/12/2016.
//  Copyright © 2016 hypo. All rights reserved.
//

#include "fp160.h"
#include "common.h"

bool is_valid_fp160_device(struct libusb_device_descriptor desc)
{
    int valid_device_count = 4;
    struct libusb_device_descriptor valid_device[4] =
    {
        { .idVendor = 0x0bca, .idProduct = 0x8160},
        { .idVendor = 0x0bca, .idProduct = 0x8164},
        { .idVendor = 0x0bca, .idProduct = 0x9160},
        { .idVendor = 0x04f3, .idProduct = 0x0903}
    };
    
    for (int i = 0; i < valid_device_count; i++)
    {
        if (desc.idVendor == valid_device[i].idVendor &&
            desc.idProduct == valid_device[i].idProduct)
        {
            return true;
        }
    }
    return false;
}

struct libusb_device_handle *fp160_device_handle()
{
    return device_handle(is_valid_fp160_device);
}

int fp160_open(struct libusb_device_handle *handle)
{
    int r = libusb_claim_interface(handle, 0);
    if (r < 0) {
        LOG("libusb_claim_interface error %d", r);
        return -3; // FIXME: what is this?
    }
    
    r = libusb_set_interface_alt_setting(handle, 0, 0);
    if (r < 0) {
        LOG("libusb_set_interface_alt_setting error %d", r);
        return -3;
    }
    return 0;
}

void fp160_close(struct libusb_device_handle *handle)
{

}

device_config fp160_read_config(struct libusb_device_handle *handle)
{
    device_config c;
    bzero(&c, sizeof(device_config));
    c.FP_IMAGE_WIDTH = 176;
    c.FP_IMAGE_HEIGHT = 176;
    return c;
}

//Support Micron, g_SST = 3
int fp160_get_image_v0(struct libusb_device_handle *handle, device_config config, uint8_t *image)
{
    uint8_t *lpImage;
    uint8_t *clpImage;
    uint8_t *rawImage;
    int offsetx, offsety;
    
    if (handle == NULL) {
        LOG("fp160 device not open\n");
        return -1;
    }
    
    lpImage = (uint8_t *)calloc(640 * 480, sizeof(uint8_t));
    clpImage = (uint8_t *)calloc(800 * 800, sizeof(uint8_t));
    rawImage = (uint8_t *)calloc(640 * 480, sizeof(uint8_t));
    
    set_vga_start(handle);
    vga_getframe(handle, rawImage, 160 * 160, 1);

    offsetx = (config.FP_IMAGE_WIDTH - 160) / 2;
    offsety = (config.FP_IMAGE_HEIGHT - 160) / 2;
    
    memset(lpImage, 255, config.FP_IMAGE_WIDTH * config.FP_IMAGE_HEIGHT);
    
    for (int i = 0; i < 160; i++) {
        memcpy(lpImage + (i + offsety) * config.FP_IMAGE_WIDTH + offsetx, rawImage + (i * 160), 160);
    }
    
    //gray level enhance
    for (int i = 0; i < config.FP_IMAGE_WIDTH * config.FP_IMAGE_HEIGHT; i++) {
        lpImage[i] = lpImage[i] * 2 - 255;
    }
    
    memcpy(image, lpImage, config.FP_IMAGE_WIDTH * config.FP_IMAGE_HEIGHT);
    
    free(lpImage);
    free(clpImage);
    free(rawImage);
    return 0;
}

int fp160_get_image(struct libusb_device_handle *handle, device_config config, uint8_t *image)
{
    switch (config.g_SST) {
        case 0:
        case 3:
            return fp160_get_image_v0(handle, config, image);
        case 1:
//            return fp160_get_image_v0(image);
        default:
            break;
    }
    return -1;
}
