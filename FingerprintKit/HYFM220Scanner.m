//
//  HYFM220Scanner.m
//  FingerprintKit
//
//  Created by Yung-Luen Lan on 22/12/2016.
//  Copyright © 2016 hypo. All rights reserved.
//

#import "HYFM220Scanner.h"
#import "fm220.h"
#import <Cocoa/Cocoa.h>

@interface HYFM220Scanner ()
@property (nonatomic, assign) struct libusb_device_handle *handle;
@property (nonatomic, assign) device_config config;
- (instancetype) initWithUSBDeviceHandle: (struct libusb_device_handle *)deviceHandle;
@end

@implementation HYFM220Scanner

static dispatch_once_t onceToken;

+ (BOOL) hasConnectedDevice
{
    dispatch_once(&onceToken, ^{
        NSAssert(libusb_init(NULL) >= 0, @"Cannot initialize libusb");
    });
    return is_fm220_connected();
}

+ (instancetype) device
{
    dispatch_once(&onceToken, ^{
        NSAssert(libusb_init(NULL) >= 0, @"Cannot initialize libusb");
    });
    
    struct libusb_device_handle *handle = fm220_device_handle();
    if (handle == NULL) return nil;
    return [[HYFM220Scanner alloc] initWithUSBDeviceHandle: handle];
}

- (instancetype) initWithUSBDeviceHandle: (struct libusb_device_handle *)deviceHandle
{
    self = [super init];
    if (self) {
        _handle = deviceHandle;
    }
    return self;
    
}

- (void) dealloc
{
    fm220_close(self.handle);
}

- (void) open
{
    fm220_open(self.handle);
    self.config = fm220_read_config(self.handle);
    fm220_sensor_config_set(self.handle, &_config); // this function will write to config
}

- (void) led: (BOOL)on
{
    (on ? led_on : led_off)(self.handle);
}

- (NSImage *) snap
{
    int width = self.config.FP_IMAGE_WIDTH;
    int height = self.config.FP_IMAGE_HEIGHT;
    uint8_t buf[width * height];
    uint8_t rgba[width * height * 4];
    
    fm220_get_image(self.handle, self.config, buf);
    
    CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
    
    for (int i = 0; i < width * height; i++) {
        rgba[i * 4] = rgba[i * 4 + 1] = rgba[i * 4 + 2] = rgba[i * 4 + 3] = buf[i];
    }
    
    CGContextRef bitmapContext = CGBitmapContextCreate(
                                                       rgba,
                                                       width,
                                                       height,
                                                       8, // bitsPerComponent
                                                       4 * width, // bytesPerRow
                                                       colorSpace,
                                                       kCGImageAlphaNoneSkipLast);
    CFRelease(colorSpace);
    CGImageRef cgImage = CGBitmapContextCreateImage(bitmapContext);

    NSImage *image = [[NSImage alloc] initWithCGImage: cgImage size: NSMakeSize(width, height)];
    return image;
}
@end
