export function decodePacket(buffer) {
    const view = new DataView(buffer);

    let offset = 0;

    const version = view.getUint16(offset, true);
    offset += 2;

    const type = view.getUint8(offset);
    offset += 1;

    const length = view.getUint32(offset, true);
    offset += 4;

    const seq = view.getBigUint64(offset, true);
    offset += 8;

    return {
        version,
        type,
        length,
        seq,
        offset,
        view
    };
}
export function decodeEvent(packet) {
    const { packetType, view, offset } = packet;

    // only EVENT packets
    //if (packetType !== 1) return null;

    let o = offset;

    const eventType = view.getUint8(o);
    o += 1;

    const orderId = Number(view.getBigUint64(o, true));
    o += 8;

    if (eventType === 0) {
        return { type: "ACK", orderId };
    }

    if (eventType === 1) {
        return { type: "REJECT", orderId };
    }

    if (eventType === 2) {
        const restingId = Number(view.getBigUint64(o, true));
        o += 8;

        const price = view.getUint32(o, true);
        o += 4;

        const qty = view.getUint32(o, true);

        return {
            type: "FILL",
            orderId,
            restingId,
            price,
            qty
        };
    }

    return null;
}

function toTradeEvent(e) {
    if (e.type !== "FILL") return null;

    return {
        time: new Date().toLocaleTimeString(),
        side: e.side ?? "TRADE", // optional if you don’t send it
        qty: e.qty,
        price: e.price,
        orderId: e.orderId,
        restingId: e.restingId
    };
}
