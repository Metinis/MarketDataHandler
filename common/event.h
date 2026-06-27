#pragma once
#include "order.h"

inline void emit_event(EngineState* state, const Event& e)
{
    {
        std::lock_guard lock(state->events_mutex);
        state->events.push(e);
    }

    state->events_cv.notify_one();
}
