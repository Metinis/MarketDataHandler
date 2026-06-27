import "./eventLog.css"

const EventType = {
    0: "ACK",
    1: "FILL",
    2: "REJECT",
};

export default function EventLog({ events }) {
    return (
        <div className="panel">
            <div className="panel-title">Event Feed</div>

            <div className="log">
                {events.map((e, i) => (
                    <div key={i} className={`event ${e.type?.toLowerCase()}`}>
                        {renderEvent(e)}
                    </div>
                ))}
            </div>
        </div>
    );
}

function renderEvent(e) {
    switch (e.type) {
        case "ACK":
            return (
                <>
                    🟢 ACK
                    <span className="muted"> order={e.orderId}</span>
                </>
            );

        case "REJECT":
            return (
                <>
                    🔴 REJECT
                    <span className="muted"> order={e.orderId}</span>
                </>
            );

        case "FILL":
            return (
                <>
                    ⚡ FILL
                    <span className="muted">
                        {" "}
                        {e.qty} @ {e.price}
                    </span>
                    <span className="dim">
                        {" "}
                        ({e.orderId} ↔ {e.restingId})
                    </span>
                </>
            );

        default:
            return (
                <>
                    ❓ UNKNOWN
                    <span className="muted"> {JSON.stringify(e)}</span>
                </>
            );
    }
}