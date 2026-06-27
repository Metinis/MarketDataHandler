#pragma once
#include <cstdint>
#include <cstring>
#include <vector>
#include <sys/socket.h>
#include <sys/types.h>

enum class PacketType : uint8_t {
    NEW_ORDER,
    EVENT,
    SNAPSHOT,
    BOOK_UPDATE
  };

#pragma pack(push, 1)
struct PacketHeader {
    uint16_t version{};
    PacketType type{};
    uint32_t length{};
    uint64_t seq{};
};
#pragma pack(pop)

struct Packet {
    PacketHeader header{};
    std::vector<char> data;
};

#pragma pack(push, 1)
struct PriceLevel {
    uint32_t price{};
    uint32_t quantity{};
};
#pragma pack(pop)

#pragma pack(push, 1)
struct Snapshot {
    uint32_t instrument_id{};
    uint32_t bid_count{};
    uint32_t ask_count{};
    std::vector<PriceLevel> bids{};
    std::vector<PriceLevel> asks{};
};
#pragma pack(pop)

enum class RejectReason : uint8_t {
    INSTRUMENT_NOT_FOUND,
    INVALID_QUANTITY,
    INVALID_PRICE,
    MARKET_CLOSED,
};
//#pragma pack(push, 1)

//#pragma pack(pop)

struct LoginRequest {
    uint64_t session_id{};
    char username[32];
    char password[32];
};

struct LoginAccepted {
    uint64_t session_id{};
};

struct LoginRejected {
    uint32_t reason{};
};

struct Heartbeat {
    uint64_t sequence{};
};

bool read_exact(int sock, void* data, size_t n);
bool send_exact(int sock, const void* data, size_t n);