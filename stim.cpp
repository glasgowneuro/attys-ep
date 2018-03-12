#include "stim.h"

void Stimulus::init() {
	int w = width();
	int h = height();

	if (pm1) delete pm1;
	pm1 = new QPixmap(w, h);

	if (pm2) delete pm2;
	pm2 = new QPixmap(w, h);

	if (pm3) delete pm3;
	pm3 = new QPixmap(w, h);

	QPainter paint1(pm1);
	QPainter paint2(pm2);
	QPainter paint3(pm3);

	int dx = w / 10;
	int dy = w / 10;

	int start_white = 0;
	paint1.fillRect(0, 0,
		w, h,
		QColor(0, 0, 0));
	paint2.fillRect(0, 0,
		w, h,
		QColor(0, 0, 0));
	paint3.fillRect(0, 0,
		w, h,
		QColor(192, 192, 255));

	for (int y = 0; y < h; y = y + dy) {
		int white = start_white;
		for (int x = 0; x < w; x = x + dx) {
			if (start_white) {
				paint3.fillRect(x, y,
					dx, dy,
					QColor(255, 255, 192));
			}
			if (white) {
				paint1.fillRect(x, y,
					dx, dy,
					QColor(255, 255, 255));
			}
			else {
				paint2.fillRect(x, y,
					dx, dy,
					QColor(255, 255, 255));
			}
			white = !white;
		}
		start_white = !start_white;
	}
}


void Stimulus::paintEvent(QPaintEvent *) {
	QPainter paint2(this);
	if (oddball) {
		paint2.drawPixmap(0, 0, *pm3);
		return;
	}
	if (inverted) {
		paint2.drawPixmap(0, 0, *pm1);
	}
	else {
		paint2.drawPixmap(0, 0, *pm2);
	}
}
