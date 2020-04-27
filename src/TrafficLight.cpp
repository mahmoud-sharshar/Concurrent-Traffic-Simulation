#include <iostream>
#include <random>
#include "TrafficLight.h"

/* Implementation of class "MessageQueue" */


template <typename T>
T MessageQueue<T>::receive()
{
   
    std::unique_lock<std::mutex> lock(mtx);
    _cond.wait(lock, [this] {return !_queue.empty();});
    T msg = std::move(_queue.back());
    _queue.pop_back();
    return msg;
}

template <typename T>
void MessageQueue<T>::send(T &&msg)
{
   
    std::lock_guard<std::mutex> lock(mtx);
  	_queue.push_back(std::move(msg));
  	_cond.notify_one();
}


/* Implementation of class "TrafficLight" */


TrafficLight::TrafficLight()
{
    _currentPhase = TrafficLightPhase::red;
}

void TrafficLight::waitForGreen()
{
    
    while(true)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    	TrafficLightPhase msg = messages.receive();
      	if(msg == TrafficLightPhase::green)
        {
          return;
        }
    }
    
}


TrafficLightPhase TrafficLight::getCurrentPhase()
{
    return _currentPhase;
}

void TrafficLight::setCurrentPhase(TrafficLightPhase newPhase)
{
  _currentPhase = newPhase;
}


void TrafficLight::simulate()
{
    
    threads.emplace_back(std::thread(&TrafficLight::cycleThroughPhases,this));
  
}

// virtual function which is executed in a thread
void TrafficLight::cycleThroughPhases()
{
   
   	double cycleDuration = 4000 + std::rand() % 2000; // duration of a single simulation cycle in ms
    std::chrono::time_point<std::chrono::system_clock> lastUpdate;

    // init stop watch
    lastUpdate = std::chrono::system_clock::now();
  	
  	while(true)
    {
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
      // compute time difference to stop watch
      long timeSinceLastUpdate = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - lastUpdate).count();
      if (timeSinceLastUpdate >= cycleDuration)
      {
        if(_currentPhase == TrafficLightPhase::red)
        {
          setCurrentPhase(TrafficLightPhase::green);
          messages.send(std::move(TrafficLightPhase::green));
        }
        else if(_currentPhase == TrafficLightPhase::green)
        {
          setCurrentPhase(TrafficLightPhase::red);
           messages.send(std::move(TrafficLightPhase::red));
        }
        
        lastUpdate = std::chrono::system_clock::now();
      }
      
    }
}

