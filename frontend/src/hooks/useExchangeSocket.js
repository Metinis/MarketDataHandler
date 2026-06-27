import {useEffect, useRef, useState} from "react";
import {decodeBookUpdate, decodeEvent, decodePacketHeader, decodeSnapshot} from "../protocol/decode.js";

export function useExchangeSocket(onEvent, onSnapshot, onBookUpdate) {
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

            const packet = decodePacketHeader(buffer);

            if(packet.type === 1) {
                const decoded = decodeEvent(packet);
                if (decoded) {
                    onEvent?.(decoded);
                }
            } else if (packet.type === 2) {
                const decoded = decodeSnapshot(packet);
                if (decoded) {
                    console.log("SNAPSHOT:", decoded);
                    onSnapshot?.(decoded);
                }
            } else if (packet.type === 3) {
                const decoded = decodeBookUpdate(packet);
                if (decoded) {
                    console.log("BOOK UPDATE:", decoded);
                    onBookUpdate?.(decoded);
                }
            }
        };

        return () => socket.current?.close();
    }, []);

    function send(buffer) {
        socket.current?.send(buffer);
    }

    return { connected, send };
}

