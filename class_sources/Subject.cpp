#include <vector>
#include <algorithm>
#include "../class_headers/Observer.h"
#include "../class_headers/GameEvents.h"
#include "../class_headers/Subject.h"

void Subject::addObserver(Observer *observer) {
    if (std::find(observers.begin(), observers.end(), observer) == observers.end()) {
        observers.push_back(observer);
    } // check for duplicates
}

void Subject::removeObserver(Observer *observer) {
    observers.erase(std::remove(observers.begin(), observers.end(), observer), observers.end());
}

void Subject::notifyObservers(GameEvent event) {
    for (Observer* observer : observers)
        if (observer) observer->onNotify(event);
}
