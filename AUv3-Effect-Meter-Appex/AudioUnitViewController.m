//
//  AudioUnitViewController.m
//  AUv3-Effect-Meter-Appex
//
//  Created by John Carlson on 7/27/19.
//  Copyright © 2019 John Carlson. All rights reserved.
//

#import "AudioUnitViewController.h"
#import "AUv3_Effect_Meter_AppexAudioUnit.h"

///////////////////////////////////////////////////////////////////////

@interface AudioUnitViewController (){
    
    AUParameter* _levelParameter;
    AUValue _level;
    AUParameterObserverToken _parameterObserverToken;
    
    __weak IBOutlet NSTextField *volumeLabel;
    __weak IBOutlet NSSlider *volumeSlider;
}

@end

///////////////////////////////////////////////////////////////////////

@implementation AudioUnitViewController {
    AUAudioUnit *audioUnit;
}

///////////////////////////////////////////////////////////////////////

- (void) viewDidLoad {
    [super viewDidLoad];
    
    self.preferredContentSize = NSMakeSize(480, 272);
    self.view.wantsLayer = true;
    self.view.layer.backgroundColor = NSColor.darkGrayColor.CGColor;
    
    if (!audioUnit) {
        return;
    }
}

///////////////////////////////////////////////////////////////////////

-(void) viewDidDisappear {
    
    [_levelParameter removeParameterObserver:_parameterObserverToken];
    
    //[_levelParameter removeObserver:self forKeyPath:@"level"];
}

///////////////////////////////////////////////////////////////////////

- (AUAudioUnit *)createAudioUnitWithComponentDescription:(AudioComponentDescription)desc error:(NSError **)error {
    audioUnit = [[AUv3_Effect_Meter_AppexAudioUnit alloc] initWithComponentDescription:desc error:error];

    
    // Get the parameter tree and add observers for any parameters that the UI needs to keep in sync with the AudioUnit
    _levelParameter = [audioUnit.parameterTree parameterWithAddress: LEVEL_PARAMETER_ADDRESS];
    
    
    _parameterObserverToken = [_levelParameter tokenByAddingParameterObserver:^(AUParameterAddress address, AUValue value) {
        dispatch_async(dispatch_get_main_queue(), ^{
            
            if(address == LEVEL_PARAMETER_ADDRESS){
                
                self->_level = value; //self->_levelParameter.value;
                NSLog(@"level = %f ",self->_level);
                
                self->volumeLabel.stringValue = [NSString stringWithFormat:@"%f",self->_level];
                
                self->volumeSlider.doubleValue = self->_level;
                
                if(self->_level < 0.002)
                {
                    self.view.layer.backgroundColor = NSColor.blueColor.CGColor;
                }
                else if(self->_level < 0.004)
                {
                    self.view.layer.backgroundColor = NSColor.greenColor.CGColor;
                }
                else if(self->_level < 0.0075)
                {
                    self.view.layer.backgroundColor = NSColor.yellowColor.CGColor;
                }
                else if(self->_level < 0.01)
                {
                    self.view.layer.backgroundColor = NSColor.orangeColor.CGColor;
                }
                else
                {
                    self.view.layer.backgroundColor = NSColor.redColor.CGColor;
                }
            }
        });
    }];
    
    return audioUnit;
}

///////////////////////////////////////////////////////////////////////

- (IBAction)volumeSliderDidChange:(NSSlider *)sender {
    
    //_level = sender.doubleValue;
    _levelParameter.value = sender.doubleValue;
}

///////////////////////////////////////////////////////////////////////

@end
