#!/usr/bin/env python3

import re
import sys
from datetime import datetime
from pathlib import Path

# checks for header in the format of NEW_HEADER
HEADER_REGEX = r'(// -\*- c\+\+ -\*-)?(?P<version>#version\s*[^/\r\n]*)?' \
               r'(?(version)//.*)?\s*/\*[\s\S]*\\file.*\n' \
               r'(?P<saved>[\s\S]*?)\n\s*G3D Innovation Engine[\s\S]*?\*/.*\n'

NEW_HEADER = """{version}/**
  \\file {file}{extra}

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-{year}, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/
"""

SHADER_EXTS = ['.glsl', '.glc', '.pix', '.vrt', '.geo']
SOURCE_EXTS = ['.cpp', '.h']


def buildHeader(filepath, extra, version):
    if extra.strip():
        extra = '\n' + extra
    if version is not None:
        version = version.strip() + '\n'
    else:
        version = ''
    return NEW_HEADER.format(file=filepath, extra=extra,
                             version=version, year=datetime.now().year)


def replaceHeaders(filepaths):
    for file in filepaths:
        text = file.read_text()
        match = re.match(HEADER_REGEX, text)
        if match:
            saved = match.group('saved')
            version = match.group('version')
            header = buildHeader(file.as_posix(), saved, version)
            body = text[match.end():]
            full = header + body
            file.write_text(full)
        else:
            print('Missing file header (could not update): ' + file.as_posix())


if len(sys.argv) == 1:
    print(buildHeader('path/to/file.h', '', None))
else:
    target = Path(sys.argv[1])
    if target.is_file():
        replaceHeaders([target])
    else:
        print('Updating all shader files in ' + target.as_posix())
        for ext in SHADER_EXTS:
            replaceHeaders(target.rglob('**/*' + ext))
        print('Updating all source files in ' + target.as_posix())
        for ext in SOURCE_EXTS:
            replaceHeaders(target.rglob('**/*' + ext))
