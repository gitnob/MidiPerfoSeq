#ifndef MIDI_PERFOSEQ_PLUGIN_INCLUDED
#define MIDI_PERFOSEQ_PLUGIN_INCLUDED

#include "DistrhoPlugin.hpp"

const int MAX_NOTE_ON_GROUPS = 128;
const int MAX_SEQUENCER_STEPS_SIZE = 16;

struct midiQueueEvent {
    int group;
    MidiEvent event;
};


enum Parameters {
    bRecord,
    bReset,
    seqStyle,
    seqStepsUp,
    seqStepsDown,
    transposeSemi,
    transposeKey,
    transposeKeyBase,
    groupNumber,
    actualGroup,
    parameterCount
};

enum PortGroups {
    gRecord,
    gSequencer,
    gTranspose,
    portGroupsCount
};

enum MachineState {
    init,
    play,
    recRequest,
    rec,
    playRequest,
    initRequest,
    stateCount
};

#endif
