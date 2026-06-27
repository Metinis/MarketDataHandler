#include "session.h"
#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <boost/beast/websocket.hpp>

Session::Session(tcp::socket socket)
    : m_ws{std::move(socket)} {
}

void Session::handshake() {
    boost::beast::error_code ec;
    m_ws.accept(ec);

    if (ec) {
        m_connected = false;
    } else {
        m_connected = true;
        m_ws.binary(true);
    }
}

Session::Session(Session&& other) noexcept
    : m_ws(std::move(other.m_ws)) {
    m_connected = other.m_connected;
    m_last_sequence = other.m_last_sequence;

    other.m_connected = false;
}

bool Session::send_packet(const Packet& packet) {
    if (!m_connected)
        return false;

    std::vector<uint8_t> buffer(
        sizeof(PacketHeader) + packet.header.length
    );

    std::memcpy(buffer.data(), &packet.header, sizeof(PacketHeader));
    std::memcpy(buffer.data() + sizeof(PacketHeader),
                packet.data.data(),
                packet.header.length);



    boost::beast::error_code ec;
    m_ws.write(boost::asio::buffer(buffer), ec);

    if (ec) {
        m_connected = false;
        return false;
    }

    return true;
}

bool Session::receive_packet(Packet& packet)
{
    if (!m_connected)
        return false;

    boost::beast::flat_buffer buffer;
    boost::beast::error_code ec;

    m_ws.read(buffer, ec);

    if (ec)
        return false;

    auto data = buffer.data();

    std::memcpy(&packet.header,
        boost::asio::buffer_cast<const void*>(data),
        sizeof(PacketHeader));

    packet.data.resize(packet.header.length);

    std::memcpy(packet.data.data(),
        (const char*)boost::asio::buffer_cast<const void*>(data)
        + sizeof(PacketHeader),
        packet.header.length);

    return true;
}

void Session::close()
{
    if (!m_connected)
        return;

    m_connected = false;

    boost::beast::error_code ec;

    m_ws.close(
        boost::beast::websocket::close_code::normal,
        ec);

    m_ws.next_layer().close(ec);
}

Session::~Session() {
    close();
}