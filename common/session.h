#pragma once
#include <memory>
#include <asio/ip/tcp.hpp>
#include <boost/beast/websocket.hpp>
#include <asio.hpp>
#include <boost/asio/ip/tcp.hpp>

#include "packet.h"


struct EngineState;


class Session : public std::enable_shared_from_this<Session>{
public:
    using tcp = boost::asio::ip::tcp;

    explicit Session(tcp::socket socket) ;

    Session(Session&&) noexcept;
    Session& operator=(Session&&) = delete;

    Session(const Session&) = delete;
    Session& operator=(const Session&) = delete;

    void handshake();

    bool send_packet(const Packet& packet);
    bool receive_packet(Packet& packet);

    void close();

    ~Session();

private:
    boost::beast::websocket::stream<tcp::socket> m_ws;
    bool m_connected{true};
    uint64_t m_last_sequence{};
};