import { useState } from "react";
import "./orderForm.css";

export default function OrderForm({ onSubmit }) {
    const [side, setSide] = useState(0);
    const [instrument, setInstrument] = useState(0);
    const [quantity, setQuantity] = useState(100);
    const [price, setPrice] = useState(200);
    const [orderType, setOrderType] = useState(1);

    function submit(e) {
        e.preventDefault();

        onSubmit({
            side,
            instrument,
            quantity,
            price,
            orderType
        });
    }

    return (
        <div className="ticket">
            <div className="ticket-title">
                Order Entry
            </div>

            <form onSubmit={submit}>
                <label>Side</label>
                <select
                    value={side}
                    onChange={e => setSide(Number(e.target.value))}
                >
                    <option value={0}>BUY</option>
                    <option value={1}>SELL</option>
                </select>

                <label>Instrument</label>
                <input
                    type="number"
                    value={instrument}
                    onChange={e =>
                        setInstrument(Number(e.target.value))
                    }
                />

                <label>Quantity</label>
                <input
                    type="number"
                    value={quantity}
                    onChange={e =>
                        setQuantity(Number(e.target.value))
                    }
                />

                <label>Price</label>
                <input
                    type="number"
                    value={price}
                    onChange={e =>
                        setPrice(Number(e.target.value))
                    }
                />

                <label>Type</label>
                <select
                    value={orderType}
                    onChange={e =>
                        setOrderType(Number(e.target.value))
                    }
                >
                    <option value={1}>LIMIT</option>
                    <option value={0}>MARKET</option>
                </select>

                <button
                    className={
                        side === 0
                            ? "buy-button"
                            : "sell-button"
                    }
                >
                    {side === 0
                        ? "BUY"
                        : "SELL"}
                </button>
            </form>
        </div>
    );
}