import { useState } from "react";
import OrderForm from "./components/orderForm";
import OrderBook from "./components/orderBook.jsx";
import EventLog from "./components/eventLog.jsx";

import { useExchangeSocket } from "./hooks/useExchangeSocket";
import { encodeOrder } from "./protocol/order";

function App() {
    const [events, setEvents] = useState([]);
    const [bids, setBids] = useState([]);
    const [asks, setAsks] = useState([]);
    const [trades, setTrades] = useState([]);

    const { connected, send } = useExchangeSocket((packet) => {
        console.log("PACKET:", packet);
        setEvents((prev) => [...prev, packet]);
    });

    function submitOrder(order) {
        const buffer = encodeOrder(order);
        send(buffer);
    }

    return (
        <div className="layout">
            <aside>
                <div>
                    Status: {connected ? "🟢 Connected" : "🔴 Disconnected"}
                </div>

                <OrderForm onSubmit={submitOrder} />
            </aside>

            <main>
                <EventLog events={events} />
                <OrderBook bids={bids} asks={asks} />
            </main>
        </div>
    );
}

export default App;