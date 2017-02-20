//
//  FingerController.m
//  FingerprintKit
//
//  Created by Yung-Luen Lan on 23/12/2016.
//  Copyright © 2016 hypo. All rights reserved.
//

#import "FingerController.h"
#import <FingerprintKit/FingerprintKit.h>

@interface FingerController ()
@property (nonatomic, strong) HYFM220Scanner *scanner;
@end

@implementation FingerController

- (void) awakeFromNib
{
    self.scanner = [HYFM220Scanner device];
    [self.scanner open];
}

- (IBAction) toggleLED: (NSButton *)sender
{
    [self.scanner led: sender.state == NSOnState];
}

- (IBAction)snap:(id)sender
{
    self.imageView.image = [self.scanner snap];
}
@end
