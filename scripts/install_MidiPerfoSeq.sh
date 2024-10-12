#!/bin/bash

cd $ZYNTHIAN_PLUGINS_SRC_DIR
if [ -d "MidiPerfoSeq" ]; then
	rm -rf "MidiPerfoSeq"
fi

git clone https://github.com/gitnob/MidiPerfoSeq.git
cd MidiPerfoSeq
# substitutions
make -j3
make install
cd ..

rm -rf "MidiPerfoSeq"
