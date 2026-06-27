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
inline void emit_book_update(EngineState* state, BookUpdate u) {
    std::lock_guard lock(state->book_mutex);
    state->book_updates.push(u);
    state->book_cv.notify_one();
}