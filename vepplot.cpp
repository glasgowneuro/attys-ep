/***************************************************************************
 *   Copyright (C) 2003 by Matthias H. Hennig, hennig@cn.stir.ac.uk        *
 *                 2023 by Bernd Porr, mail@berndporr.me.uk                *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#include "vepplot.h"

#include <QTimerEvent>

VEPPlot::VEPPlot(double *xData, double *yData, int length, QWidget *parent) :
    QwtPlot(parent),
    xData(xData),
    yData(yData)
{
  // Assign a title
  setTitle("EP");
  setAxisTitle(QwtPlot::xBottom, "Time/ms");
  setAxisTitle(QwtPlot::yLeft, "EP/V");

  dataCurve = new QwtPlotCurve("EP");
  dataCurve->setRawSamples(xData, yData, length);
  dataCurve->attach(this);
  dataCurve->setPen( QPen(Qt::cyan, 2) );
  dataCurve->setStyle(QwtPlotCurve::Steps);

  max = 0;
  min = 0;
  nDatapoints = length;
  updateCtr = 1;

  setAutoReplot(false);
}

void VEPPlot::setVEPLength(int length)
{
	nDatapoints = length;	
	dataCurve->setRawSamples(xData, yData, length);
}

void VEPPlot::startDisplay()
{
	currtimer=startTimer(150);
}

void VEPPlot::stopDisplay()
{
	killTimer(currtimer);
}

void VEPPlot::timerEvent(QTimerEvent *)
{
	updateCtr--;
	if (updateCtr==0) {
		min = INT_MAX;
		max = INT_MIN;
		for(int i=0;i<nDatapoints;i++) {
			float y = yData[i];
			if (y>max) max = y;
			if (y<min) min = y;
		}
		double d = max - min;
		setAxisScale(QwtPlot::yLeft,min-d/10,max+d/10);
		updateCtr = 10;
	}

  replot();
}
