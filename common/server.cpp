#include "server.h"
#include <cassert>
#include <iostream>
#include <thread>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include "event.h"
#include "matching_engine.h"
#include "order.h"
#include "packet.h"

constexpr int PACKET_VERSION = 1;

namespace {
    void reject(RejectReason reason) {
        //todo placeholder

    }

    [[noreturn]] void event_loop(EngineState& state, ClientManager& clients) {
        while (true) {
            Event e;
            {
                std::unique_lock lock(state.events_mutex);

                state.events_cv.wait(lock, [&] {
                    return !state.events.empty();
                });

                e = state.events.front();
                state.events.pop();
            }

            Packet p;
            p.header.type = PacketType::EVENT;
            p.header.length = sizeof(Event);

            p.data.resize(sizeof(Event));
            memcpy(p.data.data(), &e, sizeof(Event));

            // broadcast to all clients
            std::lock_guard lock(clients.mutex);
            for (auto& c : clients.clients) {
                c->send_packet(p);
            }
        }
    }
    bool check_order(const Order& order_msg, const EngineState& engine_state) {
        if (engine_state.instruments.size() - 1 < order_msg.instrument_id) {
            //todo more thorough check
            reject(RejectReason::INSTRUMENT_NOT_FOUND);
            return false;
        }
        if (!engine_state.instruments[order_msg.instrument_id].isActive) {
            reject(RejectReason::MARKET_CLOSED);
            return false;
        }
        if (order_msg.quantity <= 0) {
            reject(RejectReason::INVALID_QUANTITY);
            return false;
        }
        if (order_msg.price <= 0) {
            reject(RejectReason::INVALID_PRICE);
            return false;
        }
        return true;
    }


    void print_order(const Order& order_msg, const EngineState& engine_state) {
        std::string message{};
        message += "Message from client: ";
        if (order_msg.type == Side::BUY) {
            message += "BUY ";
        } else {
            message += "SELL ";
        }
        message += std::to_string(order_msg.quantity) + " ";
        message += engine_state.instrument_to_id.at(order_msg.instrument_id);
        message += " @ " + std::to_string(order_msg.price);
        std::cout<<message<<std::endl;
    }
    bool receive_packets(Session& connection, EngineState& engine_state) {
        //return true to keep connection alive
        Packet packet{};
        //todo emit events for the below errors
        if (!connection.receive_packet(packet)) {return false;} //lost connection
        if (packet.header.version != PACKET_VERSION) {return true;}
        if (packet.header.length > 1'000'000) {return true;}

        //handle packet type
        switch (packet.header.type) {
            case PacketType::NEW_ORDER: {
                Order order_msg;
                memcpy(&order_msg, packet.data.data(), packet.header.length);

                bool ok = true;
                if (!check_order(order_msg, engine_state)) {
                    emit_event(&engine_state, {
                        .type = EventType::REJECT,
                    });
                    //keep connection alive
                    return true;
                }
                order_msg.order_id = engine_state.next_order_id.fetch_add(1);
                emit_event(&engine_state, {
                    .type = EventType::ACK,
                    .order_id = order_msg.order_id
                });
                {
                    std::lock_guard lock(engine_state.inbox_mutex);
                    engine_state.inbox.push(order_msg);
                }
                engine_state.cv.notify_one();

                if (ok)
                    print_order(order_msg, engine_state);
                else
                    std::cout<<"Invalid Order"<<std::endl;

                return true;
            }
            default:
                std::cout<<"Invalid packet type received! "<< static_cast<uint16_t>(packet.header.type) <<std::endl;
                return true;
        }
    }

    void server_loop(Session& connection, EngineState& engine_state, ClientManager& clients) {
        while (true) {
            if (!receive_packets(connection, engine_state)) {
                connection.close();

                {
                    std::lock_guard lock(clients.mutex);

                    std::erase_if(
                        clients.clients,
                        [&](const auto& s)
                        {
                            return s.get() == &connection;
                        });
                }

                break;
            }
        }
    }
}

Server::Server(uint32_t port) : m_acceptor{m_io, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port)}{
    //todo check if connected
}
void Server::run(EngineState& engine_state) {
    MatchingEngine matching_engine{&engine_state};

    std::thread engine_thread([&] {
        matching_engine.run();
    });
    engine_thread.detach();

    ClientManager client_manager{};

    std::thread([&] {
        event_loop(engine_state, client_manager);
    }).detach();

    while (true) {
        boost::asio::ip::tcp::socket socket(m_io);
        m_acceptor.accept(socket);
        auto session = std::make_shared<Session>(std::move(socket));
        session->handshake();
        {
            std::lock_guard lock(client_manager.mutex);
            client_manager.clients.push_back(session);
        }

        std::thread([session, &engine_state, &client_manager] {
            server_loop(*session, engine_state, client_manager);
        }).detach();
    }
}

Server::~Server() {
    m_io.stop();
    m_acceptor.close();
}



