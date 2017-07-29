#ifndef STIM_H
#define STIM_H

#include <QWidget>
#include <QPainter>
#include <QPixmap>


class Stimulus : public QWidget {

         Q_OBJECT

public:

Stimulus(QWidget *parent = Q_NULLPTR,
		 Qt::WindowFlags f = Qt::WindowFlags()) :
QWidget(parent,f|Qt::Window) {
init();
}

void init();

void paintEvent( QPaintEvent * );

void resizeEvent(QResizeEvent *) {
init();
repaint();
}

public slots:
void slotInvert() {
inverted = !inverted;
repaint();
}

private:
  
  QPixmap* pm1 = NULL;
  QPixmap* pm2 = NULL;
  QPainter* paint1 = NULL;
  QPainter* paint2 = NULL;

  int inverted = 0;
};


#endif
