//
//  HYFM220Scanner.h
//  FingerprintKit
//
//  Created by Yung-Luen Lan on 22/12/2016.
//  Copyright © 2016 hypo. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "libusb.h"

@interface HYFM220Scanner : NSObject
+ (BOOL) hasConnectedDevice;
+ (instancetype) device;

- (void) open;
- (void) led: (BOOL)on;
- (NSImage *) snap;
@end
