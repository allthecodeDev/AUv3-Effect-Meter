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
    
    void setTempo(double tempo){
        currentTempo = tempo;
    }
    
    void setCurrentBeatPosition(double beatPosition){
        currentBeatPosition = beatPosition;
    }
    
    void setSampleOffsetToNextBeat(NSInteger offsetToNextBeatPosition){
        sampleOffsetToNextBeat = offsetToNextBeatPosition;
    }
    
    void setCurrentMeasureDownbeatPosition(double downbeatPosition){
        currentMeasureDownbeatPosition = downbeatPosition;
    }
    
    ///////////////////////////////////////////////////////////////////////
    
    void setParameter(AUParameterAddress address, AUValue value) {
        switch (address) {
            case LEVEL_PARAMETER:
                levelValue = value;
                [audioUnit.parameterTree.children[LEVEL_PARAMETER] setValue:[NSNumber numberWithFloat:value] forKey:@"level"];
                //[[audioUnit.parameterTree parameterWithAddress: address] setValue:value];
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
        
        /*
        double tempoValue = 1.0;
        double* tempo = &tempoValue;
        
        double timeSigNumeratorValue = 4.0;
        double* timeSigNumerator = &timeSigNumeratorValue;
        
        NSInteger timeSigDenominatorValue = 4.0;
        NSInteger* timeSigDenominator = &timeSigDenominatorValue;
        
        double currentPositionValue = 0.0;
        double* currentPosition = &currentPositionValue;
        
        NSInteger offsetToNextBeatValue = 0;
        NSInteger* offsetToNextBeat = &offsetToNextBeatValue;
        
        double currentDownbeatPositionValue = 0.0;
        double* currentDownbeatPosition = &currentDownbeatPositionValue;
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
        
        audioUnit.musicalContextBlock(tempo,
                                      timeSigNumerator,
                                      timeSigDenominator,
                                      currentPosition,
                                      offsetToNextBeat,
                                      currentDownbeatPosition);
         */
        
        if(sampleOffsetToNextBeat < frameCount){
            
            if(bufferCycles < 10)
            {
                bufferCycles++;
            }
            else
            {
                //float* leftChannelLevel = (float*)inBufferListPtr->mBuffers[sampleOffsetToNextBeat-1].mData;
                
                //setParameter(LEVEL_PARAMETER_ADDRESS, bufferCycles*valueIncrement); //*leftChannelLevel];
                
                if(valueIncrement < 10)
                {
                    valueIncrement++;
                }
                else
                {
                    valueIncrement = 0;
                }
                
                bufferCycles = 0;
            }
        }
    }
    
    ///////////////////////////////////////////////////////////////////////
    
    void startRamp(AUParameterAddress address, AUValue value, AUAudioFrameCount duration){}
    
    ///////////////////////////////////////////////////////////////////////
    
    void handleMIDIEvent(AUMIDIEvent const& midiEvent) {}
    
    ///////////////////////////////////////////////////////////////////////
    
private:
    float sampleRate = 44100.0;
    int qtyChannels = 2;
    int bufferCycles = 0;
    int valueIncrement = 0;
    
    double currentTempo = 120.0;
    double currentBeatPosition = 0.0;
    NSInteger sampleOffsetToNextBeat = 0;
    double currentMeasureDownbeatPosition = 0.0;
    
    AudioBufferList* inBufferListPtr = nullptr;
    AudioBufferList* outBufferListPtr = nullptr;
    
    AUValue levelValue = 0.0;
    AUv3_Effect_Meter_AppexAudioUnit* audioUnit;
};

#endif /* MeterDSPKernel_h */
