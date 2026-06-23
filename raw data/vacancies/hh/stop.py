import json
from collections import Counter
from pathlib import Path

# FOLDER = Path(r'c:\projects\knowledge_base\raw data\vacancies\hh\data\java')
# FOLDER = Path(r'c:\projects\knowledge_base\raw data\vacancies\hh\data\c++ developer')
# FOLDER = Path(r'c:\projects\knowledge_base\raw data\vacancies\hh\data\c++')
FOLDERS = [
    Path(r'c:\projects\knowledge_base\raw data\vacancies\hh\data\java'),
    Path(r'c:\projects\knowledge_base\raw data\vacancies\hh\data\c++ developer'),
    Path(r'c:\projects\knowledge_base\raw data\vacancies\hh\data\c++')
]
FOLDER = Path(r'c:\projects\knowledge_base\raw data\vacancies\hh')

SOURCE_FIELDS = ['requirements', 'file', 'project_description', 'responsibilities', 'nice_to_have'];
# REPLACERS = [',', '\n', '-', ';', '.', '\r\n', '\n\r', '(', ')', '/']
REPLACERS = [',', '\n', '-', ';', '.', '(', ')', '/']

def run():
    counter: Counter = Counter()
    for path in FOLDERS:
        handle_folder(path, counter)

    lines = [f'{skill}' for skill, count in counter.items()]
    print(len(lines))

    output = FOLDER / 'stop_words_original.txt'
    output.write_text('\n'.join(lines), encoding='utf-8')
    # print(f'done → {output}  ({len(counter)} unique skills)')

    # print(len(counter))


def handle_folder(path, counter):
    with open(path / 'output.json', encoding='utf-8') as f:
        vacancies = json.load(f)

    for v in vacancies:
        for key in SOURCE_FIELDS:
            if key in v:
                content = v[key]
                if not isinstance(content, str):
                    continue
                for r in REPLACERS:
                    content = content.replace(r, ' ')
                spl = content.split(' ')
                for hop in spl:
                    if hop == '':
                        continue;
                    counter[hop] += 1

if __name__ == '__main__':
    run()
