These are the remains of the DSP and cross platform GUI for the defunct uWSDR project. It's aim was to design and build an SDR that would cover individual bands from 6m upwards. The front end and control/DAC/ADC would be on seperate boards connected together. The control board design would be the same  for each band with only the RF board changing.

The project failed due to a lack of time on behalf of the volunteers. This was in the 2005-2008 time frame. Progress was made, signals were received on a jury-rigged system along with this GUI.

The GUI itself worked well, and supported from than the uWSDR. Each supported radio had a definition file associated with it, and the original SoftRock TXRX was supported and worked.

The DSP core was based on DttSP which was reworked into C++ and modified/simplified. The main change was the addition of third-methed (Weaver) modulation/demodulation for SSB and CW. The advantage of this being that any DC appeared in the middle of the passband and not at 0 Hz which made it much less noticaable. It also meant that any mixer imbalance wouldn't lead to a sideband but would instead appear within your own transmission bandwidth.

I hope someone finds the code useful.
