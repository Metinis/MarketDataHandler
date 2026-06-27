import { useEffect, useRef, useState } from "react";
import {decodeEvent, decodePacket} from "../protocol/event";

export function useExchangeSocket(onEvent) {
    const socket = useRef(null);
    const [connected, setConnected] = useState(false);

    useEffect(() => {
        socket.current = new WebSocket("ws://localhost:8080");
        socket.current.binaryType = "arraybuffer";

        socket.current.onopen = () => setConnected(true);
        socket.current.onclose = () => setConnected(false);

        socket.current.onmessage = async (event) => {
            const buffer =
                event.data instanceof Blob
                    ? await event.data.arrayBuffer()
                    : event.data;

            const packet = decodePacket(buffer);

            const decoded = decodeEvent(packet);

            if (decoded) {
                onEvent?.(decoded);
            }
        };

        return () => socket.current?.close();
    }, []);

    function send(buffer) {
        socket.current?.send(buffer);
    }

    return { connected, send };
}
