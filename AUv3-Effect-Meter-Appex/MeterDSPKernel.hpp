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
    
    void init(int channelCount, double inSampleRate) {
        
        sampleRate = float(inSampleRate);
        qtyChannels = channelCount;
    }
    
    ///////////////////////////////////////////////////////////////////////
    
    AUValue getParameter(AUParameterAddress address) {
        switch (address) {
            case LEVEL_PARAMETER:
                // Return the goal. It is not thread safe to return the ramping value.
                //return (cutoffRamper.getUIValue() * nyquist);
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
        
        float* leftChannelLevel = (float*)inBufferListPtr->mBuffers[0].mData;
        levelValue = *leftChannelLevel;
        
        for (int frameIndex = 0; frameIndex < frameCount; ++frameIndex) {
            
            int frameOffset = int(frameIndex + bufferOffset);
            
            for (int channel = 0; channel < qtyChannels; ++channel) {
                float* in  = (float*)inBufferListPtr->mBuffers[channel].mData  + frameOffset;
                float* out = (float*)outBufferListPtr->mBuffers[channel].mData + frameOffset;
                
                out = in;
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
    
    AudioBufferList* inBufferListPtr = nullptr;
    AudioBufferList* outBufferListPtr = nullptr;
    
    AUValue levelValue = 0.0;
};

#endif /* MeterDSPKernel_h */
