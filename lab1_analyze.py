import requests
from bs4 import BeautifulSoup
import os

SOURCES = {
    "habr": [
        "https://habr.com/ru/articles/766620/",
        "https://habr.com/ru/articles/765432/", 
        "https://habr.com/ru/articles/760000/"
    ],
    "wikipedia": [
        "https://ru.wikipedia.org/wiki/Компьютер",
        "https://ru.wikipedia.org/wiki/Алгоритм",
        "https://ru.wikipedia.org/wiki/Программное_обеспечение"
    ]
}

def analyze_document(url, source_name, output_dir="lab1_data"):
    try:
        if not os.path.exists(output_dir):
            os.makedirs(output_dir)
            
        headers = {'User-Agent': 'Mozilla/5.0'}
        response = requests.get(url, headers=headers, timeout=10)
        raw_html = response.content
        raw_size = len(raw_html)
        
        soup = BeautifulSoup(raw_html, 'html.parser')
        
        # Чистим мусор
        for script in soup(["script", "style", "nav", "footer", "header", "aside", "form"]):
            script.extract()
            
        text_content = soup.get_text(separator=' ')
        clean_text = ' '.join(text_content.split())
        text_size = len(clean_text.encode('utf-8'))
        
        return {
            "source": source_name,
            "raw_size": raw_size,
            "text_size": text_size
        }
    except Exception as e:
        print(f"Error: {e}")
        return None

def main():
    results = []
    for source, urls in SOURCES.items():
        for url in urls:
            res = analyze_document(url, source)
            if res:
                results.append(res)
                print(f"{res['source']}: Сырой {res['raw_size']} -> Текст {res['text_size']}")

    total_raw = sum(r['raw_size'] for r in results)
    total_text = sum(r['text_size'] for r in results)
    
    print(f"1. Средний размер «сырого» документа: {total_raw / len(results):.0f} байт")
    print(f"2. Средний объем чистого текста: {total_text / len(results):.0f} байт")
    print(f"3. Ratio (полезный текст): {(total_text/total_raw)*100:.2f}%")

if __name__ == "__main__":
    main()
