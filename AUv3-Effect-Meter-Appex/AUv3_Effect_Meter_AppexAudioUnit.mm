//
//  AUv3_Effect_Meter_AppexAudioUnit.m
//  AUv3-Effect-Meter-Appex
//
//  Created by John Carlson on 7/27/19.
//  Copyright © 2019 John Carlson. All rights reserved.
//

#import "AUv3_Effect_Meter_AppexAudioUnit.h"

#import <AVFoundation/AVFoundation.h>

#import "BufferedAudioBus.hpp"
#import "MeterDSPKernel.hpp"

////////////////////////////////////////////////////////////////////////////////////////////

// Define parameter addresses.
const AUParameterAddress LEVEL_PARAMETER_ADDRESS = 0;

////////////////////////////////////////////////////////////////////////////////////////////

@interface AUv3_Effect_Meter_AppexAudioUnit ()

@property AUAudioUnitBus *outputBus;
@property AUAudioUnitBusArray *inputBusArray;
@property AUAudioUnitBusArray *outputBusArray;

@property (nonatomic, readwrite) AUParameterTree *parameterTree;

@end

////////////////////////////////////////////////////////////////////////////////////////////

@implementation AUv3_Effect_Meter_AppexAudioUnit{
    
    // C++ members need to be ivars; they would be copied on access if they were properties.
    MeterDSPKernel  _kernel;
    BufferedInputBus _inputBus;
}

@synthesize parameterTree = _parameterTree;

////////////////////////////////////////////////////////////////////////////////////////////

- (instancetype)initWithComponentDescription:(AudioComponentDescription)componentDescription options:(AudioComponentInstantiationOptions)options error:(NSError **)outError {
    self = [super initWithComponentDescription:componentDescription options:options error:outError];
    
    if (self == nil) {
        return nil;
    }
    
    // Initialize a default format for the busses.
    AVAudioFormat *defaultFormat = [[AVAudioFormat alloc] initStandardFormatWithSampleRate:44100.0 channels:2];
    
    // Create a DSP kernel to handle the signal processing.
    _kernel.init(defaultFormat.channelCount, defaultFormat.sampleRate);
    
    // Create parameter objects.
    AUParameter *levelParameter = [AUParameterTree createParameterWithIdentifier:@"level"
                                                                    name:@"Level"
                                                                 address:LEVEL_PARAMETER_ADDRESS
                                                                     min:0
                                                                     max:100
                                                                    unit:kAudioUnitParameterUnit_Percent
                                                                unitName:nil
                                                                   flags:0
                                                            valueStrings:nil
                                                     dependentParameters:nil];
    
    // Initialize the parameter values.
    levelParameter.value = 0.0;
    
    // Create the parameter tree.
    _parameterTree = [AUParameterTree createTreeWithChildren:@[ levelParameter ]];
    
    // Create the input and output busses (AUAudioUnitBus).
    _inputBus.init(defaultFormat, 8);
    _outputBus = [[AUAudioUnitBus alloc] initWithFormat:defaultFormat error:nil];
    
    // Create the input and output bus arrays (AUAudioUnitBusArray).
    _inputBusArray  = [[AUAudioUnitBusArray alloc] initWithAudioUnit:self busType:AUAudioUnitBusTypeInput busses: @[_inputBus.bus]];
    _outputBusArray = [[AUAudioUnitBusArray alloc] initWithAudioUnit:self busType:AUAudioUnitBusTypeOutput busses: @[_outputBus]];
    
    // Make a local pointer to the kernel to avoid capturing self.
    __block MeterDSPKernel *meterKernel = &_kernel;
    
    // implementorValueProvider is called when the value needs to be refreshed.
    _parameterTree.implementorValueProvider = ^(AUParameter *param) {
        return meterKernel->getParameter(param.address);
    };
    
    // A function to provide string representations of parameter values.
    _parameterTree.implementorStringFromValueCallback = ^(AUParameter *param, const AUValue *__nullable valuePtr) {
        AUValue value = valuePtr == nil ? param.value : *valuePtr;
        
        switch (param.address) {
            case LEVEL_PARAMETER_ADDRESS:
                return [NSString stringWithFormat:@"%.f", value];
            default:
                return @"?";
        }
    };
    
    self.maximumFramesToRender = 512;
    
    return self;
}

////////////////////////////////////////////////////////////////////////////////////////////

#pragma mark - AUAudioUnit Overrides

// If an audio unit has input, an audio unit's audio input connection points.
// Subclassers must override this property getter and should return the same object every time.
// See sample code.
- (AUAudioUnitBusArray *)inputBusses {
//#pragma message("implementation must return non-nil AUAudioUnitBusArray")
    return _inputBusArray;
}

////////////////////////////////////////////////////////////////////////////////////////////

// An audio unit's audio output connection points.
// Subclassers must override this property getter and should return the same object every time.
// See sample code.
- (AUAudioUnitBusArray *)outputBusses {
//#pragma message("implementation must return non-nil AUAudioUnitBusArray")
    return _outputBusArray;
}

////////////////////////////////////////////////////////////////////////////////////////////

// Allocate resources required to render.
// Subclassers should call the superclass implementation.
- (BOOL)allocateRenderResourcesAndReturnError:(NSError **)outError {
    if (![super allocateRenderResourcesAndReturnError:outError]) {
        return NO;
    }
    
    // Validate that the bus formats are compatible.
    // Allocate your resources.
    
    if (self.outputBus.format.channelCount != _inputBus.bus.format.channelCount) {
        if (outError) {
            *outError = [NSError errorWithDomain:NSOSStatusErrorDomain code:kAudioUnitErr_FailedInitialization userInfo:nil];
        }
        // Notify superclass that initialization was not successful
        self.renderResourcesAllocated = NO;
        
        return NO;
    }
    
    _inputBus.allocateRenderResources(self.maximumFramesToRender);
    
    _kernel.init(self.outputBus.format.channelCount, self.outputBus.format.sampleRate);
    
    return YES;
}

////////////////////////////////////////////////////////////////////////////////////////////

// Deallocate resources allocated in allocateRenderResourcesAndReturnError:
// Subclassers should call the superclass implementation.
- (void)deallocateRenderResources {
    // Deallocate your resources.
    [super deallocateRenderResources];
}

////////////////////////////////////////////////////////////////////////////////////////////

#pragma mark - AUAudioUnit (AUAudioUnitImplementation)

// Block which subclassers must provide to implement rendering.
- (AUInternalRenderBlock)internalRenderBlock {
    // Capture in locals to avoid ObjC member lookups. If "self" is captured in render, we're doing it wrong. See sample code.
    
    // Specify captured objects are mutable.
    __block MeterDSPKernel *state = &_kernel;
    __block BufferedInputBus *input = &_inputBus;
    
    return ^AUAudioUnitStatus(AudioUnitRenderActionFlags *actionFlags,
                              const AudioTimeStamp *timestamp,
                              AVAudioFrameCount frameCount,
                              NSInteger outputBusNumber,
                              AudioBufferList *outputData,
                              const AURenderEvent *realtimeEventListHead,
                              AURenderPullInputBlock pullInputBlock) {
        // Do event handling and signal processing here.
        
        AudioUnitRenderActionFlags pullFlags = 0;
        
        AUAudioUnitStatus err = input->pullInput(&pullFlags, timestamp, frameCount, 0, pullInputBlock);
        
        if (err != 0) { return err; }
        
        AudioBufferList *inAudioBufferList = input->mutableAudioBufferList;
        
        // If passed null output buffer pointers, process in-place in the input buffer.
        AudioBufferList *outAudioBufferList = outputData;
        if (outAudioBufferList->mBuffers[0].mData == nullptr) {
            for (UInt32 i = 0; i < outAudioBufferList->mNumberBuffers; ++i) {
                outAudioBufferList->mBuffers[i].mData = inAudioBufferList->mBuffers[i].mData;
            }
        }
        
        state->setBuffers(inAudioBufferList, outAudioBufferList);
        state->processWithEvents(timestamp, frameCount, realtimeEventListHead);
        
        return noErr;
    };
}

@end

