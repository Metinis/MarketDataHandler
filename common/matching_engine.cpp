#include "matching_engine.h"
#include "event.h"
#include <mutex>

namespace {
    BookOrder order_to_book(const Order& order_msg) {
        return BookOrder {
            .order_id = order_msg.order_id,
            .quantity = order_msg.quantity,
            .tif = order_msg.time_in_force,
        };
    }


}

MatchingEngine::MatchingEngine(EngineState *state) : m_state(state) {
    m_running = true;
}

void MatchingEngine::process_order(const Order& incoming_order) {
    Order order = incoming_order;

    auto& instrument = m_state->instruments[order.instrument_id];
    auto& book = instrument.book;

    uint32_t remaining_qty = order.quantity;

    auto match_side = [&](auto& opposite_map, bool is_buy) {
        while (remaining_qty > 0 && !opposite_map.empty()) {

            auto best_it = opposite_map.begin();
            uint32_t best_price = best_it->first;

            // price check
            if (is_buy && best_price > order.price)
                break;
            if (!is_buy && best_price < order.price)
                break;

            auto& queue = best_it->second;

            while (remaining_qty > 0 && !queue.empty()) {

                BookOrder& resting = queue.front();

                uint32_t trade_qty =
                    std::min(remaining_qty, resting.quantity);

                emit_event(m_state, {
                    .type = EventType::FILL,
                    .order_id = order.order_id,
                    .resting_order_id = resting.order_id,
                    .price = best_price,
                    .quantity = trade_qty
                });

                emit_book_update(m_state, {
                    .instrument_id = order.instrument_id,
                    .price = best_price,
                    .delta_qty = -static_cast<int32_t>(trade_qty),
                    .side = is_buy ? Side::SELL : Side::BUY
                });

                remaining_qty -= trade_qty;
                resting.quantity -= trade_qty;

                if (resting.quantity == 0)
                    queue.pop_front();
            }

            if (queue.empty())
                opposite_map.erase(best_it);
        }
    };

    if (order.type == Side::BUY) {

        match_side(book.asks, true);

        // leftover goes on bid book
        if (remaining_qty > 0 && order.order_type == OrderType::LIMIT) {

            book.bids[order.price].push_back({
                .order_id = order.order_id,
                .quantity = remaining_qty,
                .tif = order.time_in_force
            });

            emit_book_update(m_state, {
                .instrument_id = order.instrument_id,
                .price = order.price,
                .delta_qty = static_cast<int32_t>(remaining_qty),
                .side = Side::BUY
            });
        }
    }

    else {

        match_side(book.bids, false);

        // leftover goes on ask book
        if (remaining_qty > 0 && order.order_type == OrderType::LIMIT) {

            book.asks[order.price].push_back({
                .order_id = order.order_id,
                .quantity = remaining_qty,
                .tif = order.time_in_force
            });

            emit_book_update(m_state, {
                .instrument_id = order.instrument_id,
                .price = order.price,
                .delta_qty = static_cast<int32_t>(remaining_qty),
                .side = Side::SELL
            });
        }
    }

    // MARKET orders never rest — no book update needed
}


void MatchingEngine::run() {
    while (m_running) {
        Order order;

        {
            std::unique_lock lock(m_state->inbox_mutex);

            m_state->cv.wait(lock, [&] {
                return !m_state->inbox.empty() || !m_running;
            });

            if (!m_running)
                break;

            order = m_state->inbox.front();
            m_state->inbox.pop();
        }

        process_order(order);
    }
}

