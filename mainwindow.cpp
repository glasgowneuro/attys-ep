/***************************************************************************
 *   Copyright (C) 2003 by Matthias H. Hennig                              *
 *             (C) 2013-2022 by Bernd Porr                                 *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#include "attys-ep.h"
#include "AttysComm.h"
#include "AttysScan.h"
#include "mainwindow.h"

#include <QApplication>
#include <QSplashScreen>
#include <QDesktopServices>
#include <QUrl>
#include <QFile>
#include <QMenu>
#include <QAction>
#include <QMenuBar>


MainWindow::MainWindow()
	: attys_ep(new Attys_ep(this))
{
	setCentralWidget(attys_ep);

	setStyleSheet(stylesheet);
	setAutoFillBackground( true );

	menuBar()->setStyleSheet(menuStylesheet);

	QMenu *attysEpMenu = menuBar()->addMenu(tr("&attys-ep"));
	QAction *quitAct = new QAction(tr("&Exit"), this);
	attysEpMenu->addAction(quitAct);
	connect(quitAct,&QAction::triggered,attys_ep,&QCoreApplication::quit);

	QMenu *epMenu = menuBar()->addMenu(tr("&EP"));
	QAction *showEpAct = new QAction(tr("Show stimulus"), this);
	connect(showEpAct,&QAction::triggered,attys_ep->vepStimulus,&Stimulus::show);
	epMenu->addAction(showEpAct);
	QAction *saveEpAct = new QAction(tr("&Save current EP"), this);
	connect(saveEpAct,&QAction::triggered,attys_ep,&Attys_ep::slotSaveVEP);
	epMenu->addAction(saveEpAct);
	
	QMenu *eegMenu = menuBar()->addMenu(tr("&Rec"));
	enableRecordEEGAct = new QAction(tr("&Start continous recording"), this);
	eegMenu->addAction(enableRecordEEGAct);
	connect(enableRecordEEGAct,&QAction::triggered,attys_ep,&Attys_ep::slotSaveData);
	disableRecordEEGAct = new QAction(tr("&Stop continous recording"), this);
	eegMenu->addAction(disableRecordEEGAct);
	connect(disableRecordEEGAct,&QAction::triggered,attys_ep,&Attys_ep::slotStopSavingData);
	
	QMenu *helpMenu = menuBar()->addMenu(tr("&Help"));
	QAction *runGithub = new QAction(tr("&github"), this);
	helpMenu->addAction(runGithub);
	connect(runGithub,&QAction::triggered,this,&MainWindow::slotGithub);
}

void MainWindow::slotGithub() {
	QDesktopServices::openUrl(QUrl("https://github.com/glasgowneuro/attys-ep")); 
}




int main(int argc, char **argv)
{
  QApplication app(argc, argv);

  QPixmap pixmap(":/attys.png");
  QSplashScreen* splash = new QSplashScreen(pixmap);
  splash->setFont(QFont("Helvetica", 10));
  splash->show();
  splash->showMessage("Scanning for paired devices");
  app.processEvents();

  // see if we have any Attys!
  int ret = attysScan.scan(1);
  
  // zero on success and non zero on failure
  if (ret) {
	  app.processEvents();
	  std::this_thread::sleep_for(std::chrono::milliseconds(5000));
	  delete splash;
	  return ret;
  }
        
  // none detected
  if (attysScan.getNAttysDevices()<1) {
	  printf("No Attys present or not paired.\n");
	  splash->showMessage("Cound not connect\nand/or no devices paired.");
	  app.processEvents();
	  std::this_thread::sleep_for(std::chrono::milliseconds(5000));
	  delete splash;
	  return -1;
  }

  MainWindow mainWindow;
  mainWindow.show();
  splash->finish(&mainWindow);

  return app.exec();
}
