xsi-audio-spectrum
==================

What is it?
-----------

Softimage ICE Node to get the Spectrum from a Wave file. Demo: https://vimeo.com/41955919

Limitations
-----------

Loads all Wave file in memory.
Very Limited testing; let me know if it fails, send the scene along: claude.vervoort at gmail dot com.

What's in it?
-------------

3 projects:

* fft_wav_analyzer: static lib - for Wave Loading and FFT decomposition using Kiss FFT

* fft_wav_main: application - very basic OpenGL visualization tool, mostly used for debugging (32 bits only)

* wavespectrum_icenode: dll, Softimage XSI Plug-in: the actual custom ICE node using fft_wav_analyzer lib

How to use it
--------------

The add-on is ready to be used, compiled 64 bits with Softimage 2012 Sub. Adv. Pack. Just add it to Softimage and 
you should get a new Custom ICE Node.


Beware/Disclaimer
-----------------

I would not qualify myself as a C++ expert or a Softimage SDK wizard. Well wizard maybe in the sense that I got it to work somehow :)


How-to?
-------
There are video tutorial on the plugin on Vimeo https://vimeo.com/41955919 and on my own website: http://claudeonthe.net


Feedback
--------

Let me know if anything I've used is not the 'proper'/'best' way to do it. I love to learn!


Rights?
------

Feel free to use/abuse/transform. No restriction. If you happen to really use it, I'd love to know though! 