//
//  common.c
//  FingerprintKit
//
//  Created by Yung-Luen Lan on 23/12/2016.
//  Copyright © 2016 hypo. All rights reserved.
//

#include "common.h"

struct libusb_device *first_matching_device(struct libusb_device **devs, device_predicate_t *valid_device)
{
    struct libusb_device *dev;
    int i = 0;
    while ((dev = devs[i++]) != NULL) {
        struct libusb_device_descriptor desc;
        if (libusb_get_device_descriptor(dev, &desc) < 0)
            continue;
        if (valid_device(desc)) return dev;
    }
    return NULL;
}

struct libusb_device_handle *device_handle(device_predicate_t *valid_device)
{
    struct libusb_device **devs;
    
    if (libusb_get_device_list(NULL, &devs) < 0) {
        LOG("cannot list device");
        return NULL;
    }
    
    struct libusb_device *found = first_matching_device(devs, valid_device);
    struct libusb_device_handle *handle = NULL;
    
    if (found) {
        int r = libusb_open(found, &handle);
        if (r < 0) {
            LOG("libusb open failed: %d", r);
            handle = NULL;
        }
    }
    
    libusb_free_device_list(devs, 1);
    return handle;
}

const int EP1_OUT = (1 | LIBUSB_ENDPOINT_OUT);
const int EP1_IN  = (1 | LIBUSB_ENDPOINT_IN);
const int EP2_IN  = (2 | LIBUSB_ENDPOINT_IN);

int set_vga_start(libusb_device_handle *handle)
{
    unsigned char data[0x10] = {0x00, 0x0a};
    int actual_length;
    int r = libusb_bulk_transfer(handle, EP1_OUT, data, 2, &actual_length, 500);
    if (r < 0) {
        LOG("set_vga_start %d\n", r);
        return r;
    }
    return 0;
}

void vga_frame_callback(struct libusb_transfer *transfer)
{
    if(transfer->status != LIBUSB_TRANSFER_COMPLETED) {
        fprintf(stderr, "transfer not completed!  ");
    }
    *((int *)transfer->user_data) += 1;
}

int vga_getframe(libusb_device_handle *handle, unsigned char *lpImage, int vga_width, int vga_height)
{
    struct libusb_transfer *transfer[vga_height];
    int vga_flag = 0;
    
    for (int ln = 0; ln < vga_height; ln++) {
        transfer[ln] = libusb_alloc_transfer(0);
        libusb_fill_bulk_transfer(transfer[ln], handle, EP2_IN, lpImage + ln * vga_width, vga_width, vga_frame_callback, &vga_flag, 1500);
        int r = libusb_submit_transfer (transfer[ln]);
        if (r < 0) {
            LOG("vga_getframe send cmd error %d\n", r);
            // FIXME: leak
            return r;
        }
    }
    
    while (vga_flag != vga_height) {
        if (libusb_handle_events(NULL) < 0) {
            LOG("recieve img error");
            break;
        }
    }
    
    //check FM210(VGA_HEIGHT=1) connected or not
    for (int ln = 0; ln < vga_height; ln++) {
        if (transfer[ln] != NULL) {
            if (transfer[ln]->status == 1 || transfer[ln]->status == 5)
                return -1;
        }
    }
    
    for (int ln = 0; ln < vga_height; ln++) {
        if (transfer[ln] != NULL) {
            libusb_free_transfer(transfer[ln]);
        }
    }
    return 0;
}
