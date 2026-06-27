export function encodeOrder({
                                side,
                                instrument,
                                quantity,
                                price,
                                orderType
                            }) {
    const buffer = new ArrayBuffer(64);
    const view = new DataView(buffer);

    let offset = 0;

    // PacketHeader

    view.setUint16(offset, 1, true);
    offset += 2;

    view.setUint8(offset, 0); // NEW_ORDER
    offset += 1;

    view.setUint32(offset, 27, true);
    offset += 4;

    view.setBigUint64(offset, 1n, true);
    offset += 8;

    // Order

    view.setUint32(offset, 1, true);
    offset += 4;

    view.setBigUint64(offset, 0n, true);
    offset += 8;

    view.setUint32(offset, quantity, true);
    offset += 4;

    view.setUint32(offset, price, true);
    offset += 4;

    view.setUint32(offset, instrument, true);
    offset += 4;

    view.setUint8(offset++, orderType);
    view.setUint8(offset++, 0); // TIF
    view.setUint8(offset++, side);

    return buffer.slice(0, offset);
}