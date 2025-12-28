СБОРКА:
1. Токенизатор: g++ tokenizer.cpp -o tokenizer -lsqlite3
2. Индексатор:  g++ indexer.cpp -o indexer -lsqlite3
3. Поисковик:   g++ searcher.cpp -o searcher

ЗАПУСК:
1. Сбор данных: python3 robot.py config.yaml
2. Построение индекса (читает crawler_data.db -> создает index.bin): ./indexer
3. Запуск веб-сервера: python3 server.py
   -> Открыть в браузере: http://localhost:8080

АВТОТЕСТЫ:
./stemmer_test
./searcher "linux && python"
