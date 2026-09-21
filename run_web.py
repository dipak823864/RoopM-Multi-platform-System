import http.server
import socketserver
import os
import webbrowser

PORT = 8000
WEB_DIR = os.path.join("build", "web")

class Handler(http.server.SimpleHTTPRequestHandler):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, directory=WEB_DIR, **kwargs)

print(f"Starting Roopm Web Server on http://localhost:{PORT}")
print(f"Serving directory: {WEB_DIR}")

try:
    with socketserver.TCPServer(("", PORT), Handler) as httpd:
        print("Server running... (Press Ctrl+C to stop)")
        # Calculate absolute path for user clarity
        abs_path = os.path.abspath(WEB_DIR)
        print(f"Physical path: {abs_path}")
        
        # Open in browser
        webbrowser.open(f"http://localhost:{PORT}")
        
        httpd.serve_forever()
except OSError as e:
    print(f"Error starting server: {e}")
    print("Try a different port or check if the port is in use.")
