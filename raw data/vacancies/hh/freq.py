import json
from collections import Counter
from pathlib import Path

FOLDER = Path(r'c:\projects\knowledge_base\raw data\vacancies\hh\data\java')
# FOLDER = Path(r'c:\projects\knowledge_base\raw data\vacancies\hh\data\c++ developer')
# FOLDER = Path(r'c:\projects\knowledge_base\raw data\vacancies\hh\data\c++')

SOURCE_FIELDS = ['requirements', 'file', 'project_description', 'responsibilities', 'nice_to_have'];
# REPLACERS = [',', '\n', '-', ';', '.', '\r\n', '\n\r', '(', ')', '/']
REPLACERS = [',', '\n', '-', ';', '.', '(', ')', '/']

def run():
    with open(FOLDER / 'output.json', encoding='utf-8') as f:
        vacancies = json.load(f)

    counter: Counter = Counter()

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
        # for skill in (v.get('skills_element') or []):
        #     counter[skill.strip()] += 1
        # for skill in (v.get('skills_from_text') or []):
        #     counter[skill.strip()] += 1

    # lines = [f'{count:>4}  {skill}' for skill, count in counter.most_common()]
    # output = FOLDER / 'res.txt'
    # output.write_text('\n'.join(lines), encoding='utf-8')
    # print(f'done → {output}  ({len(counter)} unique skills)')

    print(counter)

    # print(fields)
    # skills_element
    # <
    # requirements
    # file
    # project_description
    # responsibilities
    # nice_to_have


if __name__ == '__main__':
    run()
