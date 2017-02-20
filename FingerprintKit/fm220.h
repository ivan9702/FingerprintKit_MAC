//
//  fm220.h
//  FingerprintKit
//
//  Created by Yung-Luen Lan on 22/12/2016.
//  Copyright © 2016 hypo. All rights reserved.
//

#ifndef fm220_h
#define fm220_h

#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include "libusb.h"
#include "common.h"

bool is_fm220_connected();

struct libusb_device_handle *fm220_device_handle();
int fm220_open(struct libusb_device_handle *handle);
void fm220_close(struct libusb_device_handle *handle);
device_config fm220_read_config(struct libusb_device_handle *handle);
void fm220_sensor_config_set(struct libusb_device_handle *handle, device_config *config);
int fm220_get_image(struct libusb_device_handle *handle, device_config config, unsigned char *image);

int fm220_vga_getframe(libusb_device_handle *handle, unsigned char *lpImage);

int reg_read(libusb_device_handle *handle, unsigned char addr, unsigned char *value);
int reg_write(libusb_device_handle *handle, unsigned char addr, unsigned char value);
int reg_write_cmd(libusb_device_handle *handle, unsigned char cmd, unsigned char addr, unsigned char value);

int eeprom_read(struct libusb_device_handle *handle, unsigned char offset, int len, unsigned char *buf);
int eeprom_write(struct libusb_device_handle *handle, unsigned char offset, int len, unsigned char *buf);

int led_on(libusb_device_handle *handle);
int led_off(libusb_device_handle *handle);

int clear_ep2_buffer(struct libusb_device_handle *handle);
#endif /* fm220_h */
