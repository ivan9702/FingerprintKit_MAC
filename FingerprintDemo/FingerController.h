//
//  FingerController.h
//  FingerprintKit
//
//  Created by Yung-Luen Lan on 23/12/2016.
//  Copyright © 2016 hypo. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@interface FingerController : NSObject
@property (weak) IBOutlet NSImageView *imageView;
- (IBAction)toggleLED:(id)sender;
- (IBAction)snap:(id)sender;

@end
