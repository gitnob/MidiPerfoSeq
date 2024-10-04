#ifndef MIDI_PERFOSEQ_PLUGIN_INCLUDED
#define MIDI_PERFOSEQ_PLUGIN_INCLUDED

#include "DistrhoPlugin.hpp"

struct midiQueueEvent {
    int group;
    MidiEvent event;
};


enum Parameters {
    bRecord,
    bReset,
    seqStyle,
    groupNumber,
    parameterCount
};

enum PortGroups {
    gSetup,
    gPitch,
    portGroupsCount
};

#endif
