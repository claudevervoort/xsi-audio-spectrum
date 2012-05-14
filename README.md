xsi-audio-spectrum
==================

Softimage ICE Node to get the Spectrum from a Wave file

3 projects:

fft_wav_analyzer: static lib - for Wave Loading and FFT decomposition using Kiss FFT

fft_wav_main: application - very basic OpenGL visualization tool, mostly used for debugging (32 bits only)

wavespectrum_icenode: dll, Softimage XSI Plug-in: the actual custom ICE node using fft_wav_analyzer lib

The add-on is ready to be used, compiled 64 bits with Softimage 2012 Sub. Adv. Pack
