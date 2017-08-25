# attys-ep

Visually evoked potential app for [Attys](http://www.attys.tech)
for Linux

![alt tag](screenshot.png)

This program performs averaging over repetitive stimuli. It
works on continous data such as visually evoked potentials and
also on discrete events such as spikes.

# Installation

You need the following libraries to compile and run the program:

- AttysComm (https://github.com/glasgowneuro/AttysComm)
- Qt5 / Qwt (standard UBUNTU packages)
- IIR filter library (http://www.berndporr.me.uk/iir/)

Run "qmake", which should generate a Makefile and "make" to compile
everything.

# Running attys-ep

Make sure the Attys is paired with your Linux box.

Then just type: "./attys-ep"
