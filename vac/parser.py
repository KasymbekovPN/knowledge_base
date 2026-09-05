import json
import re
from bs4 import BeautifulSoup

# Укажите имя вашего исходного файла
# filename = "page3"
# filename = "page4"
# filename = "page5"
# filename = "page6"
# filename = "page7"
# filename = "page8"
# filename = "page9"
# filename = "page10"
# filename = "page11"
# filename = "page12"
# filename = "page13"
# filename = "page14"
# filename = "page15"
# filename = "page16"
# filename = "page17"
filename = "page18"
input_filename = "./data/" + filename + ".html"
output_filename = "./data/output_" + filename + ".txt"

try:
    with open(input_filename, "r", encoding="utf-8") as f:
        content = f.read()

    # 1. Попробуем извлечь данные из JSON-контекста, если это сырой ответ API/данные страницы
    vacancies_data = []
    json_match = re.search(r'"vacancies"\s*:\s*(\[[^\]]*\])', content)

    if json_match:
        try:
            vacancies_json = json.loads(json_match.group(1))
            #<
            print(len(vacancies_json))
            for vac in vacancies_json:
                name = vac.get("name")
                # Извлекаем ссылку из вложенной структуры links -> desktop
                link = vac.get("links", {}).get("desktop")
                if name and link:
                    vacancies_data.append((link, name))
        except Exception:
            pass

    # 2. Если через JSON не получилось или собралось не всё, парсим HTML-верстку
    if not vacancies_data:
        soup = BeautifulSoup(content, "html.parser")

        # Ищем элементы списков вакансий по порядковым номерам или ссылкам
        # В предоставленном тексте ссылки ведут на hh.ru/vacancy/...
        links = soup.find_all("a", href=re.compile(r"hh\.ru/vacancy/\d+"))

        #<
        print(len(links))
        for link_tag in links:
            link = link_tag.get("href")
            # Название вакансии обычно находится внутри тега ссылки или в соседнем заголовке
            name = link_tag.get_text(strip=True)

            # Если текст внутри ссылки пустой или это просто цифра, ищем заголовок рядом
            if not name or name.isdigit():
                parent = link_tag.find_parent()
                if parent:
                    # Проверяем наличие текста в родительском контейнере
                    name = parent.get_text(strip=True)

            # Очищаем имя от лишних номеров в начале (например, "1. Инженер...")
            name = re.sub(r"^\d+\.\s*", "", name)

            if link and name and (link, name) not in vacancies_data:
                vacancies_data.append((link, name))

    # Записываем результат в файл вакансий
    with open(output_filename, "w", encoding="utf-8") as out_f:
        for link, name in vacancies_data:
            pure_link = link.split('?')[0]
            out_f.write(f"{pure_link} {name}\n")

    print(
        f"Успешно обработано вакансий: {len(vacancies_data)}. Результат записан в {output_filename}"
    )

except FileNotFoundError:
    print(
        f"Ошибка: Файл {input_filename} не найден. Переименуйте ваш файл или укажите правильный путь."
    )
