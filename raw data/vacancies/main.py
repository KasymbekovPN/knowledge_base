
# i am looking for job


# import requests

# # Параметры запроса
# params = {
#     'text': 'Python Developer',  # Поисковый запрос
#     'area': 1,  # Код региона (1 — Москва)
#     'per_page': 100,  # Количество вакансий на странице
#     'page': 0  # Номер страницы (по умолчанию — 0 — первая страница)
# }

# # Выполнение запроса
# response = requests.get('https://api.hh.ru/vacancies', params=params)
# data = response.json()  # Получение JSON-ответа

# # Обработка результатов
# for vacancy in data['items']:
#     print(f"ID: {vacancy['id']}")
#     print(f"Название: {vacancy['name']}")
#     print(f"Компания: {vacancy['employer']['name']}")
#     print(f"Зарплата: {vacancy.get('salary', 'Не указана')}")
#     print('-' * 50)




# import requests
# import pandas as pd
# from collections import Counter
# import time

# # 1. Конфигурация запроса
# VACANCY_NAME = "Python developer"  # Укажите нужную профессию
# PER_PAGE = 100                     # Количество вакансий на страницу (макс. 100)
# PAGES_TO_SCAN = 5                  # Сколько страниц проверить (5 страниц = 500 вакансий)

# # Обязательный заголовок для API HeadHunter
# headers = {
#     'User-Agent': 'SkillStatsBot/1.0 (my_email@example.com)' 
# }

# skills_list = []
# print(f"Начинаем сбор вакансий по запросу: '{VACANCY_NAME}'...")

# # 2. Сбор ID вакансий через поиск
# for page in range(PAGES_TO_SCAN):
#     url = f"https://hh.ru{VACANCY_NAME}&per_page={PER_PAGE}&page={page}"
#     response = requests.get(url, headers=headers)
    
#     if response.status_with_code == 200:
#         data = response.json()
#         items = data.get('items', [])
        
#         if not items:
#             break
            
#         # 3. Запрос деталей по каждой вакансии для извлечения key_skills
#         for item in items:
#             vac_id = item['id']
#             vac_url = f"https://hh.ru{vac_id}"
#             vac_response = requests.get(vac_url, headers=headers)
            
#             if vac_response.status_code == 200:
#                 vac_data = vac_response.json()
#                 # Извлекаем навыки из массива key_skills
#                 if 'key_skills' in vac_data:
#                     for skill in vac_data['key_skills']:
#                         skills_list.append(skill['name'])
            
#             # Небольшая пауза, чтобы API не заблокировал за частые запросы
#             time.sleep(0.1)
#         print(f"Обработана страница {page + 1} из {PAGES_TO_SCAN}")
#     else:
#         print(f"Ошибка при запросе страницы {page}: {response.status_code}")
#         break

# # 4. Подсчет статистики
# skills_counter = Counter(skills_list)
# df_skills = pd.DataFrame(skills_counter.most_common(20), columns=['Навык / Требование', 'Частота'])

# print("\n=== ТОП-20 ТРЕБОВАНИЙ РАБОТОДАТЕЛЕЙ ===")
# print(df_skills.to_string(index=False))

import requests
import pandas as pd



# # Параметры запроса
# params = {
#     'text': 'Python Developer',  # Поисковый запрос
#     'area': 1,  # Код региона (1 — Москва)
#     'per_page': 100,  # Количество вакансий на странице
#     'page': 0  # Номер страницы (по умолчанию — 0 — первая страница)
# }

# # Выполнение запроса
# response = requests.get('https://api.hh.ru/vacancies', params=params)
# data = response.json()  # Получение JSON-ответа

def run():
    print('run')

if __name__ == '__main__':
    run()