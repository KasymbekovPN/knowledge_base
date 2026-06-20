import json
import re
import sys
from pathlib import Path

from bs4 import BeautifulSoup

# FOLDER = Path(r'c:\projects\knowledge_base\raw data\vacancies\hh\data\java')
# FOLDER = Path(r'c:\projects\knowledge_base\raw data\vacancies\hh\data\c++ developer')
FOLDER = Path(r'c:\projects\knowledge_base\raw data\vacancies\hh\data\c++')

SECTION_HEADERS = {
    'responsibilities': r'(?:Обязанности|Задачи|Что делать|Что предстоит делать|Чем предстоит заниматься)',
    'requirements':     r'(?:Требования|Что нужно|Что мы ожидаем|Что мы ждём|Ожидаем от кандидата|Hard Skills)',
    'conditions':       r'(?:Условия|Что предлагаем|Мы предлагаем|Что дадим|Что мы предлагаем|Мы готовы предложить)',
    'nice_to_have':     r'(?:Желательные|Будет плюсом|Приветствуется)',
}

ALL_HEADERS = '|'.join(SECTION_HEADERS.values())


def _text(el) -> str | None:
    return el.get_text(separator=' ', strip=True) if el else None


def _extract_section(full_text: str, header_pattern: str) -> str | None:
    pattern = rf'{header_pattern}[^\n]*\n(.*?)(?=\n(?:{ALL_HEADERS})[^\n]*\n|$)'
    match = re.search(pattern, full_text, re.DOTALL | re.IGNORECASE)
    return match.group(1).strip() if match else None


def _skills_from_tags(soup: BeautifulSoup) -> list[str] | None:
    elements = soup.find_all(attrs={'data-qa': 'skills-element'})
    if not elements:
        return None
    return [el.get_text(strip=True) for el in elements]


def _skills_from_text(requirements_text: str | None) -> list[str]:
    if not requirements_text:
        return []
    items = re.split(r'\.\s+|\n', requirements_text)
    return [s.strip().rstrip('.') for s in items if len(s.strip()) > 10]


def parse_vacancy(html_path: Path) -> dict:
    with open(html_path, encoding='utf-8') as f:
        soup = BeautifulSoup(f.read(), 'html.parser')

    company    = _text(soup.find(attrs={'data-qa': 'vacancy-company-name'}))
    salary_el  = soup.find(attrs={'data-qa': 'vacancy-salary'})
    salary     = _text(salary_el) if salary_el else 'не указана'
    experience = _text(soup.find(attrs={'data-qa': 'vacancy-experience'}))
    schedule   = _text(soup.find(attrs={'data-qa': 'working-hours-text'}))
    employment = _text(soup.find(attrs={'data-qa': 'common-employment-text'}))
    work_format = _text(soup.find(attrs={'data-qa': 'work-formats-text'}))

    desc_el   = soup.find(attrs={'data-qa': 'vacancy-description'}) or soup.body
    full_text = desc_el.get_text(separator='\n', strip=True) if desc_el else ''

    is_accredited = (
        bool(soup.find(attrs={'data-qa': 'employer-card-employer-it-accreditation'}))
        or bool(re.search(r'IT.{0,2}аккредитаци', full_text, re.IGNORECASE))
    )

    responsibilities = _extract_section(full_text, SECTION_HEADERS['responsibilities'])
    requirements     = _extract_section(full_text, SECTION_HEADERS['requirements'])
    conditions       = _extract_section(full_text, SECTION_HEADERS['conditions'])
    nice_to_have     = _extract_section(full_text, SECTION_HEADERS['nice_to_have'])

    project_desc = None
    first_section = rf'\n(?:{ALL_HEADERS})[^\n]*\n'
    pre_duties = re.search(rf'(.*?)(?={first_section})', full_text, re.DOTALL | re.IGNORECASE)
    if pre_duties:
        paragraphs = [p.strip() for p in pre_duties.group(1).split('\n') if len(p.strip()) > 50]
        project_desc = ' '.join(paragraphs[:3]) or None

    return {
        'file':                html_path.name,
        'company':             company,
        'schedule':            schedule,
        'employment':          employment,
        'work_format':         work_format,
        'salary':              salary,
        'experience':          experience,
        'is_it_accredited':    is_accredited,
        'project_description': project_desc,
        'conditions':          conditions,
        'responsibilities':    responsibilities,
        'requirements':        requirements,
        'nice_to_have':        nice_to_have,
        'skills_element':      _skills_from_tags(soup),
        'skills_from_text':    _skills_from_text(requirements),
    }


def run():
    folder = Path(sys.argv[1]) if len(sys.argv) > 1 else FOLDER

    results = []
    for html_file in sorted(folder.glob('*.html')):
        print(f'processing: {html_file.name}')
        results.append(parse_vacancy(html_file))

    output_path = folder / 'output.json'
    with open(output_path, 'w', encoding='utf-8') as f:
        json.dump(results, f, ensure_ascii=False, indent=2)

    print(f'done → {output_path} ({len(results)} vacancies)')


if __name__ == '__main__':
    run()
