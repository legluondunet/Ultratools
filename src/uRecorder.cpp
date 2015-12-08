/* Copyright (c) 2015 Quentin Bramas
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <cmath>
#include <QKeyEvent>

#include "uAudioManager.h"
#include "uMidiManager.h"
#include "uRecorder.h"
#include "uShowSentenceWydget.h"
#include "editorwindow.h"

Recorder::Recorder(ShowSentenceWidget * showSentenceWidget)
{
    _isRecording = false;
    _showSentenceWidget = showSentenceWidget;

    connect(&UInputManager::Instance,SIGNAL(keyPressEvent(QKeyEvent *, ulong)),this,SLOT(onKeyPressEvent(QKeyEvent *, ulong)));
    connect(&UInputManager::Instance,SIGNAL(keyReleaseEvent(QKeyEvent *, ulong)),this,SLOT(onKeyReleaseEvent(QKeyEvent *, ulong)));
#ifdef USE_MIDI
    connect(UMidiManager::getInstance(),SIGNAL(noteOnEvent(ulong,int)),this,SLOT(startNote(ulong,int)));
    connect(UMidiManager::getInstance(),SIGNAL(noteOffEvent(ulong,int)),this,SLOT(stopNote(ulong,int)));
#endif
}

int Recorder::getBeat(ulong time)
{
    if(UAudioManager::Instance.timestampToPosition(time)) {
        time -= _showSentenceWidget->getLyrics()->getGap();
        return std::lround(_showSentenceWidget->getLyrics()->timeToBeat(time));
    }
    return _showSentenceWidget->currentBeat();
}

void Recorder::onKeyPressEvent(QKeyEvent * event, ulong time)
{
    if(event->key() == Qt::Key_Space)
    {
        return;
    }

    startNote(time, 0);
}

void Recorder::startNote(ulong time, int pitch)
{
    if (!_isRecording)
    {
        return;
    }

    stopNote(time, -1);

    int beat = getBeat(time);
    _currentWord = Word(NULL,beat,1,pitch);
    _currentWord.setText("");
}

void Recorder::onKeyReleaseEvent(QKeyEvent * /*event*/, ulong time)
{
    stopNote(time, 0);

   // if(UInputManager::Instance.isKeyPressed(Qt::Key_Space))// QMessageBox::warning(NULL,"","lol");
}

void Recorder::stopNote(ulong time, int pitch)
{
    if (_currentWord.getText().isNull())
        return;

    // -1 is a valid pitch but it will never be generated by midi or keyboard input
    if (pitch != -1 && _currentWord.getPitch() != pitch)
        return;

    int beat = getBeat(time);

    if(beat - _currentWord.getTime()>1)
        _currentWord.setLength(beat - _currentWord.getTime());

    _showSentenceWidget->getLyrics()->addWord(_currentWord);
    _currentWord = Word();


}

void Recorder::start()
{
    _isRecording = true;
}
void Recorder::stop()
{
    _isRecording = false;
}

