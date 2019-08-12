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

@class AUAudioUnitBus;

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
    
    void init(AUv3_Effect_Meter_AppexAudioUnit* unit, UInt32 byteSize) {
        
        _audioUnit = unit;
        _levelParameter = [_audioUnit.parameterTree parameterWithAddress:LEVEL_PARAMETER];
        //
        _parameterScheduleBlock = _audioUnit.scheduleParameterBlock;
        _midiEventScheduleBlock = _audioUnit.scheduleMIDIEventBlock;
        //
        _sampleRate =  _audioUnit.outputBusses[0].format.sampleRate;
        _qtyChannels = _audioUnit.outputBusses[0].format.channelCount;
        _bufferDataSize = byteSize;
    }
    
    ///////////////////////////////////////////////////////////////////////
    
    void setTempo(double tempo){
        _currentTempo = tempo;
    }
    
    void setCurrentBeatPosition(double beatPosition){
        _currentBeatPosition = beatPosition;
    }
    
    void setSampleOffsetToNextBeat(NSInteger offsetToNextBeatPosition){
        _sampleOffsetToNextBeat = offsetToNextBeatPosition;
    }
    
    void setCurrentMeasureDownbeatPosition(double downbeatPosition){
        _currentMeasureDownbeatPosition = downbeatPosition;
    }
    
    ///////////////////////////////////////////////////////////////////////
    
    void setTransportState(NSUInteger transportState){
        _transportStatus = transportState;
    }
    
    ///////////////////////////////////////////////////////////////////////
    
    void setParameter(AUParameterAddress address, AUValue value) {
        switch (address) {
            case LEVEL_PARAMETER:
                _levelValue = value;
                //[levelParameter setValue:[NSNumber numberWithFloat:value] forKey:@"level"];

                break;
        }
    }
    
    ///////////////////////////////////////////////////////////////////////
    
    AUValue getParameter(AUParameterAddress address) {
        switch (address) {
            case LEVEL_PARAMETER:
                // Return the goal. It is not thread safe to return the ramping value.
                return _levelValue;
        }
        return 0.0;
    }
    
    ///////////////////////////////////////////////////////////////////////
    
    void setBuffers(AudioBufferList* inBufferList,
                    AudioBufferList* outBufferList) {
         _inBufferListPtr = inBufferList;
        _outBufferListPtr = outBufferList;
    }
    
    ///////////////////////////////////////////////////////////////////////
    
    void process(AUAudioFrameCount frameCount, AUAudioFrameCount bufferOffset){
        
        /*
        if( (sampleOffsetToNextBeat < frameCount) &&
            (AUHostTransportStateMoving == transportStatus) ){
         
            leftChannelLevelPtr = (float*)inBufferListPtr->mBuffers[0].mData + bufferDataSize*sampleOffsetToNextBeat;
            
            levelValue = *leftChannelLevelPtr;
            
            scheduleBlock(AUEventSampleTimeImmediate + sampleOffsetToNextBeat,
                          256,
                          LEVEL_PARAMETER,
                          levelValue);
         
        }
         */
        
        /*
        if(meterWaitCycleQty < 1)
        {
            meterWaitCycleQty++;
        }
        else
        {
            meterWaitCycleQty = 0;
            //
            if(AUHostTransportStateMoving == transportStatus){
                leftChannelLevelPtr = (float*)inBufferListPtr->mBuffers[0].mData;
                
                levelValue = *leftChannelLevelPtr;
                
                scheduleBlock(AUEventSampleTimeImmediate,
                              0,
                              LEVEL_PARAMETER,
                              levelValue);
            }
        }
         */
        
        if(AUHostTransportStateMoving == _transportStatus){
            _leftChannelLevelPtr = (float*)_inBufferListPtr->mBuffers[0].mData;
            
            _levelValue = *_leftChannelLevelPtr;
            
            _parameterScheduleBlock(AUEventSampleTimeImmediate,
                          0,
                          LEVEL_PARAMETER,
                          _levelValue);
            
            ////////////////
            
            _leftChannelLevelPtr = _leftChannelLevelPtr + (_bufferDataSize*frameCount/2);
            
            _levelValue = *_leftChannelLevelPtr;
            
            _parameterScheduleBlock(AUEventSampleTimeImmediate + (frameCount/2),
                                    0,
                                    LEVEL_PARAMETER,
                                    _levelValue);
            
            /*
            const uint8_t midiBytes[] = {(uint8_t)0xFF,(uint8_t)0xFF};
            
            _midiEventScheduleBlock(AUEventSampleTimeImmediate,
                                    0, //uint8_t cable,
                                    1, //NSInteger length,
                                    midiBytes); //const uint8_t *midiBytes
             */
        }
    }
    
    ///////////////////////////////////////////////////////////////////////
    
    void startRamp(AUParameterAddress address, AUValue value, AUAudioFrameCount duration){}
    
    ///////////////////////////////////////////////////////////////////////
    
    void handleMIDIEvent(AUMIDIEvent const& midiEvent) {}
    
    ///////////////////////////////////////////////////////////////////////
    
private:
    float _sampleRate = 44100.0;
    int _qtyChannels = 2;
    
    double _currentTempo = 120.0;
    double _currentBeatPosition = 0.0;
    NSInteger _sampleOffsetToNextBeat = 0;
    double _currentMeasureDownbeatPosition = 0.0;
    
    NSUInteger _transportStatus = 0;
    
    int _bufferDataSize = 0;
    int _bufferQtyChannels = 0;
    
    AudioBufferList* _inBufferListPtr = nullptr;
    AudioBufferList* _outBufferListPtr = nullptr;
    
    float* _leftChannelLevelPtr;
    
    AUValue _levelValue = 0.0;
    AUv3_Effect_Meter_AppexAudioUnit* _audioUnit;
    AUParameter* _levelParameter;
    
    AUScheduleParameterBlock _parameterScheduleBlock;
    AUScheduleMIDIEventBlock _midiEventScheduleBlock;
    
    int _meterWaitCycleQty = 0;
};

#endif /* MeterDSPKernel_h */
