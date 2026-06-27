import "./orderBook.css";
import { useMemo } from "react";

const DEPTH = 15;

export default function OrderBook({ bids, asks }) {
    // Sort and limit depth
    const sortedBids = useMemo(() => {
        return [...bids]
            .sort((a, b) => b.price - a.price)
            .slice(0, DEPTH);
    }, [bids]);

    const sortedAsks = useMemo(() => {
        return [...asks]
            .sort((a, b) => a.price - b.price)
            .slice(0, DEPTH);
    }, [asks]);

    // Calculate max quantity for depth visualization
    const maxQty = useMemo(() => {
        const allQty = [...sortedBids, ...sortedAsks].map(item => item.qty);
        return Math.max(...allQty, 1);
    }, [sortedBids, sortedAsks]);

    if (bids.length === 0 && asks.length === 0) {
        return (
            <div className="panel">
                <div className="panel-title">Order Book</div>
                <div className="empty">No data</div>
            </div>
        );
    }

    const bestBid = sortedBids.length ? sortedBids[0].price : 0;
    const bestAsk = sortedAsks.length ? sortedAsks[0].price : 0;
    const spread = bestBid && bestAsk ? (bestAsk - bestBid).toFixed(2) : "—";

    return (
        <div className="panel">
            <div className="panel-title">
                Order Book
                <span className="spread-info">
                    Spread: {spread}
                </span>
            </div>

            <div className="ladder">
                {/* Asks Section */}
                <div className="ladder-header">
                    <span>Price (ASK)</span>
                    <span>Size</span>
                    <span>Total</span>
                </div>

                <div className="asks-section">
                    {sortedAsks.map((ask, index) => {
                        const depthPercent = (ask.qty / maxQty) * 100;
                        return (
                            <div key={`ask-${ask.price}`} className="ladder-row ask-row">
                                <div className="depth-bar ask-depth" style={{ width: `${depthPercent}%` }} />
                                <span className="price-cell ask-price">
                                    {ask.price.toFixed(2)}
                                </span>
                                <span className="qty-cell">{ask.qty}</span>
                                <span className="total-cell">
                                    {calculateTotal(sortedAsks, index, false)}
                                </span>
                            </div>
                        );
                    })}
                </div>

                {/* Spread */}
                <div className="spread-row">
                    <span className="bid-price">{bestBid ? bestBid.toFixed(2) : "—"}</span>
                    <span className="spread-value">Spread: {spread}</span>
                    <span className="ask-price">{bestAsk ? bestAsk.toFixed(2) : "—"}</span>
                </div>

                {/* Bids Section */}
                <div className="bids-section">
                    {sortedBids.map((bid, index) => {
                        const depthPercent = (bid.qty / maxQty) * 100;
                        return (
                            <div key={`bid-${bid.price}`} className="ladder-row bid-row">
                                <div className="depth-bar bid-depth" style={{ width: `${depthPercent}%` }} />
                                <span className="price-cell bid-price">
                                    {bid.price.toFixed(2)}
                                </span>
                                <span className="qty-cell">{bid.qty}</span>
                                <span className="total-cell">
                                    {calculateTotal(sortedBids, index, true)}
                                </span>
                            </div>
                        );
                    })}
                </div>
            </div>
        </div>
    );
}

// Helper to calculate cumulative totals
function calculateTotal(orders, index, isBid) {
    let total = 0;
    if (isBid) {
        for (let i = 0; i <= index; i++) {
            total += orders[i].qty;
        }
    } else {
        for (let i = 0; i <= index; i++) {
            total += orders[i].qty;
        }
    }
    return total;
}