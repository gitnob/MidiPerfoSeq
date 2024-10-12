/*
 * DISTRHO Plugin Framework (DPF)
 * Copyright (C) 2012-2024 Filipe Coelho <falktx@falktx.com>
 *
 * Permission to use, copy, modify, and/or distribute this software for any purpose with
 * or without fee is hereby granted, provided that the above copyright notice and this
 * permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD
 * TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN
 * NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER
 * IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include "DistrhoPlugin.hpp"
#include "MidiPerfoSeq.h"
#include "iostream"
#include <random>
#include <queue>

START_NAMESPACE_DISTRHO

// -----------------------------------------------------------------------------------------------------------

/**
 * Plugin that demonstrates MIDI output in DPF.
 */
class MidiPerfoSeqPlugin : public Plugin
{
public:
    MidiPerfoSeqPlugin()
    : Plugin(10, 0, 0),b_record(0.0f),b_reset(0.0f) {}

protected:
    /* --------------------------------------------------------------------------------------------------------
     * Information */

    /**
     *    Get the plugin label.
     */
    const char* getLabel() const override
    {
        return "MidiPerfoSeq";
    }

    /**
     *    Get an extensive comment/description about the plugin.
     */
    const char* getDescription() const override
    {
        return "Performance oriented midi sequencer plugin";
    }

    /**
     *    Get the plugin author/maker.
     */
    const char* getMaker() const override
    {
        return "nobisoft";
    }

    /**
     *    Get the plugin homepage.
     */
    const char* getHomePage() const override
    {
        return "https://github.com/gitnob/MidiPerfoSeq";
    }

    /**
     *    Get the plugin license name (a single line of text).
     *    For commercial plugins this should return some short copyright information.
     */
    const char* getLicense() const override
    {
        return "GPLv2";
    }

    /**
     *    Get the plugin version, in hexadecimal.
     */
    uint32_t getVersion() const override
    {
        return d_version(1, 0, 0);
    }

    /* --------------------------------------------------------------------------------------------------------
     * Init and Internal data, unused in this plugin */

    void initPortGroup(uint32_t groupId, DISTRHO::PortGroup & portGroup) override
    {
        switch (groupId)
        {
            case gRecord:
                portGroup.name = "Recording Setup";
                portGroup.symbol = "recording";
                break;
            case gSequencer:
                portGroup.name = "Sequencer Control";
                portGroup.symbol = "sequencer";
                break;
            case gTranspose:
                portGroup.name = "Key Transpose";
                portGroup.symbol = "transpose";
                break;
            default:
                break;

        }
    };

    void  initParameter(uint32_t index, Parameter& parameter) override
    {
        switch (index)
        {
            case bRecord:
                parameter.hints      = kParameterIsAutomatable+kParameterIsBoolean;
                parameter.name       = "Recording";
                parameter.symbol     = "record";
                parameter.ranges.min = 0.0f;
                parameter.ranges.max = 1.0f;
                parameter.ranges.def = 0.0f;
                parameter.groupId   = gRecord;
                break;
            case bReset:
                parameter.hints      = kParameterIsAutomatable+kParameterIsTrigger;
                parameter.name       = "Reset";
                parameter.symbol     = "reset";
                parameter.ranges.min = 0.0f;
                parameter.ranges.max = 1.0f;
                parameter.ranges.def = 0.0f;
                parameter.groupId   = gRecord;
                break;
            case seqStyle:
                parameter.hints      = kParameterIsAutomatable+kParameterIsInteger;
                parameter.name       = "Sequencer Style";
                parameter.symbol     = "seqstyle";
                parameter.ranges.max = 6.0f;
                parameter.ranges.min = 0.0f;
                parameter.ranges.def = 0.0f;
                parameter.groupId   = gSequencer;
                parameter.enumValues.count = 6;
                parameter.enumValues.restrictedMode = true;
                {
                    ParameterEnumerationValue* const enumValues = new ParameterEnumerationValue[6];
                    enumValues[0].value = 0.0f;
                    enumValues[0].label = "Forward";
                    enumValues[1].value = 1.0f;
                    enumValues[1].label = "Backward";
                    enumValues[2].value = 2.0f;
                    enumValues[2].label = "Ping Pong";
                    enumValues[3].value = 3.0f;
                    enumValues[3].label = "Spiral";
                    enumValues[4].value = 4.0f;
                    enumValues[4].label = "Step (+up-down)";
                    enumValues[5].value = 5.0f;
                    enumValues[5].label = "Random";
                    parameter.enumValues.values = enumValues;
                }
                break;
            case seqStepsUp:
                parameter.hints      = kParameterIsAutomatable+kParameterIsInteger;
                parameter.name       = "Sequencer Steps Up";
                parameter.symbol     = "seqStepsUp";
                parameter.ranges.min = 1.0f;
                parameter.ranges.max = float(MAX_SEQUENCER_STEPS_SIZE);
                parameter.ranges.def = 1.0f;
                parameter.groupId   = gSequencer;
                break;
            case seqStepsDown:
                parameter.hints      = kParameterIsAutomatable+kParameterIsInteger;
                parameter.name       = "Sequencer Steps Down";
                parameter.symbol     = "seqStepsDown";
                parameter.ranges.min = 1.0f;
                parameter.ranges.max = float(MAX_SEQUENCER_STEPS_SIZE);
                parameter.ranges.def = 1.0f;
                parameter.groupId   = gSequencer;
                break;
            case transposeSemi:
                parameter.hints      = kParameterIsAutomatable+kParameterIsInteger;
                parameter.name       = "Transpose Semi";
                parameter.symbol     = "transposeSemi";
                parameter.ranges.min = -12.0f;
                parameter.ranges.max = +12.0f;
                parameter.ranges.def = 0.0f;
                parameter.groupId   = gTranspose;
                break;
            case transposeKey:
                parameter.hints      = kParameterIsAutomatable+kParameterIsBoolean;
                parameter.name       = "Transpose Last Key";
                parameter.symbol     = "transposeKey";
                parameter.ranges.min = 0.0f;
                parameter.ranges.max = 1.0f;
                parameter.ranges.def = 0.0f;
                parameter.groupId   = gTranspose;
                break;
            case transposeKeyBase:
                parameter.hints      = kParameterIsAutomatable+kParameterIsInteger;
                parameter.name       = "Transpose Key Base";
                parameter.symbol     = "transposeKeyBase";
                parameter.ranges.min = 1.0f;
                parameter.ranges.max = 127.0f;
                parameter.ranges.def = 48.0f;
                parameter.groupId   = gTranspose;
                break;
            case groupNumber:
                parameter.hints      = kParameterIsOutput+kParameterIsInteger;
                parameter.name       = "Steps";
                parameter.symbol     = "groupNumber";
                parameter.ranges.min = 0.0f;
                parameter.ranges.max = float(MAX_NOTE_ON_GROUPS);
                parameter.ranges.def = 0.0f;
                break;
            case actualGroup:
                parameter.hints      = kParameterIsOutput+kParameterIsInteger;
                parameter.name       = "Steps";
                parameter.symbol     = "actualGroup";
                parameter.ranges.min = 0.0f;
                parameter.ranges.max = float(MAX_NOTE_ON_GROUPS);
                parameter.ranges.def = 0.0f;
                break;
            default:
                break;
        }
    }
    float getParameterValue(uint32_t index) const   override
    {
        switch (index)
        {
            case bRecord:
                return b_record;
                break;
            case bReset:
                return b_reset;
                break;
            case seqStyle:
                return sequencerStyle;
                break;
            case seqStepsUp:
                return sequencerSubStepsUp;
                break;
            case seqStepsDown:
                return sequencerSubStepsDown;
                break;
            case transposeSemi:
                return transposeSemiNotes;
                break;
            case transposeKey:
                return transposeOnKeys;
                break;
            case transposeKeyBase:
                return transposeBaseKey;
                break;
            case groupNumber:
                return noteOnQueueVector.size();
                break;
            case actualGroup:
                return noteOnQueueVectorIndex+1;
            default:
                return 0.0;
                break;
        }
    }
    void  setParameterValue(uint32_t index, float value)  override
    {
        switch (index)
        {
            case bRecord:
                b_record = (value > 0);
                //noteOnQueueVectorIndex = 0;  // index auf Null setzen
                break;
            case bReset:
                b_reset = (value > 0);
                //noteOnQueueVector.clear();
                //noteOnQueueVectorIndex = 0;  // index auf Null setzen
                break;
            case seqStyle:
                sequencerStyle = int(value);
                noteOnQueueVectorIndex=0;
                sequencerStep=0;
                sequencerSubStep=0;
                break;
            case seqStepsUp:
                sequencerSubStepsUp = int(value);
                noteOnQueueVectorIndex=0;
                sequencerStep=0;
                sequencerSubStep=0;
                break;
            case seqStepsDown:
                sequencerSubStepsDown = int(value);
                noteOnQueueVectorIndex=0;
                sequencerStep=0;
                sequencerSubStep=0;
                break;
            case transposeSemi:
                transposeSemiNotes = int(value);
                break;
            case transposeKey:
                transposeOnKeys = (value > 0);
                break;
            case transposeKeyBase:
                transposeBaseKey = int(value);
                break;
            default:
                break;
        }
    }

    /* --------------------------------------------------------------------------------------------------------
     * Audio/MIDI Processing */


    /*
     * Depending on the sequencer type the next index is evaluated from the size of the noteOnQueueVector vector
     */
    int getNextSequencerIndex()
    {
        switch (sequencerStyle)
        {
            case 0:  // forward
            {
                noteOnQueueVectorIndex += 1;
                break;
            }
            case 1:  // backward
            {
                noteOnQueueVectorIndex += noteOnQueueVector.size();
                noteOnQueueVectorIndex -= 1;
                break;
            }
            case 2:  // ping pong
            {
                if (sequencerStep==0) sequencerStep=1;
                noteOnQueueVectorIndex += sequencerStep;
                if (noteOnQueueVectorIndex == noteOnQueueVector.size()-1) sequencerStep=-1;
                if (noteOnQueueVectorIndex <=0) sequencerStep=1;
                break;
            }
            case 3:  // spiral
            {
                if (sequencerStep %2)
                {
                    noteOnQueueVectorIndex = sequencerStep/2;
                } else
                {
                    noteOnQueueVectorIndex = (2*noteOnQueueVector.size()-1-sequencerStep)/2;
                }
                sequencerStep = (sequencerStep + 1) % noteOnQueueVector.size();
                break;
            }
            case 4:  // +sequencerSubStepsUp -sequencerSubStepsDown
            {
                if (sequencerSubStep<sequencerSubStepsUp-1)
                    noteOnQueueVectorIndex += 1;
                else
                    noteOnQueueVectorIndex -= sequencerSubStepsDown;
                sequencerSubStep += 1;
                sequencerSubStep %= sequencerSubStepsUp;
                noteOnQueueVectorIndex += MAX_SEQUENCER_STEPS_SIZE * noteOnQueueVector.size();
                break;
            }
            case 5:  // random
            {
                noteOnQueueVectorIndex = rand() % noteOnQueueVector.size();
                break;
            }
            case 6:  // random
            {
                noteOnQueueVectorIndex = rand() % noteOnQueueVector.size();
                break;
            }
        }
        noteOnQueueVectorIndex %= noteOnQueueVector.size();
        return noteOnQueueVectorIndex;
    }

    /*
     * Returns the actual index in the noteOnQueueVector
     */
    int getSequencerIndex()
    {
        return noteOnQueueVectorIndex;
    }


    /**
     *  Run/process function for plugins with MIDI input.
     *  The logic is a state machine, which is triggered by the lv2 parameter settings.
     */
    void run(const float**, float**, uint32_t,
             const MidiEvent* midiEvents, uint32_t midiEventCount) override
             {
                 for (uint32_t i=0; i<midiEventCount; ++i)
                 {
                     MidiEvent midiEvent = midiEvents[i];
                     if (midiEvent.size <= midiEvent.kDataSize)
                     {
                         // Count the activeNoteOnEvents and remeber last played Note
                         switch (midiEvent.data[0] & 0xF0)
                         {
                             case 0x80:
                             {
                                 activeNoteOnCount -= 1;
                                 break;

                             }
                             case 0x90:
                             {
                                 if (activeNoteOnCount == 0) lastNoteOnEvent = midiEvent;
                                 activeNoteOnCount += 1;
                                 break;
                             default:
                                 break;
                             }
                         }
                         if (activeNoteOnCount < 0) activeNoteOnCount = 0;
                         // calculate transpose value
                         int transposeNote=transposeSemiNotes;
                         if (transposeOnKeys)
                         {
                             transposeNote = lastNoteOnEvent.data[1] - transposeBaseKey + transposeSemiNotes;
                         }

                         //std::cout << "machineState: " << machineState << "\n";
                         //std::cout << "lastMachineState: " << lastMachineState << "\n";
                         //std::cout << "Tastenanzahl: " << activeNoteOnCount << "\n";
                         // std::cout << "Last Note On: " << int(lastNoteOnEvent.data[1]) << "\n";
                         // std::cout << "Semi Transpose: " << transposeSemiNotes << "\n";
                         // std::cout << "Key Transpose ONOFF: " << transposeOnKeys << "\n";
                         // std::cout << "Key Transpose Base: " << transposeBaseKey << "\n";

                         // playing notes until no key is pressed
                         int playMode = (machineState==play || machineState==recRequest || (machineState==initRequest && (lastMachineState==play || lastMachineState==recRequest))) && (noteOnQueueVector.size() > 0);
                         if (playMode)
                         {
                             activeNoteOnCount %= noteOnQueueVector.size();
                             switch (midiEvent.data[0] & 0xF0)
                             {
                                 case 0x80:
                                 {
                                     if ((noteOnQueueVector.size()>0) &&(activeNoteOnCount == 0))
                                     {
                                         for (int i=0;i<noteOnQueueVector.at(getSequencerIndex()).size();i++)
                                         {
                                             MidiEvent me = noteOnQueueVector.at(getSequencerIndex()).front();
                                             noteOnQueueVector.at(getSequencerIndex()).pop();
                                             noteOnQueueVector.at(getSequencerIndex()).push(me);
                                             me.data[0] = (me.data[0] & 0x0F) + 0x80;  // create a note off
                                             me.data[1] = (me.data[1] + 0x100 + transposeNote) % 0x100;
                                             me.frame = uint32_t(i);
                                             writeMidiEvent(me);
                                         }
                                         getNextSequencerIndex();
                                     }
                                     break;
                                 }
                                 case 0x90:
                                 {
                                     if (activeNoteOnCount == 1)
                                     {
                                         if (noteOnQueueVector.size()>0)
                                         {
                                             int sindex = getSequencerIndex();
                                             for (int i=0;i<noteOnQueueVector.at(sindex).size();i++)
                                             {
                                                 MidiEvent me = noteOnQueueVector.at(sindex).front();
                                                 noteOnQueueVector.at(sindex).pop();
                                                 noteOnQueueVector.at(sindex).push(me);
                                                 me.frame = uint32_t(i);
                                                 me.data[0] = (me.data[0] & 0x0F) + 0x90;  // create a note on
                                                 me.data[1] = (me.data[1] + 0x100 + transposeNote) % 0x100;
                                                 writeMidiEvent(me);
                                             }
                                         }
                                     }
                                     break;
                                 }
                                 default:
                                     writeMidiEvent(midiEvent);
                             }

                         }

                         // through all midi events, when no notes are in the queue array.
                         int throughMode = (machineState==play || machineState==recRequest || (machineState==initRequest && (lastMachineState==play || lastMachineState==recRequest))) && (noteOnQueueVector.size() == 0);
                         if (throughMode)
                         {
                             writeMidiEvent(midiEvent);
                         }

                         // recording notes until state logic isn't satisfied
                         int recMode = (machineState==rec || machineState==playRequest || (machineState==initRequest && (lastMachineState==rec || lastMachineState==playRequest)));
                         if (recMode)
                         {
                             switch (midiEvent.data[0] & 0xF0)
                             {
                                 case 0x90:
                                 {
                                     if (activeNoteOnCount == 1  && noteOnQueueVector.size()<MAX_NOTE_ON_GROUPS) noteOnQueueVector.push_back(MidiQueue());
                                     MidiEvent me = midiEvent;
                                     me.frame = activeNoteOnCount;
                                     noteOnQueueVector.back().push(me);

                                     break;

                                 }
                             }
                             writeMidiEvent(midiEvent);

                         }
                     }
                 }
                 // state machine logic
                 int oldMachineState = machineState;
                 switch (machineState)
                 {
                     case init:
                     {
                         if (noteOnQueueVector.size()) noteOnQueueVector.clear();
                         if (noteOnQueueVector.size()==0) machineState = play;
                         break;
                     }
                     case play:
                     {
                         if (b_record == 1) machineState = recRequest;
                         if (b_reset == 1) machineState = initRequest;
                         break;
                     }
                     case recRequest:
                     {
                         if (activeNoteOnCount==0) machineState = rec;
                         if (b_record == 0) machineState = play;
                         if (b_reset == 1) machineState = initRequest;
                         break;
                     }
                     case rec:
                     {
                         if (b_record == 0) machineState = playRequest;
                         if (b_reset == 1) machineState = initRequest;
                         break;
                     }
                     case playRequest:
                     {
                         if (b_record == 1) machineState = rec;
                         if (activeNoteOnCount==0) machineState = play;
                         if (b_reset == 1) machineState = initRequest;
                         break;
                     }
                     case initRequest:
                     {
                         if ((b_reset == 0) & (activeNoteOnCount==0))
                         {
                             //b_record = 0;
                             machineState = init;
                         }
                         break;
                     }
                 }
                 if (machineState != oldMachineState) lastMachineState=oldMachineState;

             }

             // ------------------------------------------------------------------------------------------------------

private:
    // turing machine state
    int machineState = init;
    int lastMachineState = init;
    // midi event queue vector
    typedef std::queue<MidiEvent> MidiQueue;
    typedef std::vector<MidiQueue> MidiQueueVector;
    MidiQueueVector noteOnQueueVector;
    // Actual index in noteOnQueueVector
    int noteOnQueueVectorIndex = 0;  // initial value
    // Sequencer Style
    int sequencerStyle = 0;
    // sequencer step
    int sequencerStep = 1;
    // sequencer substep counter and step size
    int sequencerSubStep = 0;
    int sequencerSubStepsUp = 2;
    int sequencerSubStepsDown = 1;
    // transposing
    MidiEvent lastNoteOnEvent;
    int transposeSemiNotes = 0;
    int transposeOnKeys = 0;
    int transposeBaseKey = 36;

    // midi note ON poly counter
    int activeNoteOnCount = 0;
    // recording switch
    int b_record = 0;
    // trigger
    int b_reset = 0;
    /**
     *    Set our plugin class as non-copyable and add a leak detector just in case.
     */
    DISTRHO_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MidiPerfoSeqPlugin)
};

/* ------------------------------------------------------------------------------------------------------------
 * Plugin entry point, called by DPF to create a new plugin instance. */

Plugin* createPlugin()
{
    return new MidiPerfoSeqPlugin();
}

// -----------------------------------------------------------------------------------------------------------

END_NAMESPACE_DISTRHO
