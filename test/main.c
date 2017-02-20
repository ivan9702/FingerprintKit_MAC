//
//  main.c
//  test
//
//  Created by Yung-Luen Lan on 22/12/2016.
//  Copyright © 2016 hypo. All rights reserved.
//

#include <stdio.h>
#include "fm220.h"
#include "fp160.h"

int main(int argc, const char * argv[]) {
    libusb_init(NULL);
    struct libusb_device_handle *dev = fp160_device_handle();
//    struct libusb_device_handle *dev = fm220_device_handle();
    fp160_open(dev);
    device_config config = fp160_read_config(dev);
    uint8_t buf[256 * 256];
    fp160_get_image(dev, config, buf);
    
    libusb_exit(NULL);
    return 0;
}
