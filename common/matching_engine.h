#pragma once
#include "order.h"

class MatchingEngine {
public:
    explicit MatchingEngine(EngineState* state);
    void run();
    void process_order(const Order& order);
private:
    EngineState* m_state{};
    bool m_running{false};
};

