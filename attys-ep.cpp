/***************************************************************************
 *   Copyright (C) 2003 by Matthias H. Hennig                              *
 *   hennig@cn.stir.ac.uk                                                  *
 *   Copyright (C) 2005-2018 by Bernd Porr                                 *
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

#include "AttysComm.h"
#include "AttysScan.h"

MainWindow::MainWindow( QWidget *parent ) :
    QWidget(parent),
    vepOn(0),
    time(0) {

	attysScan.attysComm[0]->setAdc_samplingrate_index(AttysComm::ADC_RATE_250HZ);
	sampling_rate = attysScan.attysComm[0]->getSamplingRateInHz();

	vepLength = DEFAULT_SWEEP_LENGTH / (1000 / sampling_rate);

	attysCallback = new AttysCallback(this);
	attysScan.attysComm[0]->registerCallback(attysCallback);

	// set the PGA to max gain
	attysScan.attysComm[0]->setAdc0_gain_index(AttysComm::ADC_GAIN_12);
	
	// 50Hz or 60Hz mains notch filter (see header)
	iirnotch = new Iir::Butterworth::BandStop<IIRORDER>;
	assert( iirnotch != NULL );
	iirnotch->setup (IIRORDER, sampling_rate, NOTCH_F, 2.5);

	// highpass
	iirhp = new Iir::Butterworth::HighPass<2>;
	assert( iirhp != NULL );
	iirhp->setup (IIRORDER, sampling_rate, 0.5);

	initData();

	char styleSheet[] = "padding:0px;margin:0px;border:0px;";
	char styleSheetCombo[] = "padding:0px;margin:0px;border:0px;margin-right:2px;font: 16px";
	char styleSheetGroupBox[] = "padding:1px;margin:0px;border:0px";
	char styleSheetButton[] = "background-color: rgb(224, 224, 224); border: none; outline: none; border-width: 0px; font: 16px; padding: 5px;";
	QHBoxLayout *mainLayout = new QHBoxLayout( this );

	QVBoxLayout *controlLayout = new QVBoxLayout;

	mainLayout->addLayout(controlLayout);
	
	QVBoxLayout *plotLayout = new QVBoxLayout;

	mainLayout->addLayout(plotLayout);
	
	mainLayout->setStretchFactor(controlLayout,1);
	mainLayout->setStretchFactor(plotLayout,4);
	
	// two plots
	RawDataPlot = new DataPlot(xData, yData, vepLength, 
				   attysScan.attysComm[0]->getADCFullScaleRange(0),
				   -attysScan.attysComm[0]->getADCFullScaleRange(0),
				   this);
	RawDataPlot->setMaximumSize(10000,300);
	RawDataPlot->setStyleSheet(styleSheet);
	plotLayout->addWidget(RawDataPlot);
	RawDataPlot->show();

	plotLayout->addSpacing(20);


	vepPlot = new VEPPlot(timeData, vepAveragedData, vepLength, this);
	vepPlot->setMaximumSize(10000,300);
	vepPlot->setStyleSheet(styleSheet);
	plotLayout->addWidget(vepPlot);
	vepPlot->show();
	
	/*---- Buttons ----*/

	// psth functions
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

	runVEP = new QPushButton(VEPfunGroup);
	runVEP->setStyleSheet(styleSheetButton);
	runVEP->setText("VEP on/off");
	runVEP->setCheckable(true);
	vepFunLayout->addWidget(runVEP);
	connect(runVEP, SIGNAL(clicked()), SLOT(slotRunVEP()));

	QPushButton *clearVEP = new QPushButton(VEPfunGroup);
	clearVEP->setText("clear data");
	clearVEP->setStyleSheet(styleSheetButton);
	vepFunLayout->addWidget(clearVEP);
	connect(clearVEP, SIGNAL(clicked()), SLOT(slotClearVEP()));
	
	QPushButton *saveVEP = new QPushButton(VEPfunGroup);
	saveVEP->setText("save data");
	saveVEP->setStyleSheet(styleSheetButton);
	vepFunLayout->addWidget(saveVEP);
	connect(saveVEP, SIGNAL(clicked()), SLOT(slotSaveVEP()));
	int inpWidth = saveVEP->width()*3/2;

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

	QLabel *vepLengthLabel = new QLabel("Sweep length", vepCounterGroup);
	vepCounterLayout->addWidget(vepLengthLabel);

	QwtCounter *cntSLength = new QwtCounter(vepCounterGroup);
	cntSLength->setNumButtons(1);
  	cntSLength->setIncSteps(QwtCounter::Button1, 10000);
	cntSLength->setRange(1, MAX_VEP_LENGTH);
	cntSLength->setValue(DEFAULT_SWEEP_LENGTH);
	cntSLength->setMaximumWidth(inpWidth);
	vepCounterLayout->addWidget(cntSLength);
	connect(cntSLength, 
		SIGNAL(valueChanged(double)), 
		SLOT(slotSetVEPLength(double)));
	
	vepStimulus = new Stimulus(this);
	vepStimulus->setMinimumSize(300,300);
	vepStimulus->show();
	connect ( this,
		  SIGNAL(sweepStarted(int)),
		  vepStimulus,
		  SLOT(slotInvert(int)) );
	
	// Generate timer event every 50ms
	startTimer(50);

	sweepTimer = new QTimer( this );
	sweepTimer->setTimerType(Qt::PreciseTimer);
        connect( sweepTimer, 
                 SIGNAL(timeout()),
                 this, 
                 SLOT(slotNewSweep()) );
        sweepTimer->start( DEFAULT_SWEEP_LENGTH );

	attysScan.attysComm[0]->start();
}

MainWindow::~MainWindow()
{
	sweepTimer->stop();
	attysScan.attysComm[0]->unregisterCallback();
	attysScan.attysComm[0]->quit();
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

void MainWindow::slotSaveVEP()
{
	QString fileName = QFileDialog::getSaveFileName();
	
	if( !fileName.isNull() )
	{
		QFile file(fileName);
		
		if( file.open(QIODevice::WriteOnly | QFile::Truncate) )
		{
			QTextStream out(&file);
			
			for(int i=0; i<vepLength; i++)
				out << timeData[i] << "\t" << vepAveragedData[i] << "\n";
			
			file.close();
		}
		else
		{
			// TODO: warning box
		}
	}
}

void MainWindow::slotClearVEP()
{
	time = 0;
	trialIndex = 0;
	initData();
	vepActTrial = 0;
	vepPlot->replot();
}

void MainWindow::slotRunVEP()
{
	// toggle VEP recording
	if(vepOn == 0)
	{
		vepOn = 1;
		vepPlot->startDisplay();
		trialIndex = 0;
	}
	else
	{
		vepOn = 0;
		vepPlot->stopDisplay();
		vepActTrial = 0;
	}
}

void MainWindow::slotSetVEPLength(double l)
{
	vepLength = (int)(l / (1000.0 / sampling_rate));
	if (vepLength > MAX_VEP_LENGTH) vepLength = MAX_VEP_LENGTH;
	initData();
	vepActTrial = 0;
	time = 0;
	trialIndex = 0;
	sweepTimer->setInterval((int)l);
	RawDataPlot->setVEPLength(vepLength);
	vepPlot->setVEPLength(vepLength);
}

void MainWindow::slotSelectVEPType(int idx)
{
	mode = idx;
}


void MainWindow::slotNewSweep() {
	int oddball = 0;
	if (vepOn) {
		switch (mode) {
		case 0:
			oddball = 0;
			trialIndex = 0;
			break;
		case 1:
			if (oddballCtr == 0) {
				oddballCtr = 7 + rand() / (RAND_MAX/6);
				oddball = 1;
				trialIndex = 0;
			} else {
				oddball = 0;
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


void MainWindow::hasData(float,float *sample)
{
	// we take the 1st channel
	float yNew = sample[AttysComm::INDEX_Analogue_channel_1];

	// highpass filtering of the data
	yNew=iirhp->filter(yNew);

	// removing 50Hz notch
	yNew=iirnotch->filter(yNew);

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
