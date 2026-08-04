import os
import re
import configparser
import pdfplumber

from pathlib import Path

SECTION_DIR = 'dir'
SECTION_ID = 'id'

KEY_WORK_DIR = 'work'
KEY_PDF_SOURCE_DIR = 'source.pdf'
KEY_PRIVATE_CONFIG = 'private.config'
KEY_S_ID = 's.id'
KEY_T_ID = 't.id'

INNER_KEY_LINES = 'lines'
INNER_KEY_HANDLER = 'handler'

PARAM_PDF_NAMES = 'pdf_names'

SPATTERN = r'(\d{2}\.\d{2}\.\d{4} \d{2}:\d{2})\s+(.+?)\s+([+-]?\d[\d ]*,\d{2})\s+(\d[\d ]*,\d{2})$'
TPATTERN = r'^(\d{2}\.\d{2}\.\d{2} \d{2}:\d{2}) \d{2}\.\d{2}\.\d{2} (.+?) ([\d.]+)\s*₽'

def get_config():
    main_config = configparser.ConfigParser()
    main_config.read(Path(__file__).parent / 'config.conf', encoding='utf-8')

    main_result = {}

    main_result[KEY_WORK_DIR] = Path(main_config[SECTION_DIR][KEY_WORK_DIR])

    parts = main_config[SECTION_DIR][KEY_PDF_SOURCE_DIR].strip().splitlines()
    main_result[KEY_PDF_SOURCE_DIR] = main_result[KEY_WORK_DIR].joinpath(*parts)

    private_config_path = main_result[KEY_WORK_DIR].joinpath(main_config[SECTION_DIR][KEY_PRIVATE_CONFIG])
    private_config = configparser.ConfigParser()
    private_config.read(private_config_path, encoding='utf-8')

    private_result = {}
    private_result[KEY_S_ID] = private_config[SECTION_ID][KEY_S_ID]
    private_result[KEY_T_ID] = private_config[SECTION_ID][KEY_T_ID]

    return main_result | private_result

def get_pdf_names(config):
    return os.listdir(config[KEY_PDF_SOURCE_DIR])

def handle(config, params):
    result = {}
    handle_pdf_sources(config, params, result)

    return result

def handle_pdf_sources(config, params, result):
    extract_from_pdf_sources(config, params, result)
    define_pdf_handler(config, params, result)
    for path, handler in result[INNER_KEY_HANDLER].items():
        handler(path, config, params, result)

def extract_from_pdf_sources(config, params, result):
    pdf_paths = [config[KEY_PDF_SOURCE_DIR] / name for name in params[PARAM_PDF_NAMES]]
    lines = {}
    for path in pdf_paths:
        with pdfplumber.open(path) as pdf:
            l = []
            for page in pdf.pages:
                l += page.extract_text().strip().splitlines()
            lines[path] = l
    result[INNER_KEY_LINES] = lines

def define_pdf_handler(config, params, result):
    s_id = config[KEY_S_ID]
    t_id = config[KEY_T_ID]

    handlers = {}
    for path, lines in result[INNER_KEY_LINES].items():
        handler = None
        for line in lines:
            if s_id in line:
                handler = handle_pdf_sources_as_s
                break
            elif t_id in line:
                handler = handle_pdf_sources_as_t
                break
        handlers[path] = handler
    result[INNER_KEY_HANDLER] = handlers

def handle_pdf_sources_as_s(path, config, params, result):
    buffer = {}
    for line in result[INNER_KEY_LINES][path]:

        if len(buffer) != 0:
            idx = line.find('Операция по карте')
            if idx != -1:
                sub = line[17:line.find('Операция по карте')].strip()
                if 'Перевод для К. Павел Николаевич' not in sub:
                    pure_amount = buffer['amount'].strip()
                    if pure_amount[0] != '+':
                        buffer['description'] = sub
                        write_payment(buffer)

            buffer.clear()
            continue

        m = re.match(SPATTERN, line)
        if m == None:
            continue
        date, operation, amount, balance = m.groups()
        buffer['date'] = date
        buffer['operation'] = operation
        buffer['amount'] = amount
        buffer['balance'] = balance
        buffer['source'] = 'S'

def handle_pdf_sources_as_t(path, config, params, result):
    # print('handle_pdf_sources_as_t ', path)
    # print('handle_pdf_sources_as_t ', result[INNER_KEY_LINES][path])

    buffer = {}
    for line in result[INNER_KEY_LINES][path]:
        m = re.match(TPATTERN, line)
        if m == None:
            continue

        date, description, amount = m.groups()
        date = re.sub(r'\.(\d{2})\s', r".20\1 ", date)
        buffer['date'] = date
        buffer['description'] = description
        buffer['amount'] = amount
        buffer['source'] = 'T'
        write_payment(buffer)
    pass

def write_payment(buffer):
    print(buffer)

if __name__ == '__main__':
    config = get_config()
    # print(config)

    params = {}
    params[PARAM_PDF_NAMES] = get_pdf_names(config)

    handle(config, params)