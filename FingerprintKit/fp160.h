//
//  fp160.h
//  FingerprintKit
//
//  Created by Yung-Luen Lan on 23/12/2016.
//  Copyright © 2016 hypo. All rights reserved.
//

#ifndef fp160_h
#define fp160_h

#include "common.h"

struct libusb_device_handle *fp160_device_handle();
int fp160_open(struct libusb_device_handle *handle);
void fp160_close(struct libusb_device_handle *handle);
device_config fp160_read_config(struct libusb_device_handle *handle);
int fp160_get_image(struct libusb_device_handle *handle, device_config config, uint8_t *image);
#endif /* fp160_h */
