import time
import requests
import sqlite3
import yaml
import sys
from urllib.parse import urlparse, urljoin
from bs4 import BeautifulSoup

class SearchRobot:
    def __init__(self, config_path):
        with open(config_path, 'r', encoding='utf-8') as f:
            self.config = yaml.safe_load(f)
        self.db_path = self.config['db']['path']
        self.delay = self.config['logic']['request_delay']
        self.max_docs = self.config['logic']['max_docs']
        self.allowed = self.config['logic']['allowed_domains']
        self.init_db()

    def init_db(self):
        with sqlite3.connect(self.db_path) as conn:
            cur = conn.cursor()
            cur.execute('CREATE TABLE IF NOT EXISTS queue (url TEXT PRIMARY KEY, status TEXT)')
            cur.execute('CREATE TABLE IF NOT EXISTS documents (id INTEGER PRIMARY KEY, url TEXT, source TEXT, raw_html BLOB, crawled_at REAL)')
            cur.execute("SELECT count(*) FROM queue")
            if cur.fetchone()[0] == 0:
                for url in self.config['logic']['seed_urls']:
                    cur.execute("INSERT OR IGNORE INTO queue VALUES (?, 'new')", (url,))
            conn.commit()

    def is_valid(self, url):
        if not any(d in url for d in self.allowed): return False
        if any(x in url for x in ['.jpg', '.png', '#', 'search', 'Special:', 'Служебная:']): return False
        return True

    def run(self):
        print("Робот запущен...")
        try:
            while True:
                with sqlite3.connect(self.db_path) as conn:
                    cur = conn.cursor()
                    cur.execute("SELECT count(*) FROM documents")
                    if cur.fetchone()[0] >= self.max_docs: break
                    cur.execute("SELECT url FROM queue WHERE status='new' LIMIT 1")
                    row = cur.fetchone()
                    if not row: break
                    url = row[0]
                    cur.execute("UPDATE queue SET status='processed' WHERE url=?", (url,))
                    conn.commit()

                try:
                    resp = requests.get(url, headers={'User-Agent': 'Bot/1.0'}, timeout=5)
                    if resp.status_code == 200 and 'text/html' in resp.headers.get('Content-Type',''):
                        with sqlite3.connect(self.db_path) as conn:
                            cur = conn.cursor()
                            cur.execute("INSERT INTO documents (url, source, raw_html, crawled_at) VALUES (?, ?, ?, ?)",
                                        (url, 'wiki' if 'wiki' in url else 'habr', resp.content, time.time()))
                            soup = BeautifulSoup(resp.content, 'html.parser')
                            for a in soup.find_all('a', href=True):
                                link = urljoin(url, a['href']).split('#')[0]
                                if self.is_valid(link):
                                    cur.execute("INSERT OR IGNORE INTO queue VALUES (?, 'new')", (link,))
                            conn.commit()
                        print(f"Saved: {url}")
                except Exception as e:
                    print(f"Err {url}: {e}")
                time.sleep(self.delay)
        except KeyboardInterrupt:
            print("Stopped.")

if __name__ == "__main__":
    SearchRobot(sys.argv[1]).run()
