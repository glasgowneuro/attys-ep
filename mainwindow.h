#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include<QMainWindow>

#include "attys-ep.h"

class MainWindow : public QMainWindow
{
	Q_OBJECT
	
public:
	MainWindow();
	
private:
	Attys_ep* attys_ep;
	
	void slotGithub();
};

#endif
