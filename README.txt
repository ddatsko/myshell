Практична myshell 3
Виконав: Дацко Денис

Передумови:
    - Бібліотека Boost::program_options 1.71+
    - Бібліотека readline
    - C++ компілятор з підтримкою 17-го стандарту С++
    - cmake 3.17+
    - GNU make

Компіляція:
Для компіяції виконайте наступні команди
    - $ mkdir build
    - $ cd build
    - $ cmake .. -G "Unix Makefiles"
    - $ make

Запуск:
    Запустити файл myshell ( $ ./myshell [ <script_filename> ])
       - Якщо вказати ім'я файлу - інтерпретатор його виконає, інакше запуститься інтерактивний режим



Примітки до реалізації:
    - Щоб запутити сервер потрібно ввести команду у форматі $ ./myshell --server [ --port <port_num> ]
    - Все логування іде у захардкоджений файл log.txt. Варто перенести це в CLI, але поки що так
    - Виправив Warning, пов`язаний з викликом tmpnam





Старі примітки: (те, що лишилося з практичної 2):
    - Я реалізував усі потрібні функції
    - У var=$(<command>) в якості команди можна засунути майже все (і пайп, і щось з перенаправленням, і т.д., ), проте не гарантую, що все працюватиме добре
    - В загальному, все, що є на cms працює добре. Проте, я не впевнений, що парсер зможе добре розпарсити дуже щакручені комбінації
    - Пайпи для вбудованих команд не працюють зовсім, проте з перенаправленням все ок
    - Щодо Memory Leaks, Valgrind не знайшов нічого





Старі примітки (те, що лишилося в README з першої практичної !!!!!!!!!!!)
    - Я перевіряв усе з Valgrind на memory leaks - все ОК

    - Я реалізував підтримку "" i ''

    - Я реалізував Escape Characters (\X), правда, вони працюють трохи по-іншому, ніж в BASH-i.
        (Якщо стоїть символ \ - то наступний завжди вважається Escape character-ом)
        Наприклад, у мене з
            $ mecho "\g"
        argv будуть
            ["mecho", "g"]
        а в BASH-i
            ["mecho", "\g"]

    - Я реалізуавв підтримку присвоєння локальних змінних середовища
        Приклад використання:
        - $ a=10 <something>
        ** Зауважте, що тут <something> проігнорується

    - Нотатка: всі змінні, що є в environ при запуску myshell відразу екпортуються, тобто якщо першою ж командою
        викликати myshell, дочірній матиме ті самі змінні, що й батьківський

    - Якщо мені щось було незрозуміло, я орієнтувався на те, як це відбувається в BASH

    - ./<name>.msh працює дуже криво і тільки з файлами з розширенням .msh
        , але я нічого кращого не придумав, бо інакше скрипт запускається в /bin/sh

    - Якщо видалити шлях до myshell i mycat в PATH - вони не працюваимуть
