export default function OrderBook({ bids, asks }) {
    return (
        <div className="panel">
            <div className="panel-title">
                Order Book
            </div>

            <div className="book">
                <div className="asks">
                    {asks.map(a => (
                        <div key={a.price}>
                            {a.price} × {a.qty}
                        </div>
                    ))}
                </div>

                <hr />

                <div className="bids">
                    {bids.map(b => (
                        <div key={b.price}>
                            {b.price} × {b.qty}
                        </div>
                    ))}
                </div>
            </div>
        </div>
    );
}