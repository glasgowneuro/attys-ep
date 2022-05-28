/***************************************************************************
 *   Copyright (C) 2003 by Matthias H. Hennig, hennig@cn.stir.ac.uk        *
 *   Copyright (C) 2005-2022 by Bernd Porr, mail@berndporr.me.uk           *
 *   Copyright (C) 2022 by Lucía Muñoz Bohollo                             *
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

Attys_ep::Attys_ep( MainWindow *parent ) :
	QWidget(parent),
	audiobeep(new AudioBeep(this,0.5,1000,1)),
	mainWindow(parent) {

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
	connect( vpChoices, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &Attys_ep::slotSelectVEPType);

	vepFunLayout->addWidget(new QLabel());
	notchFreq = new QComboBox(VEPfunGroup);
	notchFreq->setStyleSheet(styleSheetCombo);
        notchFreq->addItem(tr("50Hz bandstop"));
        notchFreq->addItem(tr("60Hz bandstop"));
        vepFunLayout->addWidget(notchFreq);
        connect(  notchFreq, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &Attys_ep::slotSelectNotchFreq);

	vepFunLayout->addWidget(new QLabel());
	runVEP = new QPushButton(VEPfunGroup);
	runVEP->setStyleSheet(styleSheetRecButtonOff);
	runVEP->setText("EP start/stop");
	runVEP->setCheckable(true);
	vepFunLayout->addWidget(runVEP);
	connect(runVEP, &QPushButton::clicked, this, &Attys_ep::slotRunVEP);

	beepCheckBox = new QCheckBox("Play start/stop sound");
	vepFunLayout->addWidget(beepCheckBox);
	beepCheckBox->setEnabled(audiobeep->isOK());
	
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
	vepCounterLayout->addWidget(cntSLength);
	connect(cntSLength, &QwtCounter::valueChanged, this, &Attys_ep::slotSetVEPLength);
	
	vepCounterLayout->addWidget(new QLabel());

	vepCounterLayout->addWidget(new QLabel("Avg oddball interval", vepCounterGroup));
	oddballAverage = new QwtCounter(vepCounterGroup);
	oddballAverage->setNumButtons(1);
  	oddballAverage->setIncSteps(QwtCounter::Button1, 1000);
	oddballAverage->setRange(5, MAX_ODDBALL_AVERAGE);
	oddballAverage->setValue(DEFAULT_ODDBALL_AVERAGE);
	vepCounterLayout->addWidget(oddballAverage);
	connect(oddballAverage,&QwtCounter::valueChanged,this, &Attys_ep::slotSetP300OddballAverage);

	vepCounterLayout->addWidget(new QLabel());

	vepCounterLayout->addWidget(new QLabel("Oddball +/- deviation", vepCounterGroup));
	oddballDev = new QwtCounter(vepCounterGroup);
	oddballDev->setNumButtons(1);
  	oddballDev->setIncSteps(QwtCounter::Button1, 1000);
	oddballDev->setRange(2, MAX_ODDBALL_DEV);
	oddballDev->setValue(DEFAULT_ODDBALL_DEV);
	vepCounterLayout->addWidget(oddballDev);
	connect(oddballDev, &QwtCounter::valueChanged, this, &Attys_ep::slotSetP300OddballDev);

	controlLayout->addWidget(new QLabel());

	clearVEP = new QPushButton();
	clearVEP->setText("Clear EP");
	clearVEP->setStyleSheet(styleSheetButton);
	controlLayout->addWidget(clearVEP);
	connect(clearVEP, &QPushButton::clicked, this,&Attys_ep::slotClearVEP);
	connect(clearVEP, &QPushButton::clicked, this,&Attys_ep::slotClear);
	
	vepStimulus = new Stimulus(this);
	vepStimulus->setMinimumSize(300,300);
	vepStimulus->show();
	connect ( this,
		  &Attys_ep::sweepStarted,
		  vepStimulus,
		  &Stimulus::slotInvert);
	
	oddballAverage->setEnabled(mode == P300);
	oddballDev->setEnabled(mode == P300);

	// Generate timer event every 50ms
	startTimer(50);

	sweepTimer = new QTimer( this );
	sweepTimer->setTimerType(Qt::PreciseTimer);
        connect( sweepTimer, 
                 &QTimer::timeout,
                 this, 
                 &Attys_ep::slotNewSweep );
        sweepTimer->start( DEFAULT_SWEEP_LENGTH );

	attysScan.getAttysComm(0)->start();
}

Attys_ep::~Attys_ep()
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

void Attys_ep::initData() {
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


void Attys_ep::setNotch(double f) {
	iirnotch.setup(sampling_rate, f, NOTCH_BW);
}


void Attys_ep::slotSelectNotchFreq(int f) {
	switch (f) {
	case 0:
		setNotch(50);
		break;
	case 1:
		setNotch(60);
		break;
	}
}


void Attys_ep::slotSetP300OddballAverage(double l) {
	auto d = oddballDev->value();
	if (d > (l/2)) {
		oddballDev->setValue(floor(l/2));
	}
}

void Attys_ep::slotSetP300OddballDev(double l) {
	auto a = oddballAverage->value();
	if (l > (a/2)) {
		oddballDev->setValue(floor(a/2));
	}
}



void Attys_ep::slotSaveVEP()
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



void Attys_ep::slotSaveData()
{
	if (NULL != rawfile) return;
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
		rawFileNameLabel->setText(("Ready to record: "+rawfilename).c_str());
	}
}

void Attys_ep::slotClearData() {
	if (NULL != rawfile) return;
	rawfilename = "";
	rawFileNameLabel->setText("");
}


void Attys_ep::slotClearVEP()  // to clear data from VEP graph
{
	time = 0;
	trialIndex = vepLength;
	initData();
	vepActTrial = 0;
	vepPlot->replot();
}

void Attys_ep::slotClear()	// to clear data from both graphs
{
	time = 0;
	trialIndex = vepLength;
	initData();
	vepActTrial = 0;
	vepPlot->replot();
	RawDataPlot->replot();	// to clear out raw data at same time as vep
	
}

void Attys_ep::slotRunVEP()
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
	cntSLength->setDisabled(vepOn);
	mainWindow->enableRecordEEGAct->setDisabled(vepOn);
	mainWindow->disableRecordEEGAct->setDisabled(vepOn);
}

void Attys_ep::slotSetVEPLength(double l)
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

void Attys_ep::slotSelectVEPType(int idx)
{
	mode = (Mode)idx;
	oddballAverage->setEnabled(mode == P300);
	oddballDev->setEnabled(mode == P300);
}


void Attys_ep::slotNewSweep() {
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

void Attys_ep::timerEvent(QTimerEvent *) {
	repaint();
	RawDataPlot->replot();
}


void Attys_ep::hasData(double,float *sample)
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
