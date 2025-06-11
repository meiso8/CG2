#include "../Header/FenceEvent.h"
#include<cassert>

void FenceEvent::Create() {

    fenceEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    assert(fenceEvent != nullptr);

}