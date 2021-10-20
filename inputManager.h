#ifndef INPUTMANAGER_H
#define INPUTMANAGER_H

#include <QObject>
#include <QDebug>

#include <iostream>
#include <queue>
#include <mutex>

#include "boost/circular_buffer.hpp"


#define N_SAMPLES 128

class InputManager: public QObject
{

public:
    explicit InputManager(const int samples, QObject *parent = NULL):
    samples(samples)
    {
        this->inputs = 4;
        for(int i = 0; i < this->inputs; i++)
            this->queue[i].resize(N_SAMPLES);
    }
    InputManager(QObject *parent = NULL)
    {
        this->inputs = 4;
        for(int i = 0; i < this->inputs; i++)
            this->queue[i].resize(N_SAMPLES);
    }

    ~InputManager(){}

    int getInputsNum()
    {
        return this->inputs;
    }

    void addToMemory(std::vector<float> data)
    {
        for(int i = 0; i < this->inputs; i++)
        {
            auto first = data.begin() + i * N_SAMPLES;
            auto last  = data.begin() + i * N_SAMPLES  + N_SAMPLES;

            std::vector<float> serie(first, last);

            this->queue[i].push_back(serie);
        }
    }

    void addRawDataToMemory(std::vector<uint8_t> data)
    {
        this->mutex.lock();
        this->rawBuffer.insert(std::end(rawBuffer), std::begin(data), std::end(data));
        this->mutex.unlock();
    }

    std::vector<float> getData(int serie)
    {
        if(!this->queue[serie].empty())
        {
            auto begin = this->queue[serie].front().begin();
            auto end = this->queue[serie].front().end();

            std::vector<float> data(begin,end);
            this->queue[serie].pop_front();

            return data;
        }

        std::vector<float> empty_vector;
        return empty_vector;
    }

    std::vector<uint8_t> getRawData()
    {
        this->mutex.lock();
        return this->rawBuffer;
        this->mutex.unlock();
    }

    std::vector<uint8_t> requestRawData(uint32_t size)
    {
        this->mutex.lock();
        std::vector<uint8_t> data;
        if(this->rawBuffer.size() >= size)
        {
            data.insert(data.begin(),this->rawBuffer.begin(), this->rawBuffer.begin() + size);
            this->rawBuffer.erase(this->rawBuffer.begin(), this->rawBuffer.begin() + size);
        }
        this->mutex.unlock();
        return data;
    }

    void clearBuffer()
    {
        this->mutex.lock();
        if (this->rawBuffer.size() > 0)
        {
            this->rawBuffer.clear();
        }
        this->mutex.unlock();
    }

private:
    int inputs = 0;
    int samples = 0;
    boost::circular_buffer<std::vector<float>> queue[4];
    std::vector<uint8_t> rawBuffer;
    std::mutex mutex;
};

#endif // INPUTMANAGER_H
