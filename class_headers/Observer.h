#ifndef OBSERVER_H
#define OBSERVER_H

#include "GameEvents.h"

// observer virtual class
class Observer {
public:
    virtual ~Observer() = default;
    virtual void onNotify(GameEvent event) = 0;
};

#endif //OBSERVER_H
