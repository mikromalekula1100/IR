# server.py
import http.server
import socketserver
import subprocess
import urllib.parse

PORT = 8080

class SearchHandler(http.server.SimpleHTTPRequestHandler):
    def do_GET(self):
        parsed_path = urllib.parse.urlparse(self.path)
        query_params = urllib.parse.parse_qs(parsed_path.query)
        
        if parsed_path.path == "/":
            self.send_response(200)
            self.send_header('Content-type', 'text/html; charset=utf-8')
            self.end_headers()
            
            html = """
            <html>
            <head><title>My Search Engine</title>
            <style>
                body { font-family: sans-serif; max-width: 800px; margin: 0 auto; padding: 20px; }
                input { width: 70%; padding: 10px; font-size: 16px; }
                button { padding: 10px 20px; font-size: 16px; cursor: pointer; }
                .result { margin-bottom: 20px; border-bottom: 1px solid #ccc; padding-bottom: 10px; }
                .url { color: green; font-size: 14px; }
                h3 { margin: 0; color: #1a0dab; }
                a { text-decoration: none; }
                a:hover { text-decoration: underline; }
            </style>
            </head>
            <body>
                <h1>Lab 7: Boolean Search</h1>
                <form action="/search" method="get">
                    <input type="text" name="q" placeholder="Enter query (e.g. computer && algorithm)" required>
                    <button type="submit">Search</button>
                </form>
            </body>
            </html>
            """
            self.wfile.write(html.encode('utf-8'))
            
        elif parsed_path.path == "/search":
            query = query_params.get('q', [''])[0]
            
            try:
                result = subprocess.run(['./searcher', query], capture_output=True, text=True)
                output_lines = result.stdout.split('\n')
            except Exception as e:
                output_lines = [f"Error executing searcher: {e}"]

            self.send_response(200)
            self.send_header('Content-type', 'text/html; charset=utf-8')
            self.end_headers()
            
            results_html = ""
            count = 0
            for line in output_lines:
                if line.strip():
                    count += 1
                    results_html += f"""
                    <div class="result">
                        <h3><a href="{line}">{line}</a></h3>
                        <div class="url">{line}</div>
                    </div>
                    """
            
            html = f"""
            <html>
            <head>
                <title>Results for {query}</title>
                <style>
                    body {{ font-family: sans-serif; max-width: 800px; margin: 0 auto; padding: 20px; }}
                    .result {{ margin-bottom: 15px; }}
                    a {{ font-size: 18px; color: blue; }}
                    .stats {{ color: #555; font-size: 12px; margin-bottom: 20px; }}
                    .back {{ margin-bottom: 20px; display: block; }}
                </style>
            </head>
            <body>
                <a href="/" class="back">&larr; Back to Search</a>
                <h2>Results for: <i>{query}</i></h2>
                <div class="stats">Found {count} documents</div>
                {results_html}
            </body>
            </html>
            """
            self.wfile.write(html.encode('utf-8'))
        else:
            super().do_GET()

print(f"Starting server at http://localhost:{PORT}")
with socketserver.TCPServer(("", PORT), SearchHandler) as httpd:
    httpd.serve_forever()
