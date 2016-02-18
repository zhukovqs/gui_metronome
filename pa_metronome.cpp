#include <QObject>
#include "pa_metronome.h"
#include <iostream>
#include <portaudio.h>
#include <math.h>
#include <vector>

#define SAMPLE_RATE (44100)

Metronome::Metronome(QObject *parent) : QObject(parent)
{
    pa_init();
    open(Pa_GetDefaultOutputDevice());
    std::vector<Beat> bar = { { 1000 },{ 500 },{ 500 },{ 500 } };
    setBar(bar);
}

/*=================================================================================*/

void Metronome::setBpm(int b)
{
    bpm = b;
}

void Metronome::setAddBpm(int add)
{
     addBpm = add;
}

void Metronome::setBar(std::vector<Beat> bar)   //set bar size and peat pattern
{
    this->bar = bar;
}

void Metronome::setBarLimit(int bar_lim)    //sets the number of bars, after which there is a addBpm
{
    barLimit = bar_lim;
}

int Metronome::getBpm()
{
    return bpm;
}

void Metronome::setBeatCount(int bC)
{
    beatCount = bC;
}

void Metronome::setBarCount(int bCount)
{
    barCount = bCount;
}



int Metronome::getBarLimit()    //gets the number of bars, after which there is a addBpm
{
    return barLimit;
}

int Metronome::getBarCount()    //gets the barCount
{
    return barCount;
}

int Metronome::getBeatCount()   //gets the beatCount
{
    return beatCount;
}



void Metronome::speedTr()
{

        if(barCount >= barLimit)
        {
            barCount = 0;
            bpm = bpm +addBpm;
        }

}

bool Metronome::isPlaying()     //asks the metronome on his condition
{
    return playing;
}

/*===============================================================================*/

bool Metronome::open(PaDeviceIndex index)
{
    PaError err;
    PaStreamParameters outputParameters;
    Metronome *metronome;
    outputParameters.device = index;
    if (outputParameters.device == paNoDevice)
    {
        return false;
    }

    outputParameters.channelCount = 2;
    outputParameters.sampleFormat = paFloat32;
    outputParameters.suggestedLatency = Pa_GetDeviceInfo(outputParameters.device)->defaultHighOutputLatency;
    outputParameters.hostApiSpecificStreamInfo = NULL;

    err = Pa_OpenStream(&stream,
        NULL,              /* No input. */
        &outputParameters, /* As above. */
        SAMPLE_RATE,
        256,               /* Frames per buffer. */
        paClipOff,         /* No out of range samples expected. */
        paCallback,
        this);

    return true;
}


bool Metronome::start()
{
    if (stream == 0)
        return false;

    playing = true;
    PaError err = Pa_StartStream(stream);
    return (err == paNoError);
}

bool Metronome::pa_init()
{
    PaError err = Pa_Initialize();
    if (err != paNoError) Pa_Terminate();
    return err;
}

bool Metronome::close()
{
    if (stream == 0)
        return false;

    PaError err = Pa_CloseStream(stream);
    stream = 0;

    return (err == paNoError);
}

bool Metronome::stop()
{
    if (stream == 0)
        return false;

    playing = false;
    PaError err = Pa_StopStream(stream);

    return (err == paNoError);
}


void Metronome::HandleError(PaError &err)
{
    Pa_Terminate();
    std::cout << "Произошла ошибка во время использования потока Portaudio" << std::endl;
    std::cout << "Номер ошибки: " << err << std::endl;
    std::cout << "Сообщение об ошибке: " << Pa_GetErrorText(err) << std::endl;
    exit(err);
}



int Metronome::paCallback	(const		void*						inputBuffer,
                                        void*                       outputBuffer,
                            unsigned	long						framesPerBuffer,
                            const		PaStreamCallbackTimeInfo*	timeInfo,
                                        PaStreamCallbackFlags       statusFlags,
                                        void*                       userData)

{
        Metronome *metronome = (Metronome*)userData;
        float sampleVal;
        const float pi = 3.14159265358;
        float *out = (float*)outputBuffer;
        float framesInMs = SAMPLE_RATE / 1000.0;			//количество кадров в одной мс. = 44.1
        float msCount = metronome->counter / framesInMs;	//счетчик мс
        double tick = framesInMs * 100;                     //длительность одного удара в кадрах
        double delayMs = (1000 * 60) / metronome->bpm;      //интервал в мс
        double interval = (framesInMs * delayMs);           //интервал между ударами в кадрах.


        for (unsigned int i = 0; i < framesPerBuffer; i++)
        {
            if (metronome->counter < tick)
            {
                int freq = metronome->bar[metronome->beatCount].frequencyTick;
                sampleVal = sinf(2.0 * pi * freq * metronome->counter / SAMPLE_RATE);

                *out++ = sampleVal;
                *out++ = sampleVal;
                metronome->counter++;
            }
            else
                if (metronome->counter < interval)
                {
                    *out++ = 0;
                    *out++ = 0;
                    metronome->counter++;
                }
                else
                {

                    metronome->counter = 0;
                    metronome->beatCount = (metronome->beatCount + 1) % metronome->bar.size();
                    emit metronome->beatChanged(metronome->beatCount);
                    emit metronome->bpmChanged(metronome->bpm);
                    emit metronome->barPlayed(metronome->barCount);

                    if(metronome->beatCount == 0)
                    {
                        metronome->barCount = metronome->barCount + 1;

                    }

                }
        }

        return 0;
    }

Metronome::~Metronome(void)
    {
       close();
       Pa_Terminate();
    }
