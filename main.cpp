/***************************************************************************
 *   Copyright (C) 2003 by Matthias H. Hennig                              *
 *             (C) 2013-2017 by Bernd Porr                                 *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#include "attys-ep.h"
#include "AttysComm.h"
#include "AttysScan.h"

#include <QApplication>

int main(int argc, char **argv)
{
  QApplication app(argc, argv);

  // see if we have any Attys!
  int ret = attysScan.scan(NULL,1);
  
  // zero on success and non zero on failure
  if (ret) {
	  return ret;
  }
        
  // none detected
  if (attysScan.nAttysDevices<1) {
	  printf("No Attys present or not paired.\n");
	  return -1;
  }

  MainWindow mainWindow;
  mainWindow.show();
  
  return app.exec();
}
