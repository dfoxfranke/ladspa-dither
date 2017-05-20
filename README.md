# ladspa-dither

This is a [LADSPA](http://ladspa.org/) plugin for adding triangular
[dither](https://en.wikipedia.org/wiki/Dither#Digital_audio) to audio
samples. Dithering digital audio samples prior to reducing their
bit-depth helps eliminate bias introduced by rounding error and
enables the human brain to recover detail that would otherwise be
buried below the noise floor.

The plugin takes one control input, which should be set to the
number of bits that you plan to reduce its depth to.

The `ladspa.h` header file required for compilation is part of the
LADSPA SDK, available from [ladspa.org](http://ladspa.org) or as part
of most Linux distributions.