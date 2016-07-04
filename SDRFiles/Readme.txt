This is a selection of .sdr files that cover all types of hardware supported by
the GUI.

The Demo.sdr file is as its name suggests, used for demonstration purposes, both
the transmit and receive sides work and all of the modes and filtering are
operational. The step size is large so that the interpolation oscillator is used
and so the sample signal may be tuned, but the third-method/zero-IF is not available.

The 13cms and 23cms .sdr files will be the basis of the real UWSDR hardware
description files.

The CCW-SDR-4+ is for the latest Cross Country Wireless SDR-4+ receiver. The
CCW-Sentry is for the Cross Country Wireless Sentry transceiver which covers HF
and low VHF bands.

The HackRF files are for the basic HackRF either as a transciever or as a
receiver, and for the same but with a Ham It Up RF converter in use to allow
for coverage of the HF bands.

The Skeleton.sdr is a copy of Demo.sdr with many comments included so that
anyone wishing to create or edit an .sdr file can understand the meaning of the
various entries.
