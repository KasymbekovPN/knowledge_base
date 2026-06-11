import json
from collections import Counter
from pathlib import Path

FOLDER = Path(r'c:\projects\knowledge_base\raw data\vacancies\data\c++ developer')


def run():
    with open(FOLDER / 'output.json', encoding='utf-8') as f:
        vacancies = json.load(f)

    counter: Counter = Counter()

    for v in vacancies:
        for skill in (v.get('skills_element') or []):
            counter[skill.strip()] += 1
        for skill in (v.get('skills_from_text') or []):
            counter[skill.strip()] += 1

    lines = [f'{count:>4}  {skill}' for skill, count in counter.most_common()]

    output = FOLDER / 'res.txt'
    output.write_text('\n'.join(lines), encoding='utf-8')
    print(f'done → {output}  ({len(counter)} unique skills)')


if __name__ == '__main__':
    run()
