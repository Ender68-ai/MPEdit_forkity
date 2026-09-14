// Paste this into your Deno Deploy project to immediately stop all KV reads
// It transparently forwards traffic from older mod versions to your Cloudflare Worker.
const TARGET = "https://multiplayer-edit.d050.workers.dev";

Deno.serve(async (req) => {
    if (req.method === "OPTIONS") {
        return new Response(JSON.stringify({ ok: true }), {
            status: 200,
            headers: {
                "Content-Type": "application/json",
                "Access-Control-Allow-Origin": "*",
                "Access-Control-Allow-Methods": "GET,POST,DELETE,OPTIONS",
                "Access-Control-Allow-Headers": "Content-Type, ngrok-skip-browser-warning, Bypass-Tunnel-Reminder",
            },
        });
    }

    const url = new URL(req.url);
    const targetUrl = new URL(url.pathname + url.search, TARGET);

    const headers = new Headers(req.headers);
    headers.set("Host", targetUrl.host);

    const init = {
        method: req.method,
        headers,
        redirect: "follow",
    };

    if (req.method !== "GET" && req.method !== "HEAD") {
        init.body = req.body;
    }

    try {
        const res = await fetch(targetUrl.toString(), init);
        const resHeaders = new Headers(res.headers);
        resHeaders.set("Access-Control-Allow-Origin", "*");
        resHeaders.set("Access-Control-Allow-Methods", "GET,POST,DELETE,OPTIONS");
        resHeaders.set("Access-Control-Allow-Headers", "Content-Type, ngrok-skip-browser-warning, Bypass-Tunnel-Reminder");

        return new Response(res.body, {
            status: res.status,
            statusText: res.statusText,
            headers: resHeaders,
        });
    } catch (err) {
        return new Response(JSON.stringify({ error: err.message }), {
            status: 502,
            headers: { "Content-Type": "application/json" },
        });
    }
});
