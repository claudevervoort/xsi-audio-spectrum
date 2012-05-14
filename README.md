xsi-audio-spectrum
==================

Softimage ICE Node to get the Spectrum from a Wave file

3 projects:

fft_wav_analyzer: static lib - for Wave Loading and FFT decomposition using Kiss FFT

fft_wav_main: application - very basic OpenGL visualization tool, mostly used for debugging (32 bits only)

wavespectrum_icenode: dll, Softimage XSI Plug-in: the actual custom ICE node using fft_wav_analyzer lib

The add-on is ready to be used, compiled 64 bits with Softimage 2012 Sub. Adv. Pack

There are video tutorial on the plugin on Vimeo https://vimeo.com/41955919 and on my own website: http://claudeonthe.net