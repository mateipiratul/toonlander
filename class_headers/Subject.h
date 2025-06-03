#ifndef SUBJECT_H
#define SUBJECT_H

#include <vector>
#include "Observer.h"
#include "GameEvents.h"

class Subject {
    std::vector<Observer*> observers;

protected:
    void notifyObservers(GameEvent event);

public:
    virtual ~Subject() = default;

    void addObserver(Observer* observer);
    void removeObserver(Observer* observer);

};

#endif //SUBJECT_H
