#pragma once
#include <atomic>
#include <condition_variable>
#include <deque>
#include <map>
#include <queue>
#include <unordered_map>
#include <vector>
#include "session.h"

enum class OrderType : uint8_t {
    MARKET, //Don't care about price, just buy from cheapest sellers immediately
    LIMIT, //Trade only at this price or better: BUY 100 AAPL @ 200 LIMIT: 100 shares paying at most 200
    //For selling Price >= 200, sign switch
};
enum class TimeInForce : uint8_t {
    GTC, //Good till canceled
    //BUY 100 @ 200, Av: SELL 40 @ 200: Fill 40, Rest 60 on book
    IOC, //Immediate or Cancel
    //BUY 100 @ 200, Av: SELL 40 @ 200: Fill 40, Cancel 60
    FOK, //Fill or Kill
    //BUY 100 @ 200, Av: SELL 40 @ 200: Fill 0, Cancel 100

    //todo unused for now
    GTD, //Good till date
    DAY,
};
enum class Side : uint8_t {
    BUY,
    SELL,
};

#pragma pack(push, 1)
struct Order {
    uint32_t client_order_id{};
    uint64_t order_id;
    uint32_t quantity{};
    uint32_t price{}; //ignored for market
    uint32_t instrument_id{};
    OrderType order_type{};
    TimeInForce time_in_force{};
    Side type{Side::BUY}; //Buy or Sell
};
#pragma pack(pop)

struct BookOrder {
    uint64_t order_id;
    uint32_t quantity;
    TimeInForce tif;
};

#pragma pack(push, 1)
struct BookUpdate {
    uint32_t instrument_id;
    uint32_t price;
    int32_t delta_qty;   //+ or -
    Side side;
};
#pragma pack(pop)

//need to use matching engine to match these
struct OrderBook {
    std::map<uint32_t, std::deque<BookOrder>> bids{}; //buy price and order
    std::map<uint32_t, std::deque<BookOrder>> asks{}; //sell price and order
};

inline std::vector<PriceLevel> order_map_to_vec(const std::map<uint32_t, std::deque<BookOrder>>& map) {
    std::vector<PriceLevel> ret{};
    for (auto& [price, queue] : map) {
        uint32_t total = 0;
        for (auto& o : queue)
            total += o.quantity;

        ret.push_back({price, total});
    }
    return ret;
}

struct Instrument {
    bool isActive{true};
    OrderBook book{};
};

enum class EventType : uint8_t {
    ACK,
    REJECT,
    FILL,
    PARTIAL_FILL
};

#pragma pack(push, 1)
struct Event {
    EventType type;

    uint64_t order_id;
    uint64_t resting_order_id;

    uint32_t price;
    uint32_t quantity;
};
#pragma pack(pop)

struct ClientManager {
    std::mutex mutex;
    std::vector<std::shared_ptr<Session>> clients;
};

struct EngineState {
    std::vector<Instrument> instruments{};
    std::unordered_map<uint32_t, std::string> instrument_to_id{};
    //atomic to avoid race condition
    std::atomic<uint64_t> next_order_id{1};
    std::condition_variable cv{};
    std::mutex inbox_mutex{};
    //todo move to a lock free queue
    std::queue<Order> inbox{};

    std::queue<Event> events;
    std::mutex events_mutex;
    std::condition_variable events_cv;

    std::queue<BookUpdate> book_updates;
    std::mutex book_mutex;
    std::condition_variable book_cv;
};