#!/usr/bin/env python3
# SPDX-FileCopyrightText: 2026 Valentin Eloy
# SPDX-License-Identifier: GPL-3.0-or-later

"""Vérifie qu'un outil épinglé à deux endroits l'est à la même version (refonte de la CI, phase 2).

Même défaut que celui que `check_qt_version_pin.py` ferme pour Qt : deux écritures d'un même fait
finissent par diverger sans que rien ne le signale. Ici :

- **clang-format** : `LLVM_VERSION` de ci.yml (job `format`, job `clang-tidy`) et le tag du dépôt
  `mirrors-clang-format` dans .pre-commit-config.yaml. Deux versions majeures ne formatent pas
  pareil : le hook reformaterait un fichier que la CI refuse, ou l'inverse.
- **Doxygen** : `DOXYGEN_VERSION` de ci.yml (qui valide la doc en PR) et de docs.yml (qui la
  publie).

Usage :
  python scripts/check_tool_pins.py
"""
import os
import re
import sys

CI = os.path.join('.github', 'workflows', 'ci.yml')
DOCS = os.path.join('.github', 'workflows', 'docs.yml')
PRE_COMMIT = '.pre-commit-config.yaml'


def env_value(name):
    return re.compile(r'^\s*%s:\s*[\'"]?([^\'"\s#]+)[\'"]?\s*(?:#.*)?$' % re.escape(name),
                      re.MULTILINE)


# Le SHA épinglé est suivi du tag en commentaire : `rev: <sha>  # frozen: v20.1.8`.
CLANG_FORMAT_RE = re.compile(
    r'repo:\s*https://github\.com/pre-commit/mirrors-clang-format\s*\n'
    r'\s*rev:\s*\S+\s*#\s*frozen:\s*v([0-9][^\s]*)')

PAIRS = [
    ('clang-format', (CI, env_value('LLVM_VERSION'), 'LLVM_VERSION'),
     (PRE_COMMIT, CLANG_FORMAT_RE, 'rev de mirrors-clang-format (# frozen: vX)')),
    ('Doxygen', (CI, env_value('DOXYGEN_VERSION'), 'DOXYGEN_VERSION'),
     (DOCS, env_value('DOXYGEN_VERSION'), 'DOXYGEN_VERSION')),
]


def read_single(path, pattern, label):
    try:
        with open(path, encoding='utf-8') as handle:
            matches = pattern.findall(handle.read())
    except OSError as error:
        print('ERREUR : %s illisible (%s).' % (path, error))
        return None
    if len(matches) != 1:
        print('ERREUR : %s attendu exactement une fois dans %s (trouvé %d).'
              % (label, path, len(matches)))
        return None
    return matches[0]


def main():
    os.chdir(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
    ok = True
    for tool, (path_a, pattern_a, label_a), (path_b, pattern_b, label_b) in PAIRS:
        version_a = read_single(path_a, pattern_a, label_a)
        version_b = read_single(path_b, pattern_b, label_b)
        if version_a is None or version_b is None:
            ok = False
            continue
        if version_a != version_b:
            print('ERREUR : %s épinglé à deux versions.' % tool)
            print('  %s : %s = %s' % (path_a, label_a, version_a))
            print('  %s : %s = %s' % (path_b, label_b, version_b))
            ok = False
        else:
            print('OK : %s %s dans %s et %s.' % (tool, version_a, path_a, path_b))
    return 0 if ok else 1


if __name__ == '__main__':
    sys.exit(main())
