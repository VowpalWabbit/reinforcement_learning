# Parser and reward calculator for joined binary schema v2 files

## Build:

from `external_parser`:

- mkdir build
- cd build
- cmake ..
- make -j $(nproc)

vw executable located at: `external_parser/build/vw_binary_parser/vowpalwabbit/vw`

## Run

`./vowpalwabbit/vw -d <file> --binary_parser [other vw args]`