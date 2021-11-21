#ifndef _AudioBeep
#define _AudioBeep

#include <QtMultimedia/QAudioFormat>
#include <QtMultimedia/QAudioDeviceInfo>
#include <QtMultimedia/QAudioOutput>
#include <QDebug>
#include <QBuffer>

class AudioBeep : public QObject {

	Q_OBJECT

public:
	AudioBeep(QObject *w,	// w is the parent widget
		  float beepDuration = 0.2,	// duration in seconds
		  float beepFreq = 1000,	// beep frequency in Hz
		  float volume = 1.0);		// volume of the tone, from 0 to 1

	void play();	// plays sine wave asynchronously

private:
	const unsigned int sampleRate = 48000;	// sample rate

	QObject* qparent;
	QByteArray byteBuffer;
	QAudioFormat audioFormat;
};

#endif
