/***************************************************************************
 *   Copyright (C) 2003 by Matthias H. Hennig                              *
 *   hennig@cn.stir.ac.uk                                                  *
 *   Copyright (C) 2005-2017 by Bernd Porr                                      *
 *   mail@berndporr.me.uk                                                  *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#ifndef ATTYS_EP
#define ATTYS_EP

#include <QWidget>
#include <QPushButton>
#include <QTextEdit>
#include <QCheckBox>
#include <QComboBox>

#include "AttysComm.h"
#include <qwt_counter.h>
#include <qwt_plot_marker.h>

#include "psthplot.h"
#include "dataplot.h"
#include "stim.h"
#include <Iir.h>

// maximal length of the PSTH (for memory alloctaion) in samples
#define MAX_PSTH_LENGTH 5000

// in ms
#define DEFAULT_SWEEP_LENGTH 500

#define NOTCH_F 50 // filter out 50Hz noise
#define IIRORDER 2


class MainWindow : public QWidget
{
  Q_OBJECT
    
  // show the raw serai data here
  DataPlot *RawDataPlot;
  // here the PSTH will be shown
  PsthPlot *MyPsthPlot;
  
  // length of the PSTH, this is the length on one trial
  int psthLength;
  // bin width for the PSTH
  int psthBinw;
  // treshold for a spike
  double spikeThres;

  // sampling rate
  double sampling_rate;

  // sample index within one sweep
  int trialIndex;

  // boo, activate/deactivate the psth plot
  int psthOn;

  // count trials while recording
  int psthActTrial;
  
  // bool, set when a spike is detected and the activity has not
  // gone back to resting potential
  bool spikeDetected;
  
  // data
  double xData[MAX_PSTH_LENGTH], yData[MAX_PSTH_LENGTH];
  // PSTH, t is time, p is spike count, psth is spikes/sec
  double timeData[MAX_PSTH_LENGTH], spikeCountData[MAX_PSTH_LENGTH], psthData[MAX_PSTH_LENGTH];
  
  // serai file desc
  int usbFd;
  
  // time counter
  long int time;
  
  int linearAverage;

  Iir::Butterworth::BandStop<IIRORDER>* iirnotch;

  Iir::Butterworth::HighPass<IIRORDER>* iirhp;

  QComboBox *averagePsth;
  QwtCounter *cntBinw;
  QTextEdit *editSpikeT;
  QPushButton *triggerPsth;
  QwtPlotMarker *thresholdMarker;
  Stimulus *stimulus;

  QTimer *sweepTimer;

private slots:

  // actions:
  void slotClearPsth();
  void slotTriggerPsth();
  void slotSetPsthLength(double l);
  void slotSetPsthBinw(double b);
  void slotSetSpikeThres();
  void slotSavePsth();
  void slotAveragePsth(int idx);
  void slotNewSweep();

protected:

  /// timer to plot the data
  virtual void timerEvent(QTimerEvent *e);

  struct AttysCallback : AttysComm::CallbackInterface {
	  MainWindow* mainwindow;
	  AttysCallback(MainWindow* _mainwindow) { mainwindow = _mainwindow; };
	  virtual void hasSample(float ts,float *data) {
		  mainwindow->hasData(ts,data);
	  };
  };

  void hasData(float,float *sample);

  AttysCallback* attysCallback;

  void initData();

signals:
  void sweepStarted();

public:

  MainWindow( QWidget *parent=0 );
  ~MainWindow();

};

#endif
