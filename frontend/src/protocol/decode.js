export function decodePacketHeader(buffer) {
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
    const { view, offset } = packet;

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

export function decodeSnapshot(packet) {
    const { view, offset } = packet;

    let o = offset;

    const instrumentId = view.getUint32(o, true);
    o += 4;

    const bidCount = view.getUint32(o, true);
    o += 4;

    const askCount = view.getUint32(o, true);
    o += 4;

    const bids = [];

    for (let i = 0; i < bidCount; i++) {
        const price = view.getUint32(o, true);
        o += 4;

        const qty = view.getUint32(o, true);
        o += 4;

        bids.push({ price, qty });
    }

    const asks = [];

    for (let i = 0; i < askCount; i++) {
        const price = view.getUint32(o, true);
        o += 4;

        const qty = view.getUint32(o, true);
        o += 4;

        asks.push({ price, qty });
    }

    return {
        instrumentId,
        bids,
        asks
    };
}

export function decodeBookUpdate(packet) {
    const { view, offset } = packet;

    let o = offset;

    const instrumentId = view.getUint32(o, true);
    o += 4;

    const price = view.getUint32(o, true);
    o += 4;

    const deltaQty = view.getInt32(o, true);
    o += 4;

    const side = view.getUint8(o);
    o += 1;

    return {
        type: "BOOK_UPDATE",
        instrumentId,
        price,
        deltaQty,
        side: side === 0 ? "BUY" : "SELL"
    };
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
