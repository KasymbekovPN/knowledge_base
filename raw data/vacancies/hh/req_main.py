import requests
import pandas as pd

TEXT_0 = 'Python developer'
AREA_M = 1
PER_PAGE = 10

PARAMS = {
    'text': TEXT_0,
    'area': AREA_M,
    'per_page': PER_PAGE
}

HEADERS = {
    'User-Agent': 'iamlookingforjob/1.0 (kasymbekovpn@yandex.ru)'
}

def run():
    # response = requests.get('https://hh.ru/search/vacancy?text=c%2B%2B+developer&area=1')
    # response = requests.get('https://api.hh.ru/vacancies', headers=HEADERS, params=PARAMS)
    # data = response.json()
    # print(response)
    print(123)

if __name__ == '__main__':
    run()