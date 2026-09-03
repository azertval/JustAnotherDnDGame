#!/usr/bin/env python3
# SPDX-FileCopyrightText: 2026 Valentin Eloy
# SPDX-License-Identifier: GPL-3.0-or-later

"""Lint des identifiants d'exigences (EX-...) de JustAnotherDnDGame.

Vérifie que les identifiants d'exigences forment un référentiel cohérent :
- chaque exigence est **déclarée exactement une fois** (ancre Doxygen
  ``\\anchor EX-XXX-NNN`` dans les spécifications) ;
- toute **référence** à un ``EX-XXX-NNN`` (spécifications, lots, code,
  workflows) pointe vers une exigence déclarée (aucune référence orpheline) ;
- toute exigence **déclarée** est référencée au moins une fois quelque part (spécification
  détaillée, lot, code) — sauf si elle est explicitement qualifiée d'**invariant transverse** ou de
  **post-MVP** dans le fichier de spécification qui la déclare (LOT-66 TACHE-03) : ce silence-là est
  documenté, pas orphelin.

Usage :
  python scripts/lint_exigences.py           # contrôle (code de sortie 1 si problème)
  python scripts/lint_exigences.py --next     # affiche le prochain numéro libre par catégorie
"""
import os
import re
import sys

ID_RE = re.compile(r'EX-[A-Z]+-[0-9]+')
ANCHOR_RE = re.compile(r'\\anchor\s+(EX-[A-Z]+-[0-9]+)')
SPLIT_RE = re.compile(r'(EX-[A-Z]+)-([0-9]+)')

SCAN_EXTENSIONS = ('.md', '.h', '.hpp', '.cpp', '.yml', '.yaml')
EXCLUDED_DIRS = {'.git', 'generated', 'build', 'build-release', 'out', 'External', 'bin', 'obj',
                 # Worktree d'agent (ex. fork Claude Code) : un clone complet du dépôt peut y vivre
                 # temporairement et fait sortir chaque identifiant en double (LOT-66).
                 '.claude'}

# Exigences déclarées sans être jamais référencées, PAR CONSTRUCTION : des invariants transverses
# que tout lot respecte sans avoir à les citer (documenté dans le fichier qui les déclare), ou un
# reste post-MVP explicitement écarté. Toute nouvelle entrée doit s'accompagner de cette même
# qualification dans la spécification concernée — ne pas s'en servir pour faire taire le lint.
UNREFERENCED_ALLOWED = {
    'EX-ARCH-001', 'EX-ARCH-060', 'EX-ARCH-070',  # invariants transverses, architecture.md
    'EX-NFR-032',                                  # invariant transverse, exigences-non-fonctionnelles.md
    'EX-DEC-031',                                  # post-MVP (section 3, hors 0.1.0), decors.md
}


def iter_files(root):
    for dirpath, dirnames, filenames in os.walk(root):
        dirnames[:] = [d for d in dirnames if d not in EXCLUDED_DIRS]
        for name in filenames:
            if name.endswith(SCAN_EXTENSIONS):
                yield os.path.join(dirpath, name)


def collect(root):
    """Retourne (declarations, references).

    declarations : dict id -> liste de (fichier, ligne) des ``\\anchor``.
    references   : dict id -> liste de (fichier, ligne) de toutes les autres mentions.
    """
    declarations = {}
    references = {}
    for path in iter_files(root):
        rel = os.path.relpath(path, root)
        try:
            with open(path, encoding='utf-8') as handle:
                lines = handle.readlines()
        except (UnicodeDecodeError, OSError):
            continue
        for number, line in enumerate(lines, start=1):
            anchors_on_line = set(ANCHOR_RE.findall(line))
            for rid in anchors_on_line:
                declarations.setdefault(rid, []).append((rel, number))
            for rid in ID_RE.findall(line):
                if rid in anchors_on_line:
                    continue  # le token de l'ancre n'est pas une référence
                references.setdefault(rid, []).append((rel, number))
    return declarations, references


def check(root):
    declarations, references = collect(root)
    errors = []

    for rid, places in sorted(declarations.items()):
        if len(places) > 1:
            spots = ', '.join('%s:%d' % p for p in places)
            errors.append('DOUBLON : %s declaree %d fois (%s)' % (rid, len(places), spots))

    for rid, places in sorted(references.items()):
        if rid not in declarations:
            spots = ', '.join('%s:%d' % p for p in places[:5])
            errors.append('ORPHELINE : %s referencee mais jamais declaree (%s)' % (rid, spots))

    for rid, places in sorted(declarations.items()):
        if rid not in references and rid not in UNREFERENCED_ALLOWED:
            spot = '%s:%d' % places[0]
            errors.append('NON REFERENCEE : %s declaree (%s) mais jamais referencee ailleurs '
                          '(a referencer, ou a qualifier d\'invariant/post-MVP dans '
                          'UNREFERENCED_ALLOWED)' % (rid, spot))

    if errors:
        print('Lint exigences : %d probleme(s)' % len(errors))
        for message in errors:
            print('  - ' + message)
        return 1

    print('Lint exigences : OK (%d exigences declarees, %d referencees).'
          % (len(declarations), len(references)))
    return 0


def next_free(root):
    declarations, _ = collect(root)
    by_category = {}
    width = {}
    for rid in declarations:
        match = SPLIT_RE.match(rid)
        category, num = match.group(1), match.group(2)
        by_category.setdefault(category, set()).add(int(num))
        width[category] = max(width.get(category, 3), len(num))
    print('Prochain numero libre par categorie :')
    for category in sorted(by_category):
        used = by_category[category]
        candidate = 1
        while candidate in used:
            candidate += 1
        print('  %-10s : %s-%s  (max utilise : %d)'
              % (category, category, str(candidate).zfill(width[category]), max(used)))
    return 0


def main():
    root = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
    if '--next' in sys.argv[1:]:
        return next_free(root)
    return check(root)


if __name__ == '__main__':
    sys.exit(main())
