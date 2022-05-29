/***************************************************************************
 *   Copyright (C) 2003 by Matthias H. Hennig                              *
 *   hennig@cn.stir.ac.uk                                                  *
 *   Copyright (C) 2005-2022 by Bernd Porr                                 *
 *   mail@berndporr.me.uk                                                  *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/
class Attys_ep;

#ifndef ATTYS_EP
#define ATTYS_EP

#include <QWidget>
#include <QPushButton>
#include <QTextEdit>
#include <QCheckBox>
#include <QComboBox>
#include <QLabel>
#include <QAction>

#include "AttysComm.h"
#include <qwt_counter.h>
#include <qwt_plot_marker.h>

#include "vepplot.h"
#include "dataplot.h"
#include "mainwindow.h"
#include "stim.h"
#include <Iir.h>

#include "audiobeep.h"

// maximal length of the VEP (for memory alloctaion) in samples
#define MAX_VEP_LENGTH 5000

// bluetooth latency
#define DEFAULT_BT_LATENCY 90 //ms
#define MAX_BT_LATENCY 150 //ms

// in ms
#define DEFAULT_SWEEP_LENGTH 750

// P300 Oddball timings
#define MAX_ODDBALL_AVERAGE 50
#define DEFAULT_ODDBALL_AVERAGE 10

#define MAX_ODDBALL_DEV 50
#define DEFAULT_ODDBALL_DEV 3

#define NOTCH_F 50 // filter out 50Hz noise
#define NOTCH_BW 2.5
#define IIRORDER 4

#define styleSheetAll "padding:0px;margin:0px;border:1px;"
#define styleSheetCombo "padding:0px;margin:0px;border:1px;margin-right:2px;font: 16px;"
#define styleSheetGroupBox "padding:1px;margin:0px;border:0px"
#define styleSheetButton "background-color: grey; border: none; outline: none; border-width: 1px; font: 16px; padding: 5px; color: white;"
#define styleSheetRecButtonOn "background-color: red; border: none; outline: none; border-width: 1px; font: 16px; padding: 5px; color: white;"
#define styleSheetRecButtonOff "background-color: grey; border: none; outline: none; border-width: 1px; font: 16px; padding: 5px; color: white;"

class Attys_ep : public QWidget
{
	Q_OBJECT
    
protected:

	// set notch frequency
	void setNotch(double f);

	// timer to plot the data
	virtual void timerEvent(QTimerEvent *e);

	// implements the callback via this interface
	struct AttysCallback : AttysCommListener {
		// pointer to an instance of the parent class
		Attys_ep* mainwindow;
		// new constructor
		AttysCallback(Attys_ep* _mainwindow) { mainwindow = _mainwindow; };
		// is called from the Attys whenever a sample has arrived
		virtual void hasSample(double ts,float *data) {
			mainwindow->hasData(ts,data);
		};
	};

	// callback called from the interface below
	void hasData(double,float *sample);

	// interface which is registered with the Attys
	AttysCallback* attysCallback;

	// inits everything
	void initData();

signals:
	// timer event that a new sweep begins
	void sweepStarted(bool);

public:

	Attys_ep( MainWindow *parent=0 );
	~Attys_ep();
  
	AudioBeep* audiobeep;	// pointer to the audiobeep class

	FILE* rawfile = NULL;
	std::string rawfilename;

	const QString filefilters = QString(tr("tab separated values (*.tsv)"));

	bool sweepStartFlag = false;

	// show the raw serai data here
	DataPlot *RawDataPlot;
	
	// here the VEP will be shown
	VEPPlot *vepPlot;
  
	// length of the VEP, this is the length on one trial
	int vepLength;

	// sampling rate
	double sampling_rate = 250;

	// sample index within one sweep
	int trialIndex = 0;

	// bool, activate/deactivate the vep plot
	bool vepOn = false;

	// count trials while recording
	int vepActTrial = 0;

	// double bt latency
	double btLatency = DEFAULT_BT_LATENCY;
  
	// data
	double xData[MAX_VEP_LENGTH] = {}, yData[MAX_VEP_LENGTH] = {};
  
	// VEP
	double timeData[MAX_VEP_LENGTH] = {};
	double vepSummedUpData[MAX_VEP_LENGTH] = {};
	double vepAveragedData[MAX_VEP_LENGTH] = {};
  
	// time counter
	long int time = 0;

	enum Mode {VEP=0, P300=1};

	Mode mode = VEP;

	// P300 oddball
	int oddballCtr = 10;

	// 50/60 Hz notch
	Iir::Butterworth::BandStop<IIRORDER> iirnotch;

	// highpass
	Iir::Butterworth::HighPass<IIRORDER> iirhp;

	QComboBox *vpChoices = nullptr;
	QComboBox* notchFreq = nullptr;
	QPushButton *runVEP = nullptr;
	Stimulus *vepStimulus = nullptr;
	QLabel* rawFileNameLabel = nullptr;
	QwtCounter* oddballDev = nullptr;
	QwtCounter* oddballAverage = nullptr;
	QTimer *sweepTimer = nullptr;
  	QCheckBox* beepCheckBox = nullptr;
	QPushButton *clearVEP = nullptr;
	QwtCounter *cntSLength = nullptr;
	QPushButton* cleardata = nullptr;
	QwtCounter *cntBTlatency = nullptr;

	const MainWindow* mainWindow;

	// actions:
	void slotClear();
	void slotRunVEP();
	void slotSetVEPLength(double l);
	void slotSetBTlatency(double l);
	void slotSaveVEP();
	void slotSaveData();	// to save raw data plot in file
	void slotClearData();	// to clear raw data plot in filename
	void slotSelectVEPType(int idx);
	void slotNewSweep();
	void slotSelectNotchFreq(int);
	void slotSetP300OddballAverage(double l);
	void slotSetP300OddballDev(double l);

};

#endif
