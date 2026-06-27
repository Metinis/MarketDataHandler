import { useState, useEffect } from "react";
import OrderForm from "./components/orderForm";
import OrderBook from "./components/orderBook.jsx";
import EventLog from "./components/eventLog.jsx";
import { useExchangeSocket } from "./hooks/useExchangeSocket";
import { encodeOrder } from "./protocol/order";
import "./App.css"

const updateOrderBook = (orders, price, deltaQty, isBid) => {
    const map = new Map();

    orders.forEach(o => {
        map.set(o.price, { ...o });
    });

    const existing = map.get(price);

    const newQty =
        (existing?.qty ?? 0) + deltaQty;

    if (newQty <= 0) {
        map.delete(price);
    } else {
        map.set(price, {
            price,
            qty: newQty
        });
    }

    const result = Array.from(map.values());

    return isBid
        ? result.sort((a, b) => b.price - a.price)
        : result.sort((a, b) => a.price - b.price);
};

function App() {
    const [events, setEvents] = useState([]);
    const [bids, setBids] = useState([]);
    const [asks, setAsks] = useState([]);
    const { connected, send } = useExchangeSocket(
        event => setEvents(e => [...e, event]),

        snapshot => {
            setBids(snapshot.bids);
            setAsks(snapshot.asks);
        },

        update => {
            if (update.side === "BUY")
                setBids(prev =>
                    updateOrderBook(
                        prev,
                        update.price,
                        update.deltaQty,
                        true
                    )
                );
            else
                setAsks(prev =>
                    updateOrderBook(
                        prev,
                        update.price,
                        update.deltaQty,
                        false
                    )
                );
        }
    );

    function submitOrder(order) {
        console.log("📤 Submitting order:", order);
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