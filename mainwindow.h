class MainWindow;
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include<QMainWindow>

#include "attys-ep.h"

const char stylesheet[] = "background-color:rgb(64,64,64);color: white;";

const char menuStylesheet[] =
"QMenu::item{ background-color: rgb(64, 64, 64); color: white; }"
"QMenu::item:selected{ background-color: rgb(0, 64, 64); white; }";

class MainWindow : public QMainWindow
{
	Q_OBJECT
	
public:
	MainWindow();

	QAction *enableRecordEEGAct;
	QAction *disableRecordEEGAct;

private:
	Attys_ep* attys_ep;
	
	void slotGithub();
};

#endif
