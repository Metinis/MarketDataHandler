#pragma once
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>

struct EngineState;

class Server {
public:
    explicit Server(uint32_t port);
    [[noreturn]] void run(EngineState& engine_state);
    ~Server();
private:
    boost::asio::io_context m_io;
    boost::asio::ip::tcp::acceptor m_acceptor;
};
