/***************************************************************************
 *   Copyright (C) 2003 by Matthias H. Hennig                              *
 *   hennig@cn.stir.ac.uk                                                  *
 *   Copyright (C) 2005-2018 by Bernd Porr                                      *
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

#include "vepplot.h"
#include "dataplot.h"
#include "stim.h"
#include <Iir.h>

// maximal length of the VEP (for memory alloctaion) in samples
#define MAX_VEP_LENGTH 5000

// in ms
#define DEFAULT_SWEEP_LENGTH 500

#define NOTCH_F 50 // filter out 50Hz noise
#define IIRORDER 2


class MainWindow : public QWidget
{
  Q_OBJECT
    
  // show the raw serai data here
  DataPlot *RawDataPlot;
  // here the VEP will be shown
  VEPPlot *vepPlot;
  
  // length of the VEP, this is the length on one trial
  int vepLength;

  // sampling rate
  double sampling_rate;

  // sample index within one sweep
  int trialIndex;

  // boo, activate/deactivate the vep plot
  int vepOn;

  // count trials while recording
  int vepActTrial;
  
  // data
  double xData[MAX_VEP_LENGTH], yData[MAX_VEP_LENGTH];
  
  // VEP
  double timeData[MAX_VEP_LENGTH], vepSummedUpData[MAX_VEP_LENGTH], vepAveragedData[MAX_VEP_LENGTH];
  
  // time counter
  long int time;

  // mode
  // 0 = VEP
  // 1 = P300
  int mode = 0;

  // P300 oddball
  int oddballCtr = 10;

  // 50/60 Hz notch
  Iir::Butterworth::BandStop<IIRORDER>* iirnotch;

  // highpass
  Iir::Butterworth::HighPass<IIRORDER>* iirhp;

  QComboBox *vpChoices;
  QComboBox* notchFreq;
  QPushButton *runVEP;
  Stimulus *vepStimulus;

  QTimer *sweepTimer;

private slots:

  // actions:
  void slotClearVEP();
  void slotRunVEP();
  void slotSetVEPLength(double l);
  void slotSaveVEP();
  void slotSelectVEPType(int idx);
  void slotNewSweep();
  void slotSelectNotchFreq(int);

protected:

  // set notch frequency
  void setNotch(double f);

  // timer to plot the data
  virtual void timerEvent(QTimerEvent *e);

  // implements the callback via this interface
  struct AttysCallback : AttysCommListener {
	  // pointer to an instance of the parent class
	  MainWindow* mainwindow;
	  // new constructor
	  AttysCallback(MainWindow* _mainwindow) { mainwindow = _mainwindow; };
	  // is called from the Attys whenever a sample has arrived
	  virtual void hasSample(float ts,float *data) {
		  mainwindow->hasData(ts,data);
	  };
  };

  // callback called from the interface below
  void hasData(float,float *sample);

  // interface which is registered with the Attys
  AttysCallback* attysCallback;

  // inits everything
  void initData();

signals:
  // timer event that a new sweep begins
  void sweepStarted(int);

public:

  MainWindow( QWidget *parent=0 );
  ~MainWindow();

};

#endif
