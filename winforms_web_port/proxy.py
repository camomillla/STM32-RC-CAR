from http.server import ThreadingHTTPServer, BaseHTTPRequestHandler
from urllib.parse import urlparse, parse_qs
from urllib.request import Request, urlopen
from urllib.error import URLError, HTTPError
import re

PORT = 8787

def add_cors(h):
    h.send_header("Access-Control-Allow-Origin", "*")
    h.send_header("Access-Control-Allow-Methods", "GET, OPTIONS")
    h.send_header("Access-Control-Allow-Headers", "Content-Type")

class Handler(BaseHTTPRequestHandler):
    def do_OPTIONS(self):
        self.send_response(204)
        add_cors(self)
        self.end_headers()

    def do_GET(self):
        parsed = urlparse(self.path)
        qs = parse_qs(parsed.query)
        ip = (qs.get("ip", [""])[0]).strip()

        if parsed.path == "/":
            self.send_response(200)
            add_cors(self)
            self.send_header("Content-Type", "text/plain; charset=utf-8")
            self.end_headers()
            self.wfile.write(b"OK. Use /init?ip=...  /telemetry?ip=...  /motor/2?ip=...")
            return

        is_motor = re.fullmatch(r"/motor/\d+", parsed.path) is not None
        is_simple = parsed.path in ("/init", "/telemetry", "/engine", "/horn", "/lights")

        if not is_motor and not is_simple and parsed.path != "/":
            self.send_response(404)
            add_cors(self)
            self.end_headers()
            self.wfile.write(b"Not found")
            return
        
        if not ip:
            self.send_response(400)
            add_cors(self)
            self.end_headers()
            self.wfile.write(b"Missing ip param, e.g. /init?ip=192.168.1.128")
            return

        if not ip.startswith("http://") and not ip.startswith("https://"):
            ip = "http://" + ip
        ip = ip.rstrip("/")

        target_url = f"{ip}{parsed.path}"

        try:
            req = Request(target_url, method="GET", headers={"Accept": "application/json"})
            with urlopen(req, timeout=10) as r:
                body = r.read()
                status = r.status
                ctype = r.headers.get("Content-Type", "application/json")
        except HTTPError as e:
            status = e.code
            body = e.read() if e.fp else str(e).encode("utf-8")
            ctype = "text/plain; charset=utf-8"
        except URLError as e:
            self.send_response(502)
            add_cors(self)
            self.send_header("Content-Type", "text/plain; charset=utf-8")
            self.end_headers()
            self.wfile.write(f"Bad gateway: {e}".encode("utf-8"))
            return

        self.send_response(status)
        add_cors(self)
        self.send_header("Content-Type", ctype)
        self.end_headers()
        self.wfile.write(body)

if __name__ == "__main__":
    server = ThreadingHTTPServer(("127.0.0.1", PORT), Handler)
    print(f"Proxy running on http://127.0.0.1:{PORT}")
    server.serve_forever()
