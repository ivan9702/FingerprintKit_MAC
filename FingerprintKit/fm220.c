//
//  fm220.c
//  FingerprintKit
//
//  Created by Yung-Luen Lan on 22/12/2016.
//  Copyright © 2016 hypo. All rights reserved.
//

#include "fm220.h"
#include "image.h"

bool is_valid_fm220_device(struct libusb_device_descriptor desc)
{
    int valid_device_count = 2;
    struct libusb_device_descriptor valid_device[2] =
    {
        { .idVendor = 0x0bca, .idProduct = 0x8225},
        { .idVendor = 0x0bca, .idProduct = 0x8220}
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

bool is_fm220_connected()
{
    struct libusb_device **devs;
    
    if (libusb_get_device_list(NULL, &devs) < 0) {
        LOG("cannot list device");
        return NULL;
    }
    
    struct libusb_device *found = first_matching_device(devs, is_valid_fm220_device);
    
    libusb_free_device_list(devs, 1);
    return (found != NULL);

}

struct libusb_device_handle *fm220_device_handle()
{
    return device_handle(is_valid_fm220_device);
}

int fm220_open(struct libusb_device_handle *handle)
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

void fm220_close(struct libusb_device_handle *handle)
{
    if (handle != NULL) {
        led_off(handle);
        libusb_release_interface(handle, 0);
        libusb_close(handle);
    }
}

device_config fm220_read_config(struct libusb_device_handle *handle)
{
    unsigned char buf[64];
    
    // read 48 bytes from EEPROM to buf
    int len = 48;
    int r = eeprom_read(handle, 0, len, buf);

    if (r < 0) {
        LOG("error: %d", r);
        libusb_reset_device(handle);
        r = libusb_claim_interface(handle, 0);
        r = libusb_set_interface_alt_setting(handle, 0, 0);
        eeprom_read(handle, 0, len, buf);
    }
    
    if (strncmp("FM220", (char *)buf, 5) == 0) {
        device_config c;
        c.g_Mjr_Ver = buf[6];
        c.g_Mnr_Ver = buf[7];
        c.ori_500DPI_W = buf[8] * 256 + buf[9];
        c.ori_500DPI_H = buf[10] * 256 + buf[11];
        c.ori_CLIP_START_X = buf[12] * 256 + buf[13];
        c.ori_CLIP_START_Y = buf[14] * 256 + buf[15];
        c.g_AGC = buf[16];
        c.g_RCG = buf[17];
        c.g_BCG = buf[18];
        c.g_AEC = buf[19];
        c.g_BRT = buf[20];
        c.g_CST = buf[21];
        c.g_SST = buf[22];
        
        c.g_Func = buf[42];
        
        //version check
        for (int r = 32; r < 36; r++) {
            if (buf[r] > 50) {
                c.g_Func=0;
            }
        }
        
        // derive image size
        c.FP_IMAGE_WIDTH = 264;
        c.FP_IMAGE_HEIGHT = 324;
        c._500DPI_H = c.ori_500DPI_H * c.FP_IMAGE_HEIGHT / 256;
        c._500DPI_W = c.ori_500DPI_W * c.FP_IMAGE_WIDTH / 256;
        int clip_diff_h = ((c.FP_IMAGE_HEIGHT - 256) / 2 * c._500DPI_H) / c.FP_IMAGE_HEIGHT;
        c.CLIP_START_Y = (c.ori_CLIP_START_Y > clip_diff_h) ? c.ori_CLIP_START_Y - clip_diff_h : 0;
        int clip_diff_w = ((c.FP_IMAGE_WIDTH - 256) / 2 * c._500DPI_W) / c.FP_IMAGE_WIDTH;
        c.CLIP_START_X = (c.ori_CLIP_START_X > clip_diff_w) ? c.ori_CLIP_START_X - clip_diff_w : 0;
        if ((c.CLIP_START_Y + c._500DPI_H) > 480)
            c.CLIP_START_Y = 0;	//holing modified 20120731 new fm220 optical 480-_500DPI_H;
        if ((c.CLIP_START_X + c._500DPI_W) > 640)
            c.CLIP_START_X = 640 - c._500DPI_W;
        return c;
    } else {
        LOG("Not implemented yet");
        assert(false);
    }
}

void fm220_sensor_config_set(struct libusb_device_handle *handle, device_config *config)
{
    if (config->g_SST == 0)
    {
        //sensor register setting
        reg_write(handle, 0x0, config->g_AGC);	//AGC
        reg_write(handle, 0x1, config->g_RCG);	//RCG
        reg_write(handle, 0x2, config->g_BCG);	//BCG
        reg_write(handle, 0x5, config->g_CST);	//CNT
        reg_write(handle, 0x6, config->g_BRT);	//BRT
        reg_write(handle, 0x10, config->g_AEC);	//AEC
        reg_write(handle, 0x17, 28);     //HREFST
        reg_write(handle, 0x18, 188);       //HREFST
    }
    else if (config->g_SST == 1)
    {
        config->g_AEC = 140;
        reg_write_cmd(handle, 0x40, 0x05, config->g_AEC);
        reg_write_cmd(handle, 0x40, 0x0E, config->g_AGC);
        reg_write_cmd(handle, 0x40, 0x09, 6);	//b	6
        reg_write_cmd(handle, 0x40, 0x0A, 1);	//g1	1
        reg_write_cmd(handle, 0x40, 0x0B, 1);	//g2	1
        reg_write_cmd(handle, 0x40, 0x0C, 4);	//r	4
    }
    else if (config->g_SST == 3)
    {
        reg_write_cmd(handle, 0x5D, 0x35, 0);	//actually, it's the reg 0x35's first byte
        reg_write_cmd(handle, 0x5D, 0x80, config->g_AGC);	//actually, it's the reg 0x35's second byte
        int tmp = config->g_AEC * 2;
        reg_write_cmd(handle, 0x5D, 0x09, (tmp/256));	//actually, it's the reg 0x35's first byte
        reg_write_cmd(handle, 0x5D, 0x80, (uint8_t)(tmp & 0xff));	//actually, it's the reg 0x35's second byte
    }

}

//Support Micron, g_SST = 3
int fm220_get_image_v0(struct libusb_device_handle *handle, device_config config, unsigned char *image)
{
    unsigned char *lpImage;
    unsigned char *clpImage;
    unsigned char *rawImage;
    
    uint8_t *p1, *p2;
    
    if (handle == NULL) {
        LOG("fm220 device not open\n");
        return -1;
    }
    
    lpImage  = (unsigned char*)calloc(640 * 480, sizeof(unsigned char));
    clpImage = (unsigned char*)calloc(800 * 800, sizeof(unsigned char));	//holing modified 20120731 for new optical fm220
    rawImage = (unsigned char*)calloc(640 * 480, sizeof(unsigned char));
    
    //Billy suggest for clear ep2 buffer
    clear_ep2_buffer(handle);
    //End Billy suggest for clear ep2 buffer
    
    int rtn = led_on(handle);
#ifdef REPORT_DEVICE_ERROR
    if (rtn != 0) {
        result = -2;
        goto image_v0_cleanup;
    }
#endif
    usleep(45000);
    rtn = set_vga_start(handle);
#ifdef REPORT_DEVICE_ERROR
    if (rtn != 0) {
        result = -2;
        goto image_v0_cleanup;
    }
#endif
    rtn = fm220_vga_getframe(handle, rawImage);
    
    rtn = led_off(handle);
    
/*  FIXME:
    check_black_pix is buggy. It repeatedly check (rawImage[0] < 20) 
    for 640 * 480 times. cnt is at most 1. it always return zero.
 */
//    if (check_black_pix(rawImage, 640, 480, 640*240) == -1) {
//        result = -3;
//        goto image_v0_cleanup;
//    }
    
#ifdef REPORT_DEVICE_ERROR
    if (rtn != 0) {
        result = -2;
        goto image_v0_cleanup;
    }
#endif
    
    if (config.g_SST == 3) {
        rgb_bayer_to_gray(rawImage, lpImage, 640, 480);
    } else {
        memcpy(lpImage, rawImage, 640 * 480);
    }
    
    clip(640, 480, lpImage, clpImage, config.CLIP_START_X, config.CLIP_START_Y, config._500DPI_W, config._500DPI_H);

    //resize
    resize_cubic_v6(clpImage, lpImage, config._500DPI_W, config._500DPI_H,config.FP_IMAGE_WIDTH, config.FP_IMAGE_HEIGHT);
    
    //gray enhance
    gray_enhance(lpImage, config.FP_IMAGE_WIDTH, config.FP_IMAGE_HEIGHT);
    
    // copy memory and inverse image
    p1 = lpImage + config.FP_IMAGE_WIDTH * (config.FP_IMAGE_HEIGHT - 1);
    p2 = image;
    for (int i = 0; i < config.FP_IMAGE_HEIGHT; i++, p1 -= config.FP_IMAGE_WIDTH, p2 += config.FP_IMAGE_WIDTH)
        memcpy(p2, p1, config.FP_IMAGE_WIDTH);
 
image_v0_cleanup:
    free(lpImage);
    free(clpImage);
    free(rawImage);
    return 0;
}

int fm220_get_image_v1(struct libusb_device_handle *handle, device_config config, unsigned char *image)
{
    LOG("Not implement yet");
    return 0;
}

int fm220_get_image(struct libusb_device_handle *handle, device_config config, unsigned char *image)
{
    switch (config.g_SST) {
        case 0:
        case 3:
            return fm220_get_image_v0(handle, config, image);
        case 1:
            return fm220_get_image_v1(handle, config, image);
        default:
            return -1;
    }
}

#pragma mark - USB Command

const int EEPROM_START_ADDRESS = 0x1C00;        //7k

int reg_read(libusb_device_handle *handle, unsigned char addr, unsigned char *value)
{
    unsigned char data[0x10] = {0x21, addr};
    int actual_length;
    
    int r = libusb_bulk_transfer(handle, EP1_OUT, data, 2, &actual_length, 500);
    if (r < 0) {
        LOG("reg_read send cmd error %d\n", r);
        return r;
    }
    
    
    r = libusb_bulk_transfer(handle, EP1_IN, data, 1, &actual_length, 500);
    if (r < 0) {
        LOG("reg_read send cmd error %d\n", r);
        return r;
    }
    
    *value = data[0];
    return 0;
}

int reg_write_cmd(libusb_device_handle *handle, unsigned char cmd, unsigned char addr, unsigned char value)
{
    unsigned char data[0x10] = {cmd, addr, value};
    int actual_length;
    int r = libusb_bulk_transfer(handle, EP1_OUT, data, 3, &actual_length, 500);
    if (r < 0) {
        LOG("send cmd error %d\n", r);
        return r;
    }
    return 0;
}

int reg_write(libusb_device_handle *handle, unsigned char addr, unsigned char value)
{
    unsigned char data[0x10] = {0x21, addr, value};
    int actual_length;
    
    int r = libusb_bulk_transfer(handle, EP1_OUT, data, 3, &actual_length, 500);
    if (r < 0) {
        LOG("reg_write send cmd error %d\n", r);
        return r;
    }
    return 0;
}

int eeprom_read(struct libusb_device_handle *handle, unsigned char offset, int len, unsigned char *buf)
{
    unsigned char data[0x10];
    
    // reading bytes one by one
    for (int index = 0; index < len; index++) {
        usleep(10000);	//10ms
        data[0] = 0x51;
        data[1] = (EEPROM_START_ADDRESS + offset + index) >> 8;
        data[2] = (EEPROM_START_ADDRESS + offset + index) & 0xff;
        
        int actual_length = 0;
        int r = libusb_bulk_transfer(handle, EP1_OUT, data, 3, &actual_length, 500);
        
        if (r < 0) {
            LOG("send cmd1 error %d index=%d\n", r, index);
            return r;
        }
        
        usleep(10000);	//10ms
        r = libusb_bulk_transfer(handle, EP1_IN, data, 1,&actual_length, 1000);
        
        if (r < 0) {
            LOG("send cmd2 error %d index=%d\n", r, index);
            return r;
        }
        memcpy(buf + index, data, 1);
    }
    return 0;
}

int eeprom_write_cmd(struct libusb_device_handle *handle, unsigned char cmd, unsigned char offset, int len, unsigned char *buf)
{
    unsigned char data[0x10];
    
    for (int index = 0; index < len; index++) {
        usleep(10000);	//10ms
        data[0] = cmd;
        data[1] = (EEPROM_START_ADDRESS + offset + index) >> 8;
        data[2] = (EEPROM_START_ADDRESS + offset + index) & 0xff;
        memcpy(data + 3, buf + index, 1);
        
        int actual_length = 0;
        int r = libusb_bulk_transfer(handle, EP1_OUT, data, 4,&actual_length, 500);
        
        if (r < 0) {
            LOG("send cmd1 error %d index=%d\n", r, index);
            return r;
        }
    }
    return 0;
}

int eeprom_write(struct libusb_device_handle *handle, unsigned char offset, int len, unsigned char *buf)
{
    int r;
    unsigned char data[32];
    r = eeprom_write_cmd(handle, 0x51, offset, len, buf);
    r = eeprom_read(handle, offset, len, data);
    r = strncmp((const char *)buf, (const char *)data, len);

    LOG("EEPROM_write 0x51 cmp r %d %s %s\n", r,buf,data);
    
    if (r == 0) return 0;
    
    r = eeprom_write_cmd(handle, 0x55, offset, len, buf);
    r = eeprom_read(handle, offset, len, data);
    r = strncmp((const char *)buf, (const char *)data, len);
    LOG("EEPROM_write 0x55 cmp r %d %s %s\n", r, buf, data);
    return (r == 0) ? 0 : -1;
}

int led_on(libusb_device_handle *handle)
{
    unsigned char data[0x10] = {0x00, 0x07, 0x07};
    int actual_length;
    int r = libusb_bulk_transfer(handle, EP1_OUT, data, 3, &actual_length, 500);
    if (r < 0) {
        LOG("led_on error %d", r);
        return r;
    }
    return 0;
}


int led_off(libusb_device_handle *handle)
{
    unsigned char data[0x10] = {0x00, 0x07, 0x00};
    int actual_length;
    int r = libusb_bulk_transfer(handle, EP1_OUT, data, 3, &actual_length, 500);
    if (r < 0) {
        LOG("led_off error %d", r);
        return r;
    }
    return 0;
}

int fm220_vga_getframe(libusb_device_handle *handle, unsigned char *lpImage)
{
    return vga_getframe(handle, lpImage, 640 * 480, 1);
}

int clear_ep2_buffer(struct libusb_device_handle *handle)
{
    int actual_length;
    unsigned char buffer[512];

    int r = 0;
    while (r == 0) {
        r = libusb_bulk_transfer(handle, EP2_IN, buffer, 512, &actual_length, 50);
    }
    return r;
}
