/*
Modified 2023 MUSTARDTIGER
Copyright 2016 Aapo Nikkilä

This file is part of PPM Reader.

PPM Reader is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

PPM Reader is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with PPM Reader.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "PPMReader.h"

PPMReader::PPMReader(byte channelCount) {
    // Check for valid parameters
    if (channelCount > 0) {
        // Setup an array for storing channel values
        this->channelCount = channelCount;
        rawValues = new unsigned long[channelCount];
        validValues = new unsigned long[channelCount];
        for (int i = 0; i < channelCount; ++i) {
            rawValues[i] = 0;
            validValues[i] = 0;
        }
    }
}

PPMReader::~PPMReader() {
    delete [] rawValues;
    delete [] validValues;
}

void PPMReader::handleInterrupt() {
    // Remember the current micros() and calculate the time since the last pulseReceived()
    unsigned long previousMicros = microsAtLastPulse;
    microsAtLastPulse = interruptMicros;
    unsigned long time = microsAtLastPulse - previousMicros;

    if (time > blankTime) {
        /* If the time between pulses was long enough to be considered an end
         * of a signal frame, prepare to read channel values from the next pulses */
        if (pulseCounter == channelCount && this->onFrameComplete != nullptr) {
            this->onFrameComplete();
        }
        pulseCounter = 0;
    }
    else {
        // Store times between pulses as channel values
        //Serial.printf("%d(%d) = %d\n", pulseCounter, channelCount, time);
        if (pulseCounter < channelCount) {
            rawValues[pulseCounter] = time;
            
            if (time >= minChannelValue - channelValueMaxError && time <= maxChannelValue + channelValueMaxError) {
                validValues[pulseCounter] = constrain(time, minChannelValue, maxChannelValue);
            }
        }
        ++pulseCounter;
    }
}

unsigned long PPMReader::rawChannelValue(byte channel) {
    // Check for channel's validity and return the latest raw channel value or 0
    unsigned long value = 0;
    if (channel >= 1 && channel <= channelCount) {
        value = rawValues[channel-1];
    }
    return value;
}

unsigned long PPMReader::latestValidChannelValue(byte channel, unsigned long defaultValue) {
    // Check for channel's validity and return the latest valid channel value or defaultValue.
    unsigned long value = defaultValue;
    if (channel >= 1 && channel <= channelCount) {
        value = validValues[channel-1];
    }
    return value;
}
