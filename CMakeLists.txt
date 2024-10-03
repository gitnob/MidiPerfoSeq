cmake_minimum_required(VERSION 3.0)

project(perfoseq)

#add_executable(perfoseq main.cpp)

add_subdirectory(dpf)
dpf_add_plugin(midithrough
  TARGETS jack lv2
  FILES_DSP
      plugins/midithrough/MidiThroughExamplePlugin.cpp
)
target_include_directories(midithrough PUBLIC plugins/midithrough/.)

#install(TARGETS perfoseq RUNTIME DESTINATION bin)
