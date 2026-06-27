#include <cstring>
#include <iostream>
#include <sstream>
#include <thread>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include "../common/order.h"
#include "../common/packet.h"

struct Command {
    Side side;
    uint32_t instrument_id;
    uint32_t quantity;
    uint32_t price;
    OrderType type;
};

bool is_numeric(const std::string &s) {
    for (const auto &c: s) {
        if (!std::isdigit(c)) {
            return false;
        }
    }
    return true;
}

void handle_event(const Event &e) {
    switch (e.type) {
        case EventType::ACK:
            std::cout << "ACK " << e.order_id << "\n";
            break;

        case EventType::REJECT:
            std::cout << "REJECT " << e.order_id << "\n";
            break;

        case EventType::FILL:
            std::cout << "FILL "
                    << e.quantity
                    << " @ "
                    << e.price
                    << "\n";
            break;
    }
}

void handle_packet(const Packet &p) {
    switch (p.header.type) {
        case PacketType::EVENT: {
            Event e;
            memcpy(&e, p.data.data(), sizeof(Event));

            handle_event(e);
            break;
        }
        default:
            std::cout << "Unknown packet\n";
    }
}

std::optional<Command> parse_command(const std::string &line) {
    std::stringstream ss(line);

    std::array<std::string, 5> args;
    for (int i = 0; i < 5; i++) {
        if (!std::getline(ss, args[i], ' '))
            return std::nullopt;
    }

    Command cmd{};

    if (args[0] == "BUY") cmd.side = Side::BUY;
    else if (args[0] == "SELL") cmd.side = Side::SELL;
    else return std::nullopt;

    if (!is_numeric(args[1]) || !is_numeric(args[2]) || !is_numeric(args[3]))
        return std::nullopt;

    cmd.instrument_id = std::stoul(args[1]);
    cmd.quantity = std::stoul(args[2]);
    cmd.price = std::stoul(args[3]);

    if (args[4] == "MARKET") cmd.type = OrderType::MARKET;
    else if (args[4] == "LIMIT") cmd.type = OrderType::LIMIT;
    else return std::nullopt;

    return cmd;
}

Order to_order(const Command &cmd, uint64_t client_id) {
    Order o{};
    o.type = cmd.side;
    o.instrument_id = cmd.instrument_id;
    o.quantity = cmd.quantity;
    o.price = cmd.price;
    o.order_type = cmd.type;
    o.client_order_id = client_id;
    return o;
}

Packet build_order_packet(const Order &order) {
    Packet p;

    p.header.version = 1;
    p.header.type = PacketType::NEW_ORDER;
    p.header.length = sizeof(Order);

    p.data.resize(sizeof(Order));
    memcpy(p.data.data(), &order, sizeof(order));

    return p;
}

void receiver_loop(Session &connection) {
    while (true) {
        Packet p;
        if (!connection.receive_packet(p)) break;
        handle_packet(p);
    }
}

std::optional<Packet> next_packet() {
    std::string line;
    std::getline(std::cin, line);

    auto cmd = parse_command(line);
    if (!cmd) return std::nullopt;

    static uint64_t id{1};
    Order order = to_order(*cmd, id++);
    return build_order_packet(order);
}

using asio::ip::tcp;

int main() {
    asio::io_context io;

    tcp::socket socket(io);

    tcp::resolver resolver(io);
    auto endpoints = resolver.resolve("127.0.0.1", "8080");

    asio::connect(socket, endpoints);

    //send length, then message
    Session connection{(std::move(socket))};
    std::thread receiver([&] {
        receiver_loop(connection);
    });
    std::cout << "Command format: <BUY/SELL> <symbol> <qty> <price> <MARKET/LIMIT>" << std::endl;
    while (true) {
        std::cout << "Enter Command: ";

        auto packet = next_packet();
        if (!packet) {
            std::cout << "Invalid command\n";
            continue;
        }

        if (!connection.send_packet(*packet)) {
            break;
        }
    }
    receiver.join();
}
