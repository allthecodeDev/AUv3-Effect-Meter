//
//  MeterDSPKernel.hpp
//  AUv3-Effect-Meter
//
//  Created by John Carlson on 7/27/19.
//  Copyright © 2019 John Carlson. All rights reserved.
//

#ifndef MeterDSPKernel_h
#define MeterDSPKernel_h

#import "DSPKernel.hpp"
#import "AUv3_Effect_Meter_AppexAudioUnit.h"
#import <CoreAudio/CoreAudioTypes.h>
#import <vector>
#import <memory>
#import <cstring>

///////////////////////////////////////////////////////////////////////

enum{
    LEVEL_PARAMETER = 0
};

///////////////////////////////////////////////////////////////////////

/*
 Performs signal processing.
 As a non-ObjC class, this is safe to use from render thread.
 */

class MeterDSPKernel : public DSPKernel {
public:
    ///////////////////////////////////////////////////////////////////////
    
    // MARK: Member Functions
    
    ///////////////////////////////////////////////////////////////////////
    
    MeterDSPKernel() {}
    
    ///////////////////////////////////////////////////////////////////////
    
    void init(AUv3_Effect_Meter_AppexAudioUnit* unit) {
        
        audioUnit = unit;
        sampleRate =  audioUnit.outputBusses[0].format.sampleRate;
        qtyChannels = audioUnit.outputBusses[0].format.channelCount;
    }
    
    ///////////////////////////////////////////////////////////////////////
    
    void setParameter(AUParameterAddress address, AUValue value) {
        switch (address) {
            case LEVEL_PARAMETER:
                levelValue = value;
                [[audioUnit.parameterTree parameterWithAddress: address] setValue:value];
                break;
        }
    }
    
    ///////////////////////////////////////////////////////////////////////
    
    AUValue getParameter(AUParameterAddress address) {
        switch (address) {
            case LEVEL_PARAMETER:
                // Return the goal. It is not thread safe to return the ramping value.
                return levelValue;
        }
        return 0.0;
    }
    
    ///////////////////////////////////////////////////////////////////////
    
    void setBuffers(AudioBufferList* inBufferList, AudioBufferList* outBufferList) {
        inBufferListPtr = inBufferList;
        outBufferListPtr = outBufferList;
    }
    
    ///////////////////////////////////////////////////////////////////////
    
    void process(AUAudioFrameCount frameCount, AUAudioFrameCount bufferOffset){
        
        //https://developer.apple.com/documentation/audiotoolbox/auhostmusicalcontextblock?language=objc
        
        audioUnit.musicalContextBlock = ^BOOL(double * _Nullable currentTempo,
                                              double * _Nullable timeSignatureNumerator,
                                              NSInteger * _Nullable timeSignatureDenominator,
                                              double * _Nullable currentBeatPosition,
                                              NSInteger * _Nullable sampleOffsetToNextBeat,
                                              double * _Nullable currentMeasureDownbeatPosition) {
            
            //TODO: get downbeat position and provide set level parameter value on downbeat only
            if(*sampleOffsetToNextBeat < frameCount){
                
                float* leftChannelLevel = (float*)inBufferListPtr->mBuffers[*sampleOffsetToNextBeat].mData;
                
                [[audioUnit.parameterTree parameterWithAddress: LEVEL_PARAMETER_ADDRESS] setValue: *leftChannelLevel];
            }
            
            if(currentTempo){
                return YES;
            }
            else{
                return NO;
            }
        };
        
        
    }
    
    ///////////////////////////////////////////////////////////////////////
    
    void startRamp(AUParameterAddress address, AUValue value, AUAudioFrameCount duration){}
    
    ///////////////////////////////////////////////////////////////////////
    
    void handleMIDIEvent(AUMIDIEvent const& midiEvent) {}
    
    ///////////////////////////////////////////////////////////////////////
    
private:
    float sampleRate = 44100.0;
    int qtyChannels = 2;
    
    AudioBufferList* inBufferListPtr = nullptr;
    AudioBufferList* outBufferListPtr = nullptr;
    
    AUValue levelValue = 0.0;
    AUv3_Effect_Meter_AppexAudioUnit* audioUnit;
};

#endif /* MeterDSPKernel_h */
