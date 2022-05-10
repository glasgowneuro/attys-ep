/***************************************************************************
 *   Copyright (C) 2003 by Matthias H. Hennig                              *
 *   hennig@cn.stir.ac.uk                                                  *
 *   Copyright (C) 2005-2022 by Bernd Porr                                 *
 *   mail@berndporr.me.uk                                                  *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#include "attys-ep.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QLabel>
#include <QFileDialog>
#include <QTextStream>
#include <QComboBox>
#include <QTimer>
#include <QMessageBox>

#include "AttysComm.h"
#include "AttysScan.h"

MainWindow::MainWindow( QWidget *parent ) :
	QWidget(parent),
	vepOn(false),
	time(0) {

	audiobeep = new AudioBeep(this,0.5,1000,1);	// uses default parameters set in audiobeep.h

	if (strstr(attysScan.getAttysComm(0)->getAttysName(),"ATTYS2")) {
		attysScan.getAttysComm(0)->setAdc_samplingrate_index(AttysComm::ADC_RATE_500HZ);
	} else {
		attysScan.getAttysComm(0)->setAdc_samplingrate_index(AttysComm::ADC_RATE_250HZ);
	}
	sampling_rate = attysScan.getAttysComm(0)->getSamplingRateInHz();

	vepLength = DEFAULT_SWEEP_LENGTH / (1000 / sampling_rate);

	attysCallback = new AttysCallback(this);
	attysScan.getAttysComm(0)->registerCallback(attysCallback);

	// set the PGA to max gain
	attysScan.getAttysComm(0)->setAdc0_gain_index(AttysComm::ADC_GAIN_12);
	
	// 50Hz or 60Hz mains notch filter (see header)
	setNotch(NOTCH_F);

	// highpass
	iirhp.setup (sampling_rate, 0.5);

	initData();

	setStyleSheet("background-color:rgb(64,64,64);color: white;");
	setAutoFillBackground( true );

	QHBoxLayout *mainLayout = new QHBoxLayout( this );

	QVBoxLayout *controlLayout = new QVBoxLayout;

	mainLayout->addLayout(controlLayout);
	
	QVBoxLayout *plotLayout = new QVBoxLayout;

	rawFileNameLabel = new QLabel();
	plotLayout->addWidget(rawFileNameLabel);

	mainLayout->addLayout(plotLayout);

	mainLayout->setStretchFactor(controlLayout,1);
	mainLayout->setStretchFactor(plotLayout,4);
	
	// two plots
	RawDataPlot = new DataPlot(xData, yData, vepLength, 
				   attysScan.getAttysComm(0)->getADCFullScaleRange(0),
				   -attysScan.getAttysComm(0)->getADCFullScaleRange(0),
				   this);
	RawDataPlot->setMaximumSize(10000,300);
	RawDataPlot->setStyleSheet(styleSheetAll);
	plotLayout->addWidget(RawDataPlot);
	RawDataPlot->show();

	auto s = std::string("fs = ")+std::to_string(int(sampling_rate))+" Hz";
	plotLayout->addWidget(new QLabel(s.c_str()));
	plotLayout->addSpacing(20);

	vepPlot = new VEPPlot(timeData, vepAveragedData, vepLength, this);
	vepPlot->setMaximumSize(10000,300);
	vepPlot->setStyleSheet(styleSheetAll);
	plotLayout->addWidget(vepPlot);
	vepPlot->show();
	
	// vep functions
	QGroupBox   *VEPfunGroup  = new QGroupBox( this );
	VEPfunGroup->setStyleSheet(styleSheetGroupBox);
	QVBoxLayout *vepFunLayout = new QVBoxLayout;

	VEPfunGroup->setLayout(vepFunLayout);
	VEPfunGroup->setAlignment(Qt::AlignJustify);
	VEPfunGroup->setSizePolicy( QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed) );
	controlLayout->addWidget( VEPfunGroup );

	vpChoices = new QComboBox(VEPfunGroup);
	vpChoices->setStyleSheet(styleSheetCombo);
	vpChoices->addItem(tr("VEP"));
	vpChoices->addItem(tr("P300"));
	vepFunLayout->addWidget(vpChoices);
	connect( vpChoices, SIGNAL(currentIndexChanged(int)), SLOT(slotSelectVEPType(int)) );

	vepFunLayout->addWidget(new QLabel());
	notchFreq = new QComboBox(VEPfunGroup);
	notchFreq->setStyleSheet(styleSheetCombo);
        notchFreq->addItem(tr("50Hz bandstop"));
        notchFreq->addItem(tr("60Hz bandstop"));
        vepFunLayout->addWidget(notchFreq);
        connect(  notchFreq, SIGNAL(currentIndexChanged(int)), SLOT(slotSelectNotchFreq(int)) );

	vepFunLayout->addWidget(new QLabel());
	runVEP = new QPushButton(VEPfunGroup);
	runVEP->setStyleSheet(styleSheetRecButtonOff);
	runVEP->setText("EP start/stop");
	runVEP->setCheckable(true);
	vepFunLayout->addWidget(runVEP);
	connect(runVEP, SIGNAL(clicked()), SLOT(slotRunVEP()));

	beepCheckBox = new QCheckBox("Play start/stop sound");
	vepFunLayout->addWidget(beepCheckBox);
	beepCheckBox->setEnabled(audiobeep->isOK());

	clearVEP = new QPushButton(VEPfunGroup);
	clearVEP->setText("Clear EP");
	clearVEP->setStyleSheet(styleSheetButton);
	vepFunLayout->addWidget(clearVEP);
	connect(clearVEP, SIGNAL(clicked()), SLOT(slotClearVEP()));
	connect(clearVEP, SIGNAL(clicked()), SLOT(slotClear()));
	
	saveVEP = new QPushButton(VEPfunGroup);
	saveVEP->setText("Save EP");
	saveVEP->setStyleSheet(styleSheetButton);
	vepFunLayout->addWidget(saveVEP);
	connect(saveVEP, SIGNAL(clicked()), SLOT(slotSaveVEP()));
	int inpWidth = saveVEP->width()*3/2;

	vepFunLayout->addWidget(new QLabel());
	vepFunLayout->addWidget(new QLabel("Raw EEG data:"));

	// raw data functions
	savedata = new QPushButton(VEPfunGroup);
	savedata->setText("Saving EEG On");
	savedata->setStyleSheet(styleSheetButton);
	connect(savedata, SIGNAL(clicked()), this, SLOT(slotSaveData()));
	vepFunLayout->addWidget(savedata);

	// raw data functions
	cleardata = new QPushButton(VEPfunGroup);
	cleardata->setText("Saving EEG Off");
	cleardata->setStyleSheet(styleSheetButton);
	connect(cleardata, SIGNAL(clicked()), this, SLOT(slotClearData()));
	vepFunLayout->addWidget(cleardata);

	// VEP params
	QGroupBox   *vepCounterGroup = new QGroupBox( this );
	vepCounterGroup->setStyleSheet(styleSheetGroupBox);
	QVBoxLayout *vepCounterLayout = new QVBoxLayout;

	vepCounterGroup->setLayout(vepCounterLayout);
	vepCounterGroup->setSizePolicy( QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed) );
	vepCounterGroup->setAlignment(Qt::AlignJustify);
	vepCounterGroup->setSizePolicy( QSizePolicy(QSizePolicy::Fixed,
						    QSizePolicy::Fixed) );
	controlLayout->addWidget( vepCounterGroup );

	vepCounterLayout->addWidget(new QLabel("Sweep length", vepCounterGroup));

	cntSLength = new QwtCounter(vepCounterGroup);
	cntSLength->setNumButtons(1);
  	cntSLength->setIncSteps(QwtCounter::Button1, 10000);
	cntSLength->setRange(1, MAX_VEP_LENGTH);
	cntSLength->setValue(DEFAULT_SWEEP_LENGTH);
	cntSLength->setMaximumWidth(inpWidth);
	vepCounterLayout->addWidget(cntSLength);
	connect(cntSLength, 
		SIGNAL(valueChanged(double)), 
		SLOT(slotSetVEPLength(double)));
	
	vepCounterLayout->addWidget(new QLabel());

	vepCounterLayout->addWidget(new QLabel("Avg oddball interval", vepCounterGroup));
	oddballAverage = new QwtCounter(vepCounterGroup);
	oddballAverage->setNumButtons(1);
  	oddballAverage->setIncSteps(QwtCounter::Button1, 1000);
	oddballAverage->setRange(5, MAX_ODDBALL_AVERAGE);
	oddballAverage->setValue(DEFAULT_ODDBALL_AVERAGE);
	oddballAverage->setMaximumWidth(inpWidth);
	vepCounterLayout->addWidget(oddballAverage);
	connect(oddballAverage, 
		SIGNAL(valueChanged(double)), 
		SLOT(slotSetP300OddballAverage(double)));	

	vepCounterLayout->addWidget(new QLabel());

	vepCounterLayout->addWidget(new QLabel("Oddball +/- deviation", vepCounterGroup));
	oddballDev = new QwtCounter(vepCounterGroup);
	oddballDev->setNumButtons(1);
  	oddballDev->setIncSteps(QwtCounter::Button1, 1000);
	oddballDev->setRange(2, MAX_ODDBALL_DEV);
	oddballDev->setValue(DEFAULT_ODDBALL_DEV);
	oddballDev->setMaximumWidth(inpWidth);
	vepCounterLayout->addWidget(oddballDev);
	connect(oddballDev, 
		SIGNAL(valueChanged(double)), 
		SLOT(slotSetP300OddballDev(double)));	

	vepStimulus = new Stimulus(this);
	vepStimulus->setMinimumSize(300,300);
	vepStimulus->show();
	connect ( this,
		  SIGNAL(sweepStarted(bool)),
		  vepStimulus,
		  SLOT(slotInvert(bool)) );
	
	oddballAverage->setEnabled(mode == P300);
	oddballDev->setEnabled(mode == P300);

	// Generate timer event every 50ms
	startTimer(50);

	sweepTimer = new QTimer( this );
	sweepTimer->setTimerType(Qt::PreciseTimer);
        connect( sweepTimer, 
                 SIGNAL(timeout()),
                 this, 
                 SLOT(slotNewSweep()) );
        sweepTimer->start( DEFAULT_SWEEP_LENGTH );

	attysScan.getAttysComm(0)->start();
}

MainWindow::~MainWindow()
{
	sweepTimer->stop();
	attysScan.getAttysComm(0)->unregisterCallback();
	attysScan.getAttysComm(0)->quit();
	
	delete audiobeep;

	if (rawfile != NULL) {
		fclose(rawfile);
		rawfile=NULL;
	}
}

void MainWindow::initData() {
	//  Initialize data for plots
	for(int i=0; i<MAX_VEP_LENGTH; i++)
	{
		xData[i] = i/(double)sampling_rate*1000;
		yData[i] = 0;
		timeData[i] = i/(double)sampling_rate*1000;
		vepSummedUpData[i] = 0;
		vepAveragedData[i] = 0;
	}
}


void MainWindow::setNotch(double f) {
	iirnotch.setup(sampling_rate, f, NOTCH_BW);
}


void MainWindow::slotSelectNotchFreq(int f) {
	switch (f) {
	case 0:
		setNotch(50);
		break;
	case 1:
		setNotch(60);
		break;
	}
}


void MainWindow::slotSetP300OddballAverage(double l) {
	auto d = oddballDev->value();
	if (d > (l/2)) {
		oddballDev->setValue(floor(l/2));
	}
}

void MainWindow::slotSetP300OddballDev(double l) {
	auto a = oddballAverage->value();
	if (l > (a/2)) {
		oddballDev->setValue(floor(a/2));
	}
}



void MainWindow::slotSaveVEP()
{
	QString fileName = QFileDialog::getSaveFileName();
	if( !fileName.isNull() )
	{
		const char suffix[] = ".tsv";
		QFileInfo fileinfo(fileName);
		QFile file;
		if (fileinfo.completeSuffix().isEmpty()) {
			file.setFileName(fileName+suffix);
		} else {
			file.setFileName(fileName);
		}
		
		if( file.open(QIODevice::WriteOnly | QFile::Truncate) )
		{
			QTextStream out(&file);
			for(int i=0; i<vepLength; i++)
				out << ((double)i/sampling_rate) << "\t" << vepAveragedData[i] << "\n";
			
			file.close();
		}
		else
		{
			QString s = "Could not save to: "+fileName;
			QMessageBox msgBox;
			msgBox.setText(s);
			msgBox.setModal(true);
			msgBox.exec();
		}
	}
}



void MainWindow::slotSaveData()
{
	QString fileName = QFileDialog::getSaveFileName();
	if( !fileName.isNull() )
	{
		const char suffix[] = ".tsv";
		QFileInfo fileinfo(fileName);
		if (fileinfo.completeSuffix().isEmpty()) {
			rawfilename = fileName.toStdString() + suffix;;
		} else {
			rawfilename = fileName.toStdString();
		}
		rawFileNameLabel->setText(rawfilename.c_str());
	}
}

void MainWindow::slotClearData() {
	rawfilename = "";
	rawFileNameLabel->setText("");
}


void MainWindow::slotClearVEP()  // to clear data from VEP graph
{
	time = 0;
	trialIndex = vepLength;
	initData();
	vepActTrial = 0;
	vepPlot->replot();
}

void MainWindow::slotClear()	// to clear data from both graphs
{
	time = 0;
	trialIndex = vepLength;
	initData();
	vepActTrial = 0;
	vepPlot->replot();
	RawDataPlot->replot();	// to clear out raw data at same time as vep
	
}

void MainWindow::slotRunVEP()
{
	// toggle VEP recording
	if(!vepOn)
	{
		vepOn = true;
		vepPlot->startDisplay();
		trialIndex = vepLength;
		sweepStartFlag = false;
		oddballCtr = (int)(oddballAverage->value());
		// save raw data - open file command
		if (!rawfilename.empty()) {
			rawfile = fopen(rawfilename.c_str(),"wt");
			if (rawfile == NULL) {
				std::string s = "Could not open: "+rawfilename;
				rawFileNameLabel->setText(s.c_str());
			} else {
				std::string s = "RECORDING: "+rawfilename;
				rawFileNameLabel->setText(s.c_str());
			}
		}
		if (beepCheckBox->checkState())
		{
			audiobeep->play();
		}
		runVEP->setStyleSheet(styleSheetRecButtonOn);
	}
	else
	{
		vepOn = false;
		vepPlot->stopDisplay();
		vepActTrial = 0;
		rawfilename = "";
		rawFileNameLabel->setText(rawfilename.c_str());
		// close raw data file
		if (rawfile != NULL) {
			fclose(rawfile);
			rawfile=NULL;
		}
		if (beepCheckBox->checkState())
		{
			audiobeep->play();
		}
		runVEP->setStyleSheet(styleSheetRecButtonOff);
	}
	vpChoices->setDisabled(vepOn);
	notchFreq->setDisabled(vepOn);
	oddballDev->setDisabled(vepOn);
	oddballAverage->setDisabled(vepOn);
	savedata->setDisabled(vepOn);
	cntSLength->setDisabled(vepOn);
	
}

void MainWindow::slotSetVEPLength(double l)
{
	vepLength = (int)(l / (1000.0 / sampling_rate));
	if (vepLength > MAX_VEP_LENGTH) vepLength = MAX_VEP_LENGTH;
	initData();
	vepActTrial = 0;
	time = 0;
	trialIndex = vepLength;
	sweepTimer->setInterval((int)l);
	RawDataPlot->setVEPLength(vepLength);
	vepPlot->setVEPLength(vepLength);
	vepPlot->replot();
}

void MainWindow::slotSelectVEPType(int idx)
{
	mode = (Mode)idx;
	oddballAverage->setEnabled(mode == P300);
	oddballDev->setEnabled(mode == P300);
}


void MainWindow::slotNewSweep() {
	if (vepOn) {
		bool oddball = false;
		switch (mode) {
		case 0:
			oddball = false;
			trialIndex = 0;
			sweepStartFlag = true;
			break;
		case 1:
			int min = (int)(oddballAverage->value()) - ((int)(oddballDev->value()));
			int dev = (int)(oddballDev->value())*2;
			if (oddballCtr == 0) {
				oddballCtr = min + rand() / (RAND_MAX/dev);
				oddball = true;
				trialIndex = 0;
				sweepStartFlag = true;
			} else {
				oddball = false;
				oddballCtr--;
			}
		}
		emit sweepStarted(oddball);
	}
}

void MainWindow::timerEvent(QTimerEvent *) {
	repaint();
	RawDataPlot->replot();
}


void MainWindow::hasData(double,float *sample)
{
	// we take the 1st channel
	float yNew = sample[AttysComm::INDEX_Analogue_channel_1];
	float yNew2 = sample[AttysComm::INDEX_Analogue_channel_2];

	if (NULL != rawfile)
	{
		fprintf(rawfile,"%e\t%e\t%d\n", yNew, yNew2, (int)sweepStartFlag);
		sweepStartFlag = false;
	}

	// save data here

	// highpass filtering of the data
	yNew=iirhp.filter(yNew);

	// removing 50Hz notch
	yNew=iirnotch.filter(yNew);

	// plot the data
	RawDataPlot->setNewData(yNew);

	// buffer full
	if (!(trialIndex<vepLength)) return;

	// Are we recording?
	if( vepOn )
	{
		// average the VEP
		vepSummedUpData[trialIndex] += yNew;		
		vepAveragedData[trialIndex] = vepSummedUpData[trialIndex] / (time/vepLength + 1);
	}
	
	time++;
	trialIndex++;
}
