#!/usr/bin/python3

# (C) 2022 Bernd Porr
# (C) Lucia Muñoz Bohollo
# GPL V3

import matplotlib.pyplot as plt
import numpy as np
import scipy.signal as signal
import sys


class Evoked_potentials:
    Fs = 500
    HPfc = 0.5
        
    def __init__(self,filename):
        """
        Loads the P300 or VEP of one Participant.
        """
        self.data = np.loadtxt(filename)
        self.t = np.linspace(0,len(self.data)/self.Fs,len(self.data))
        self.eeg = self.data[:,0]
        self.oddball_flags = self.data[:,2]
        self.oddball_samples = np.argwhere(self.oddball_flags > 0.5)
        self.initial_samples_to_ignore = 0
        self.__filter_data()
        
    def __filter_data(self):
        # Remove DC
        bHigh,aHigh = signal.butter(2,0.25/self.Fs*2,'high')
        self.eeg = signal.lfilter(bHigh,aHigh,self.eeg);
        # for VEP
        self.initial_samples_to_ignore = int(self.Fs / self.HPfc) * 3

        # Remove 50Hz noise
        b50,a50 = signal.butter(4,[48/self.Fs*2,52/self.Fs*2],'stop')
        self.eeg = signal.lfilter(b50,a50,self.eeg);

        # Remove 150Hz interference
        b100,a100 = signal.butter(4,[98/self.Fs*2,102/self.Fs*2],'stop')
        self.eeg = signal.lfilter(b100,a100,self.eeg);

        
    def get_averaged_ep(self):
        """
        Calculates the evoked potential usign the oddball samples and
        averages over them.
        Returns: timestamps,vep
        """
        mean, stddev, tmin, tmax = self.get_stimulus_stats()
        self.navg = int(self.Fs * mean)
        self.avg = np.zeros(self.navg)
        
        n = 0
        for [ob] in self.oddball_samples:
            if ((ob+self.navg) < len(self.eeg)) and (ob > self.initial_samples_to_ignore):
                self.avg = self.avg + self.eeg[int(ob):int(ob+self.navg)]
                n = n + 1
                
        avg = self.avg / n
        avg = avg*1e6
        
        time = np.linspace(0,self.navg/self.Fs,self.navg) * 1000
        return time,avg

    def get_stimulus_stats(self):
        dt = np.array([])
        for i in range(len(self.oddball_samples)-1):
            dt = np.append(dt,(self.oddball_samples[i+1] - self.oddball_samples[i])/self.Fs)
        return np.mean(dt),np.std(dt),int(np.round(min(dt))),int(np.round(max(dt)))

def plotEP(filename):
    title = filename
    plt.title(title)
    evoked_potential = Evoked_potentials(filename)
    mean, stddev, tmin, tmax = evoked_potential.get_stimulus_stats()
    print("{}, stimulus stats: average = {}s, standard deviation = {}s, min = {}s, max = {}s."
          .format(title,mean,stddev,tmin,tmax))
    eeg = evoked_potential.eeg
    ign = evoked_potential.initial_samples_to_ignore
    eeg[0:ign] = 0
    ax = plt.subplot(211)
    ax.plot(evoked_potential.t,eeg*1e6)
    ax.set_xlabel('Time (sec)')
    ax.set_ylabel('Amplitude ($\mu$V)')
    ax.set_title("Raw filtered data")

    t, avg = evoked_potential.get_averaged_ep()
    ax = plt.subplot(212)
    ax.plot(t,avg)
    ax.set_xlabel("Time (ms)")
    ax.set_ylabel("Amplitude ($\mu$V)")
    ax.set_title("Evoked potential")


filename = "fake_vep_eeg_with_ldr.tsv"
if len(sys.argv) > 1:
    participant = int(sys.argv[1])
else:
    print("You can specify a filename with: {} <filname>"
          .format(sys.argv[0]))

plotEP(filename)

plt.show()
