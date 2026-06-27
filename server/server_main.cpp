#include <iostream>
#include "../common/server.h"
#include "../common/order.h"
#include <asio.hpp>

int main() {
    Server server{8080};
    Instrument instrument {
        .isActive = true,
        .book = {},
    };
    EngineState engine{};
    //todo make a function
    engine.instruments.push_back(instrument);
    engine.instrument_to_id[0] = "APPL";

    server.run(engine);
}