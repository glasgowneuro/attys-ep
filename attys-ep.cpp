/***************************************************************************
 *   Copyright (C) 2003 by Matthias H. Hennig                              *
 *   hennig@cn.stir.ac.uk                                                  *
 *   Copyright (C) 2005-2017 by Bernd Porr                                 *
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
#include <QPrinter>
#include <QPrintDialog>
#include <QFileDialog>
#include <QTextStream>
#include <QComboBox>
#include <QTimer>

#include "AttysComm.h"
#include "AttysScan.h"

#include <unistd.h>

MainWindow::MainWindow( QWidget *parent ) :
    QWidget(parent),
    psthBinw(20),
    spikeThres(1),
    psthOn(0),
    spikeDetected(false),
    time(0),
    linearAverage(0)
{

	attysScan.attysComm[0]->setAdc_samplingrate_index(AttysComm::ADC_RATE_250HZ);
	sampling_rate = attysScan.attysComm[0]->getSamplingRateInHz();

	psthLength = DEFAULT_SWEEP_LENGTH / (1000 / sampling_rate);

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

	QHBoxLayout *mainLayout = new QHBoxLayout( this );

	QVBoxLayout *controlLayout = new QVBoxLayout;

	mainLayout->addLayout(controlLayout);
	
	QVBoxLayout *plotLayout = new QVBoxLayout;
	//plotLayout->addStrut(400);
	mainLayout->addLayout(plotLayout);
	
	mainLayout->setStretchFactor(controlLayout,1);
	mainLayout->setStretchFactor(plotLayout,4);
	
	// two plots
	RawDataPlot = new DataPlot(xData, yData, psthLength, 
				   attysScan.attysComm[0]->getADCFullScaleRange(0),
				   -attysScan.attysComm[0]->getADCFullScaleRange(0),
				   this);
	RawDataPlot->setMaximumSize(10000,300);
	RawDataPlot->setStyleSheet(styleSheet);
	plotLayout->addWidget(RawDataPlot);
	RawDataPlot->show();

	plotLayout->addSpacing(20);
	
	MyPsthPlot = new PsthPlot(timeData, psthData, psthLength/psthBinw, this);
	MyPsthPlot->setMaximumSize(10000,300);
	MyPsthPlot->setStyleSheet(styleSheet);
	plotLayout->addWidget(MyPsthPlot);
	MyPsthPlot->show();
	
	/*---- Buttons ----*/

	// psth functions
	QGroupBox   *PSTHfunGroup  = new QGroupBox( "Actions", this );
	QVBoxLayout *PSTHfunLayout = new QVBoxLayout;

	PSTHfunGroup->setLayout(PSTHfunLayout);
	PSTHfunGroup->setAlignment(Qt::AlignJustify);
	PSTHfunGroup->setSizePolicy( QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed) );
	controlLayout->addWidget( PSTHfunGroup );

	averagePsth = new QComboBox(PSTHfunGroup);
	averagePsth->addItem(tr("VEP"));
	averagePsth->addItem(tr("PSTH"));
	PSTHfunLayout->addWidget(averagePsth);
	connect( averagePsth, SIGNAL(currentIndexChanged(int)), SLOT(slotAveragePsth(int)) );

	triggerPsth = new QPushButton(PSTHfunGroup);
	triggerPsth->setText("PSTH on");
	triggerPsth->setCheckable(true);
	PSTHfunLayout->addWidget(triggerPsth);
	connect(triggerPsth, SIGNAL(clicked()), SLOT(slotTriggerPsth()));

	QPushButton *clearPsth = new QPushButton(PSTHfunGroup);
	clearPsth->setText("clear data");
	PSTHfunLayout->addWidget(clearPsth);
	connect(clearPsth, SIGNAL(clicked()), SLOT(slotClearPsth()));
	
	QPushButton *savePsth = new QPushButton(PSTHfunGroup);
	savePsth->setText("save data");
	PSTHfunLayout->addWidget(savePsth);
	connect(savePsth, SIGNAL(clicked()), SLOT(slotSavePsth()));
	int inpWidth = savePsth->width()*3/2;

	// psth params
	QGroupBox   *PSTHcounterGroup = new QGroupBox( "Parameters", this );
	QVBoxLayout *PSTHcounterLayout = new QVBoxLayout;

	PSTHcounterGroup->setLayout(PSTHcounterLayout);
	PSTHcounterGroup->setSizePolicy( QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed) );
	PSTHcounterGroup->setAlignment(Qt::AlignJustify);
	PSTHcounterGroup->setSizePolicy( QSizePolicy(QSizePolicy::Fixed,
					       QSizePolicy::Fixed) );
	controlLayout->addWidget( PSTHcounterGroup );

	QLabel *psthLengthLabel = new QLabel("Sweep length", PSTHcounterGroup);
	PSTHcounterLayout->addWidget(psthLengthLabel);

	QwtCounter *cntSLength = new QwtCounter(PSTHcounterGroup);
	cntSLength->setNumButtons(2);
	cntSLength->setIncSteps(QwtCounter::Button1, 10);
	cntSLength->setIncSteps(QwtCounter::Button2, 100);
	cntSLength->setRange(1, MAX_PSTH_LENGTH);
	cntSLength->setValue(DEFAULT_SWEEP_LENGTH);
	cntSLength->setMaximumWidth(inpWidth);
	PSTHcounterLayout->addWidget(cntSLength);
	connect(cntSLength, 
		SIGNAL(valueChanged(double)), 
		SLOT(slotSetPsthLength(double)));
	
	QLabel *binwidthLabel = new QLabel("Binwidth", PSTHcounterGroup);
	PSTHcounterLayout->addWidget(binwidthLabel);
	
	cntBinw = new QwtCounter(PSTHcounterGroup);
	cntBinw->setNumButtons(2);
	cntBinw->setIncSteps(QwtCounter::Button1, 1);
	cntBinw->setIncSteps(QwtCounter::Button2, 10);
	cntBinw->setRange(1, 100);
	cntBinw->setValue(psthBinw);
	cntBinw->setMaximumWidth(inpWidth);
	PSTHcounterLayout->addWidget(cntBinw);
	connect(cntBinw, SIGNAL(valueChanged(double)), SLOT(slotSetPsthBinw(double)));
	
	QLabel *thresholdLabel = new QLabel("Spike Threshold", PSTHcounterGroup);
	PSTHcounterLayout->addWidget(thresholdLabel);
	
	editSpikeT = new QTextEdit("0");
	editSpikeT->setMaximumWidth(inpWidth);
	QFont editFont("Courier",14);
	QFontMetrics editMetrics(editFont);
	editSpikeT->setMaximumHeight ( editMetrics.height()*1.2 );
	editSpikeT->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	editSpikeT->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	editSpikeT->setFont(editFont);
	PSTHcounterLayout->addWidget(editSpikeT);
	connect(editSpikeT, SIGNAL(textChanged()), SLOT(slotSetSpikeThres()));
	
	thresholdMarker = new QwtPlotMarker();
	thresholdMarker->setValue(0,0);
	thresholdMarker->attach(RawDataPlot);
	thresholdMarker->setLineStyle(QwtPlotMarker::HLine);

	slotAveragePsth(0);

	stimulus = new Stimulus(this);
	stimulus->setMinimumSize(300,300);
	stimulus->show();
	connect ( this,
		  SIGNAL(sweepStarted()),
		  stimulus,
		  SLOT(slotInvert()) );
	
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
	for(int i=0; i<MAX_PSTH_LENGTH; i++)
	{
		xData[i] = i/(double)sampling_rate*1000;
		yData[i] = 0;
		timeData[i] = i/(double)sampling_rate*1000;
		spikeCountData[i] = 0;
		psthData[i] = 0;
	}
}

void MainWindow::slotSavePsth()
{
	QString fileName = QFileDialog::getSaveFileName();
	
	if( !fileName.isNull() )
	{
		QFile file(fileName);
		
		if( file.open(QIODevice::WriteOnly | QFile::Truncate) )
		{
			QTextStream out(&file);
			
			for(int i=0; i<psthLength/psthBinw; i++)
				out << timeData[i] << "\t" << psthData[i] << "\n";
			
			file.close();
		}
		else
		{
			// TODO: warning box
		}
	}
}

void MainWindow::slotClearPsth()
{
	time = 0;
	trialIndex = 0;
	initData();
	spikeDetected = false;
	psthActTrial = 0;
	MyPsthPlot->replot();
}

void MainWindow::slotTriggerPsth()
{
	if(psthOn == 0)
	{
		psthOn = 1;
		MyPsthPlot->startDisplay();
		trialIndex = 0;
	}
	else
	{
		MyPsthPlot->stopDisplay();
		psthOn = 0;
		psthActTrial = 0;
		spikeDetected = false;
	}
}

void MainWindow::slotSetPsthLength(double l)
{
	psthLength = (int)(l / (1000.0 / sampling_rate));
	if (psthLength > MAX_PSTH_LENGTH) psthLength = MAX_PSTH_LENGTH;
	initData();
	spikeDetected = false;
	psthActTrial = 0;
	time = 0;
	trialIndex = 0;
	sweepTimer->setInterval((int)l);
	RawDataPlot->setPsthLength(psthLength);
	MyPsthPlot->setPsthLength(psthLength/psthBinw);
}

void MainWindow::slotSetPsthBinw(double b)
{
	psthBinw = (int)b;
	initData();
	spikeDetected = false;
	psthActTrial = 0;
	time = 0;
	trialIndex = 0;
	MyPsthPlot->setPsthLength(psthLength/psthBinw);
}

void MainWindow::slotSetSpikeThres()
{
	QString t = editSpikeT->toPlainText();
	spikeThres = t.toFloat();
	thresholdMarker->setValue(0,spikeThres);
	printf("%lf\n",spikeThres);
	spikeDetected = false;
}

void MainWindow::slotAveragePsth(int idx)
{
	linearAverage = (idx == 0);
	if ( linearAverage )
	{
		cntBinw->setEnabled(false);
		editSpikeT->setEnabled(false);
		MyPsthPlot->setYaxisLabel("Averaged Data");
		MyPsthPlot->setAxisTitle(QwtPlot::yLeft, "average/V");
		MyPsthPlot->setTitle("VEP");
		triggerPsth->setText("Averaging on");
		psthBinw = 1;
		cntBinw->setValue(psthBinw);
	}
	else
	{
		cntBinw->setEnabled(true);
		editSpikeT->setEnabled(true);
		MyPsthPlot->setYaxisLabel("Spikes/s");
		MyPsthPlot->setAxisTitle(QwtPlot::yLeft, "Spikes/s");
		MyPsthPlot->setTitle("PSTH");
		triggerPsth->setText("PSTH on");
	}
}


void MainWindow::slotNewSweep() {
	trialIndex = 0;
	if (psthOn) {
		emit sweepStarted();
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

	yNew=iirhp->filter(yNew);

	yNew=iirnotch->filter(yNew);

	RawDataPlot->setNewData(yNew);

	// buffer full
	if (!(trialIndex<psthLength)) return;

	if( linearAverage && psthOn )
	{
		spikeCountData[trialIndex] += yNew;		
		psthData[trialIndex] = spikeCountData[trialIndex] / (time/psthLength + 1);
	}
	else if( !spikeDetected && yNew>spikeThres )
	{
		if(psthOn)
		{
			int psthIndex = trialIndex / psthBinw;
			
			spikeCountData[psthIndex] += 1;
			
			psthData[psthIndex] = ( spikeCountData[psthIndex]*1000 ) / 
				( psthBinw * (time/psthLength + 1) );
			
			spikeDetected = true;
		}
	}
	else if( yNew < spikeThres )
	{
		spikeDetected = false;
	}
    
	time++;
	trialIndex++;
}
