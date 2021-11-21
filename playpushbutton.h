#ifndef _QPlayButton
#define _QPlayButton

#include<QPushButton>
#include "audiobeep.h"

// let's create a window which just contains a button and it hosts the player
class QPlayButton : public QPushButton {
	Q_OBJECT
public:
	QPlayButton() : QPushButton() {
		connect(this, SIGNAL( clicked() ),
			this, SLOT( buttonPressed() ) );
		audiobeep = new AudioBeep(this);
	}

	~QPlayButton() {
		delete audiobeep;
	};

public slots:
	void buttonPressed() {
		audiobeep->play();
	}

private:
	AudioBeep* audiobeep;
};

#endif
