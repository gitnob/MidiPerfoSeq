#!/bin/bash

cd $ZYNTHIAN_PLUGINS_SRC_DIR
if [ -d "MidiPerfoSeq" ]; then
	rm -rf "MidiPerfoSeq"
fi

git clone --recursive https://github.com/gitnob/MidiPerfoSeq.git
cd MidiPerfoSeq
# substitutions
mkdir build
cd build
cmake ..
make -j3 midiperfoseq-lv2
cp -r bin/midiperfoseq.lv2 $ZYNTHIAN_PLUGINS_DIR/lv2
cd ../..

rm -rf "MidiPerfoSeq"
