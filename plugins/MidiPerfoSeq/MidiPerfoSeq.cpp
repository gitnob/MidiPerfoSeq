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
    : Plugin(3, 0, 0),b_record(0.0f),b_trigger(0.0f) {}

protected:
    /* --------------------------------------------------------------------------------------------------------
     * Information */

    /**
     *    Get the plugin label.
     *    This label is a short restricted name consisting of only _, a-z, A-Z and 0-9 characters.
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
                break;
            case bReset:
                parameter.hints      = kParameterIsAutomatable+kParameterIsTrigger;
                parameter.name       = "Reset";
                parameter.symbol     = "reset";
                parameter.ranges.min = 0.0f;
                parameter.ranges.max = 1.0f;
                parameter.ranges.def = 0.0f;
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
            case groupNumber:
                return lastGroup;
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
                activeNoteOnCount = 0;
                break;
            case bReset:
                activeNoteOnCount = 0;
                // lastGroup = 0;
                while (!noteOnQueue.empty()) noteOnQueue.pop();
                break;
            default:
                break;
        }
    }

    /* --------------------------------------------------------------------------------------------------------
     * Audio/MIDI Processing */

    /**
     *    Run/process function for plugins with MIDI input.
     *    In this case we just pass-through all MIDI events.
     */
    void run(const float**, float**, uint32_t,
             const MidiEvent* midiEvents, uint32_t midiEventCount) override
             {
                 for (uint32_t i=0; i<midiEventCount; ++i)
                 {
                     MidiEvent midiEvent = midiEvents[i];
                     //std::cout << int(midiEvent.frame) << " time offset (frames)\n";
                     //std::cout << int(midiEvent.size) << " Bytes used\n";
                     //std::cout << std::hex << int(midiEvent.data[0]) << ",";
                     //std::cout << std::hex << int(midiEvent.data[1]) << ",";
                     //std::cout << std::hex << int(midiEvent.data[2]) << ",";
                     //std::cout << std::hex << int(midiEvent.data[3]);
                     //std::cout << "\n";
                     if (midiEvent.size <= midiEvent.kDataSize)
                     {
                         if (b_record)
                         {
                             switch (midiEvent.data[0] & 0xF0)
                             {
                                 case 0x80:
                                 {
                                     if (activeNoteOnCount > 0) activeNoteOnCount -= 1;
                                     // std::cout << "NoteOff\n";
                                     // std::cout << "lastGroup: " << lastGroup << "\n";
                                     break;
                                 }
                                 case 0x90:
                                 {
                                     // get last used group number
                                     if (noteOnQueue.empty())
                                     {
                                         lastGroup = 0;
                                     }
                                     if (activeNoteOnCount == 0) lastGroup += 1;
                                     activeNoteOnCount += 1;
                                     midiQueueEvent mqe;
                                     mqe.group = lastGroup;
                                     mqe.event = midiEvent;
                                     mqe.event.frame = activeNoteOnCount;
                                     noteOnQueue.push(mqe);

                                     // std::cout << "Queueing in group (" << lastGroup << "): ";
                                     // std::cout << std::hex << int(midiEvent.data[0]) << ",";
                                     // std::cout << std::hex << int(midiEvent.data[1]) << ",";
                                     // std::cout << std::hex << int(midiEvent.data[2]) << "\n";
                                     // std::cout << "Queue size: " << noteOnQueue.size() << "\n";

                                     break;

                                 }
                             }
                             //playQueue.push(midiEvent);
                             std::cout << writeMidiEvent(midiEvent) << "\n";

                         }
                         else
                         {
                             switch (midiEvent.data[0] & 0xF0)
                             {
                                 case 0x80:
                                 {
                                     if (activeNoteOnCount > 0) activeNoteOnCount -= 1;
                                     while (!noteOffQueue.empty())
                                     {
                                         std::cout << writeMidiEvent(noteOffQueue.front()) << "\n";
                                         //playQueue.push(noteOffQueue.front());
                                         noteOffQueue.pop();
                                     }
                                     break;
                                 }
                                 case 0x90:
                                 {
                                     // get last used group number
                                     if (activeNoteOnCount == 0)
                                     {
                                         activeNoteOnCount += 1;
                                         for (uint16_t i=0;i<noteOnQueue.size();i++)
                                         {
                                             // sent midi events in actual group
                                             midiQueueEvent mqe = noteOnQueue.front();
                                             actualGroup = mqe.group;
                                             noteOnQueue.pop();
                                             noteOnQueue.push(mqe);
                                             std::cout << int(mqe.event.frame) << " time offset (frames)\n";
                                             std::cout << std::hex << int(mqe.event.data[0]) << ",";
                                             std::cout << std::hex << int(mqe.event.data[1]) << ",";
                                             std::cout << std::hex << int(mqe.event.data[2]);
                                             std::cout << "\n";
                                             std::cout << writeMidiEvent(mqe.event) << "\n";
                                             //playQueue.push(mqe.event);
                                             // push NOFF into note off queue
                                             mqe.event.data[0] = (mqe.event.data[0] & 0x0F) | 0x80;
                                             noteOffQueue.push(mqe.event);
                                             int nextGroup = noteOnQueue.front().group;
                                             if (actualGroup != nextGroup)
                                             {
                                                 actualGroup = nextGroup;
                                                 break;  // for loop
                                             }
                                         }
                                     }
                                     break;

                                 }
                             }
                         }

                     }
                 }
            }

             // ------------------------------------------------------------------------------------------------------

private:
    // midi event queue
    std::queue<midiQueueEvent> noteOnQueue;
    std::queue<MidiEvent> noteOffQueue;
    std::queue<MidiEvent> playQueue;
    // midi note ON poly counter
    int activeNoteOnCount = 0;
    // last group number
    int lastGroup;
    // actual group number of midi events to be sent
    int actualGroup = 1;
    // recording switch
    int b_record = 1;
    // trigger
    int b_trigger = 0;
    // trigger
    int b_last_trigger = 0;
    // trigger change event
    int b_trigger_changed = 0;
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
