# RTC_HSI_OFF
Выключение генератора HSI в МК серии 1986ВЕ9х.

Пример показывает обход ошибки Errata - "0004 Невозможность выключить генератор HSI при нулевом ALRF", а так же то, что если после выключения HSI сбросить бит ALRF, HSI запускается вновь.

Подробнее в заметке - https://startmilandr.ru/doku.php/prog:bug:hsi_off_ve9x
