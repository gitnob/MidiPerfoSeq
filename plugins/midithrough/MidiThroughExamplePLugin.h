#ifndef MIDI_THROUGH_EXAMPLE_PLUGIN_INCLUDED
#define MIDI_THROUGH_EXAMPLE_PLUGIN_INCLUDED

#include "DistrhoPlugin.hpp"

struct midiQueueEvent {
    int group;
    MidiEvent event;
};


#endif
