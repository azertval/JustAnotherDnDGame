# SPDX-FileCopyrightText: 2026 Valentin Eloy
# SPDX-License-Identifier: GPL-3.0-or-later
"""Garde-fou : les illustrations livrees et leur manifeste ne doivent pas diverger.

`Source/Elements/Assets/UI/` porte les illustrations extraites du corpus (`LOT-67`,
`EX-IHM-076`) et le manifeste `illustrations.json` qui dit d'ou chacune vient. Le corpus, lui,
n'est pas versionne (`EX-CNT-023`) : sur un poste sans les PDF -- et sur le runner de CI, qui n'en
aura jamais -- rien ne peut rejouer l'extraction pour verifier ce que ces fichiers contiennent.

Ce que ce controle rend impossible, c'est donc l'ecart SILENCIEUX :

- une illustration retouchee a la main, ou remplacee par une image venue d'ailleurs (empreinte) ;
- une illustration supprimee, ou ajoutee sans passer par le catalogue du module (orpheline) ;
- un manifeste redige a la main dont les dimensions ne sont pas celles du fichier ;
- une cle citee par le C++ que le manifeste ne porte pas -- et l'inverse, une illustration livree
  que plus personne ne nomme.

Le dernier point est celui qui casse le plus souvent a l'usage : les trois premiers protegent une
donnee, celui-la protege le LIEN entre la donnee et le code, qui est ce qui lache quand on renomme
un fichier sans y penser.

Aucune dependance : l'empreinte est du hashlib, et les dimensions se lisent dans l'en-tete du
fichier. Meme motif que `check_design_tokens.py`.

Usage :
    python scripts/check_ui_assets.py     # code de sortie non nul si divergence
"""

from __future__ import annotations

import hashlib
import json
import re
import struct
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
UI = ROOT / "Source" / "Elements" / "Assets" / "UI"
MANIFEST = UI / "illustrations.json"
# Seul endroit du code ou un nom de fichier d'illustration est ecrit.
NAMING_SOURCE = ROOT / "Source" / "HMI" / "Interface" / "MainMenu.h"

errors: list[str] = []


def fail(message: str) -> None:
    errors.append(message)


def jpeg_size(data: bytes) -> tuple[int, int]:
    """Largeur et hauteur d'un JPEG, lues dans son premier marqueur de cadre.

    Un JPEG n'a pas d'en-tete a decalage fixe : il faut parcourir ses marqueurs jusqu'au SOFn, qui
    porte les dimensions. Une quinzaine de lignes, et aucune dependance -- la CI n'installe pas
    Pillow, et ce controle doit tourner partout ou tourne un python.
    """
    if data[:2] != b"\xff\xd8":
        raise ValueError("signature JPEG absente")
    offset = 2
    while offset + 4 <= len(data):
        if data[offset] != 0xFF:
            raise ValueError(f"marqueur attendu au decalage {offset}")
        marker = data[offset + 1]
        # SOF0..SOF15, sauf DHT (C4), JPG (C8) et DAC (CC) qui ne sont pas des cadres.
        if 0xC0 <= marker <= 0xCF and marker not in (0xC4, 0xC8, 0xCC):
            height, width = struct.unpack(">HH", data[offset + 5 : offset + 9])
            return width, height
        (length,) = struct.unpack(">H", data[offset + 2 : offset + 4])
        offset += 2 + length
    raise ValueError("aucun marqueur de cadre (SOFn) dans le fichier")


def read_manifest() -> dict:
    if not MANIFEST.is_file():
        print(f"check_ui_assets : {MANIFEST.relative_to(ROOT)} absent.", file=sys.stderr)
        sys.exit(1)
    return json.loads(MANIFEST.read_text(encoding="utf-8"))


def check_illustrations(manifest: dict) -> set[str]:
    """Chaque illustration declaree existe, et est bien celle que le manifeste decrit."""
    declared: dict[str, str] = {}
    for entry in manifest["illustrations"]:
        identifier = entry["id"]
        if identifier in declared:
            fail(f"`{identifier}` declare deux fois dans le manifeste")
        declared[identifier] = entry["file"]

        path = UI / entry["file"]
        if not path.is_file():
            fail(f"`{identifier}` : {entry['file']} absent de {UI.relative_to(ROOT)}")
            continue
        data = path.read_bytes()

        if hashlib.sha256(data).hexdigest() != entry["sha256"]:
            fail(
                f"`{identifier}` : {entry['file']} n'a plus l'empreinte du manifeste. "
                f"Rejouer l'extraction :\n    python scripts/sourcebook illustrations"
            )
        if len(data) != entry["bytes"]:
            fail(f"`{identifier}` : {len(data)} octets, {entry['bytes']} annonces")
        try:
            width, height = jpeg_size(data)
        except (ValueError, struct.error) as error:
            fail(f"`{identifier}` : {entry['file']} n'est pas un JPEG lisible ({error})")
            continue
        if [width, height] != entry["size"]:
            fail(f"`{identifier}` : {width}x{height}, {entry['size']} annonces")

        # La provenance n'est pas decorative : elle repond, le jour d'une publication, a la
        # question « qu'est-ce qui doit sauter ? » (EX-CNT-001).
        if not entry.get("provenance"):
            fail(f"`{identifier}` : aucune provenance declaree")
    return set(declared.values())


def check_orphan_files(declared_files: set[str]) -> None:
    """Aucun fichier d'image du dossier n'echappe au manifeste."""
    for path in sorted(UI.iterdir()):
        if path.suffix.lower() in (".jpg", ".jpeg", ".png") and path.name not in declared_files:
            fail(
                f"{path.name} n'est declare par aucune entree du manifeste. Une image deposee a "
                f"la main ne vient d'aucune page, et rien ne dit d'ou elle sort."
            )


def check_code_keys(declared_files: set[str]) -> None:
    """Les noms de fichiers cites par le C++ et ceux du manifeste sont les memes, dans les deux sens."""
    if not NAMING_SOURCE.is_file():
        fail(f"{NAMING_SOURCE.relative_to(ROOT)} absent : plus rien ne relie les images au code")
        return
    used = set(
        re.findall(r'"([A-Za-z0-9_-]+\.(?:jpe?g|png))"', NAMING_SOURCE.read_text(encoding="utf-8"))
    )
    if not used:
        fail(f"aucun nom d'illustration dans {NAMING_SOURCE.relative_to(ROOT)} (lecture cassee ?)")
        return
    for name in sorted(used - declared_files):
        fail(f"`{name}` nomme par le code, absent du manifeste")
    for name in sorted(declared_files - used):
        fail(f"`{name}` livre et declare, mais nomme par aucun code")


def main() -> None:
    manifest = read_manifest()
    declared_files = check_illustrations(manifest)
    check_orphan_files(declared_files)
    check_code_keys(declared_files)

    if errors:
        for message in errors:
            print(f"check_ui_assets : {message}", file=sys.stderr)
        sys.exit(1)
    print(
        f"check_ui_assets : {len(declared_files)} illustration(s) conforme(s) au manifeste "
        f"et nommee(s) par le code."
    )


if __name__ == "__main__":
    main()
