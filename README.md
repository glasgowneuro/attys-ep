# attys-ep

Visually evoked potential app for [Attys](http://www.attys.tech)
for Windows/Linux

![alt tag](screenshot.png)

attys-ep performs averaging over repetitive stimuli. It
works on continous data such as visually evoked potentials and
also on discrete events such as spikes.

# Installation

## Linux

### Linux Ubuntu packages for Ubuntu LTS
Add these two repositories...
```
sudo add-apt-repository ppa:berndporr/attys
sudo add-apt-repository ppa:berndporr/dsp
```
...and then select `attys-ep` in your favourite package manager. This will then install also
the other required packages.

### Compilation from source

You need the following libraries to compile and run the program:

- attys-comm (https://github.com/glasgowneuro/attys-comm): `apt-get install attyscomm-dev`
- Qt5 / Qwt standard UBUNTU packages: `apt-get install qt5-default libqwt-headers libqwt-qt5-dev`
- IIR filter library (https://github.com/berndporr/iir1): `apt-get install iir1-dev`

Run `qmake`, which generates a Makefile and `make` to compile it.

# Running attys-ep

![alt tag](setup.jpg)

Just type: `attys-ep` or `./attys-ep`


# Credits

attys-ep is based in part on the work of the Qwt project (http://qwt.sf.net).
