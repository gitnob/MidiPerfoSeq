cmake_minimum_required(VERSION 3.0)

project(perfoseq)

#add_executable(perfoseq main.cpp)

add_subdirectory(dpf)
dpf_add_plugin(midiperfoseq
  TARGETS jack lv2
  FILES_DSP
      #plugins/midithrough/MidiThroughExamplePlugin.cpp
      plugins/MidiPerfoSeq/MidiPerfoSeq.cpp
)
target_include_directories(midiperfoseq PUBLIC plugins/MidiPerfoSeq/.)

#install(TARGETS perfoseq RUNTIME DESTINATION bin)
