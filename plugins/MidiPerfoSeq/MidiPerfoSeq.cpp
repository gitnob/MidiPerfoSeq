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
    : Plugin(4, 0, 0),b_record(0.0f),b_trigger(0.0f) {}

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
                case gSetup:
                    portGroup.name = "Setup Sequencer";
                    portGroup.symbol = "setup";
                    break;
                case gPitch:
                    portGroup.name = "Pitch Control";
                    portGroup.symbol = "pitch";
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
                parameter.groupId   = gSetup;
                break;
            case bReset:
                parameter.hints      = kParameterIsAutomatable+kParameterIsTrigger;
                parameter.name       = "Reset";
                parameter.symbol     = "reset";
                parameter.ranges.min = 0.0f;
                parameter.ranges.max = 1.0f;
                parameter.ranges.def = 0.0f;
                parameter.groupId   = gSetup;
                break;
            case seqStyle:
                parameter.hints      = kParameterIsAutomatable+kParameterIsInteger;
                parameter.name       = "Sequencer Style";
                parameter.symbol     = "seqstyle";
                parameter.ranges.min = 0.0f;
                parameter.ranges.max = 2.0f;
                parameter.ranges.def = 0.0f;
                parameter.groupId   = gSetup;
                parameter.enumValues.count = 3;
                parameter.enumValues.restrictedMode = true;
                {
                    ParameterEnumerationValue* const enumValues = new ParameterEnumerationValue[3];
                    enumValues[0].value = 0.0f;
                    enumValues[0].label = "Forward";
                    enumValues[1].value = 1.0f;
                    enumValues[1].label = "Backward";
                    enumValues[2].value = 2.0f;
                    enumValues[2].label = "Random";
                    parameter.enumValues.values = enumValues;
                }
                break;
            case groupNumber:
                parameter.hints      = kParameterIsOutput;
                parameter.name       = "Steps";
                parameter.symbol     = "groupNumber";
                parameter.ranges.min = 0.0f;
                parameter.ranges.max = 128.0f;
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
                return b_trigger;
                break;
            case seqStyle:
                return sequencerStyle;
                break;
            case groupNumber:
                return noteOnQueueVector.size();
                break;
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
                noteOnQueueVectorIndex = 0;  // index auf Null setzen
                break;
            case bReset:
                noteOnQueueVector.clear();
                noteOnQueueVectorIndex = 0;  // index auf Null setzen
                break;
            case seqStyle:
                sequencerStyle = int(value);
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
                noteOnQueueVectorIndex %= noteOnQueueVector.size();
                break;
            }
            case 1:  // backward
            {
                noteOnQueueVectorIndex += noteOnQueueVector.size();
                noteOnQueueVectorIndex -= 1;
                noteOnQueueVectorIndex %= noteOnQueueVector.size();
                break;
            }
            case 2:  // random
            {
                noteOnQueueVectorIndex = rand() % noteOnQueueVector.size();
                break;
            }
        }
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
     *    Run/process function for plugins with MIDI input.
     *  It collects all midi events during a gate on (any key is pressed), and
     * saves it in a queue, which itself is stored in an array, which will be incremented,
     * when a new gate on for all keys appears.
     * This function is only during recording state.
     * After recording, a single key event results in a sequence of midi events (single notes or chords).
     * The sequence is played depending from the sequenceStyle state.
     */
    void run(const float**, float**, uint32_t,
             const MidiEvent* midiEvents, uint32_t midiEventCount) override
             {
                 for (uint32_t i=0; i<midiEventCount; ++i)
                 {
                     MidiEvent midiEvent = midiEvents[i];
                     if (midiEvent.size <= midiEvent.kDataSize)
                     {
                         if (b_record)
                         {
                             switch (midiEvent.data[0] & 0xF0)
                             {
                                 case 0x80:
                                 {
                                     if (activeNoteOnCount > 0) activeNoteOnCount -= 1;
                                     break;
                                 }
                                 case 0x90:
                                 {
                                     if (activeNoteOnCount == 0) noteOnQueueVector.push_back(MidiQueue());
                                     activeNoteOnCount += 1;
                                     MidiEvent me = midiEvent;
                                     me.frame = activeNoteOnCount;
                                     noteOnQueueVector.back().push(me);

                                     break;

                                 }
                             }
                             writeMidiEvent(midiEvent);

                         }
                         else
                         {
                             switch (midiEvent.data[0] & 0xF0)
                             {
                                 case 0x80:
                                 {
                                     if (activeNoteOnCount > 0) activeNoteOnCount -= 1;
                                     if ((noteOnQueueVector.size()>0) &&(activeNoteOnCount == 0))
                                     {
                                         for (int i=0;i<noteOnQueueVector.at(getSequencerIndex()).size();i++)
                                         {
                                             MidiEvent me = noteOnQueueVector.at(getSequencerIndex()).front();
                                             me.data[0] = (me.data[0] & 0x0F) + 0x80;  // create a note off
                                             me.frame = uint32_t(i);
                                             // std::cout << int(me.frame) << " time offset (frames)\n";
                                             // std::cout << std::hex << int(me.data[0]) << ",";
                                             // std::cout << std::hex << int(me.data[1]) << ",";
                                             // std::cout << std::hex << int(me.data[2]);
                                             // std::cout << "\n";
                                             writeMidiEvent(me);
                                             noteOnQueueVector.at(getSequencerIndex()).pop();
                                             noteOnQueueVector.at(getSequencerIndex()).push(me);
                                         }
                                         getNextSequencerIndex();
                                     }

                                     break;
                                 }
                                 case 0x90:
                                 {
                                     if (activeNoteOnCount == 0)
                                     {
                                         if (noteOnQueueVector.size()>0)
                                         {
                                             int sindex = getSequencerIndex();
                                             for (int i=0;i<noteOnQueueVector.at(sindex).size();i++)
                                             {
                                                 MidiEvent me = noteOnQueueVector.at(sindex).front();
                                                 me.frame = uint32_t(i);
                                                 me.data[0] = (me.data[0] & 0x0F) + 0x90;  // create a note off
                                                 writeMidiEvent(me);
                                                 noteOnQueueVector.at(sindex).pop();
                                                 noteOnQueueVector.at(sindex).push(me);
                                             }
                                         }
                                     }
                                     activeNoteOnCount += 1;
                                     break;

                                 }
                             }
                         }

                     }
                 }
             }

// ------------------------------------------------------------------------------------------------------

private:
    // midi event queue vector
    typedef std::queue<MidiEvent> MidiQueue;
    typedef std::vector<MidiQueue> MidiQueueVector;
    MidiQueueVector noteOnQueueVector;
    // Actual index in noteOnQueueVector
    int noteOnQueueVectorIndex = 0;  // initial value
    // Sequencer Style
    int sequencerStyle = 0;
    // midi note ON poly counter
    int activeNoteOnCount = 0;
    // recording switch
    int b_record = 1;
    // trigger
    int b_trigger = 0;
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
