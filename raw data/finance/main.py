import os
import configparser
import pdfplumber

from pathlib import Path

SECTION_DIR = 'dir'

KEY_WORK_DIR = 'work'
KEY_PDF_SOURCE_DIR = 'source.pdf'

PARAM_PDF_NAMES = 'pdf_names'

def get_config():
    config = configparser.ConfigParser()
    config.read(Path(__file__).parent / 'config.conf', encoding='utf-8')

    result = {}

    result[KEY_WORK_DIR] = Path(config[SECTION_DIR][KEY_WORK_DIR])

    parts = config[SECTION_DIR][KEY_PDF_SOURCE_DIR].strip().splitlines()
    result[KEY_PDF_SOURCE_DIR] = result[KEY_WORK_DIR].joinpath(*parts)

    return result

def get_pdf_names(config):
    return os.listdir(config[KEY_PDF_SOURCE_DIR])

def handle(config, params):
    result = {}
    handle_pdf_sources(config, params, result)

    return result

def handle_pdf_sources(config, params, result):
    pdf_paths = [config[KEY_PDF_SOURCE_DIR] / name for name in params[PARAM_PDF_NAMES]]
    for path in pdf_paths:
        with pdfplumber.open(path) as pdf:
            for page in pdf.pages:
                print(page.extract_text())
        print('\n\n\n')
        # break

if __name__ == '__main__':
    config = get_config()

    params = {}
    params[PARAM_PDF_NAMES] = get_pdf_names(config)

    handle(config, params)