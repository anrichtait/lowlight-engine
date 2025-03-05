#!/usr/bin/env python3

import re
import sys
from pathlib import Path

# this is copied from fileheader.py
HEADER_REGEX = r'(// -\*- c\+\+ -\*-)?(?P<version>#version\s*[^/\r\n]*)?' \
               r'(?(version)//.*)?\s*/\*[\s\S]*\\file.*\n' \
               r'(?P<saved>[\s\S]*?)\n\s*G3D Innovation Engine[\s\S]*?\*/.*\n'

def run_filter(file):
    text = file.read_text()

    match = re.match(HEADER_REGEX, text)
    if match:
        sys.stdout.write(text[:match.end()])

    classname = file.stem + '_' + file.suffix[1:]

    sys.stdout.write('namespace glsl {\n')
    if match and match.group('saved').strip():
        sys.stdout.write('/** ')
        sys.stdout.write(match.group('saved'))
        sys.stdout.write(' */\n')
    else:
        sys.stdout.write('/** Shader program */')
    sys.stdout.write('class ' + classname + ' {\n')
    sys.stdout.write('public:\n')
    if match:
        sys.stdout.write(text[match.end():])
    else:
        sys.stdout.write(text)
    sys.stdout.write('\n}; \n }')


run_filter(Path(sys.argv[1]))
