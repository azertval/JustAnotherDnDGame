# SPDX-FileCopyrightText: 2026 Valentin Eloy
# SPDX-License-Identifier: GPL-3.0-or-later
"""Garde-fou : la palette d'identite des maquettes et celle du code ne doivent pas diverger.

Les planches de `.design-mockups/` sont la matiere de conception des ecrans du jeu (`LOT-68`,
`EX-IHM-070`), et `hmi::identityTokens()` est ce que le jeu peint reellement. Les deux portent la
MEME palette, ecrite deux fois : une fois en CSS pour la planche, une fois en C++ pour le theme.

Rien ne les reliait. Une retouche de teinte d'un cote laissait l'autre en arriere sans qu'aucune
Pull Request ne le signale, et la maquette cessait silencieusement de decrire le jeu -- ce qui lui
retire tout interet, puisqu'on ne s'y refere plus qu'en la croyant fidele.

Ce controle est purement textuel : aucun binaire a construire, aucune dependance. Meme motif que
`check_qt_version_pin.py` (version de Qt CMake/CI) ou `check_demo_sequence.py`.

Usage :
    python scripts/check_design_tokens.py     # code de sortie non nul si divergence
"""

from __future__ import annotations

import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
MOCKUP = ROOT / ".design-mockups" / "_page_head.html"
TOKENS = ROOT / "Source" / "HMI" / "Interface" / "DesignTokens.cpp"

# Role du jeton -> nom de la variable CSS de la planche (portee identite, bloc `.screen`).
# Le nom CSS est celui du dessin, le nom C++ celui du role : c'est la correspondance qu'on fige.
CSS_VARIABLE_BY_ROLE = {
    "background": None,  # pas une variable : la propriete `background` du bloc `.screen`
    "text": None,  # pas une variable : la propriete `color` du bloc `.screen`
    "accent": "accent",
    "surface": "surface",
    "surfaceAlt": "surfaceAlt",
    "frameEdge": "edge",
    "frameOrnament": "ornament",
    "frameShadow": "shadow",
    "textMuted": "muted",
}


def fail(message: str) -> None:
    print(f"check_design_tokens : {message}", file=sys.stderr)
    sys.exit(1)


def read_mockup_palette() -> dict[str, str]:
    """Couleurs de la portee identite de la planche, par nom de variable CSS."""
    text = MOCKUP.read_text(encoding="utf-8")
    # La page declare DEUX blocs `.screen` : celui du chrome (mise a l'echelle des planches sur
    # la page) et celui de l'habillage, qui devient la section identite de theme.qss. Le bon
    # est designe par son contenu -- il est le seul a declarer la palette -- plutot que par sa
    # position, qu'une reorganisation de la feuille suffirait a changer.
    blocks = [b for b in re.findall(r"\.screen\s*\{(.*?)\}", text, re.DOTALL)
              if "--accent" in b]
    if len(blocks) != 1:
        fail(
            f"{len(blocks)} bloc(s) `.screen` declarant `--accent` dans "
            f"{MOCKUP.relative_to(ROOT)} (un seul attendu)"
        )
    block = blocks[0]

    palette: dict[str, str] = {}
    for name, value in re.findall(r"--([A-Za-z][A-Za-z0-9]*)\s*:\s*(#[0-9a-fA-F]{6})", block):
        palette[name] = value.lower()
    for prop, key in (("background", "__background__"), ("color", "__color__")):
        prop_match = re.search(rf"(?<!-){prop}\s*:\s*(#[0-9a-fA-F]{{6}})\s*;", block)
        if prop_match:
            palette[key] = prop_match.group(1).lower()
    return palette


def read_code_palette() -> dict[str, str]:
    """Couleurs de `buildIdentityTokens()`, par role, au format `#rrggbb`."""
    text = TOKENS.read_text(encoding="utf-8")
    start = text.find("buildIdentityTokens")
    if start < 0:
        fail(f"`buildIdentityTokens` introuvable dans {TOKENS.relative_to(ROOT)}")
    end = text.find("buildEditorDarkTokens", start)
    body = text[start : end if end > 0 else len(text)]

    palette: dict[str, str] = {}
    pattern = (
        r"tokens\.color\.(\w+)\s*=\s*DesignColor\{"
        r"\.r\s*=\s*0x([0-9a-fA-F]{2}),\s*\.g\s*=\s*0x([0-9a-fA-F]{2}),\s*\.b\s*=\s*0x([0-9a-fA-F]{2})"
    )
    for role, red, green, blue in re.findall(pattern, body):
        palette[role] = f"#{red}{green}{blue}".lower()
    return palette


def main() -> int:
    if not MOCKUP.is_file():
        fail(f"{MOCKUP.relative_to(ROOT)} introuvable")
    if not TOKENS.is_file():
        fail(f"{TOKENS.relative_to(ROOT)} introuvable")

    mockup = read_mockup_palette()
    code = read_code_palette()

    divergences: list[str] = []
    checked = 0
    for role, css_name in CSS_VARIABLE_BY_ROLE.items():
        if role not in code:
            divergences.append(f"  role `{role}` absent de buildIdentityTokens()")
            continue
        if role == "background":
            key, label = "__background__", "`.screen { background }`"
        elif role == "text":
            key, label = "__color__", "`.screen { color }`"
        else:
            key, label = css_name, f"`--{css_name}`"
        if key not in mockup:
            divergences.append(f"  {label} absent de la planche")
            continue
        checked += 1
        if mockup[key] != code[role]:
            divergences.append(
                f"  {label} = {mockup[key]} sur la planche, "
                f"mais tokens.color.{role} = {code[role]} dans le code"
            )

    if divergences:
        print(
            "check_design_tokens : la palette des maquettes et celle du code divergent.\n"
            + "\n".join(divergences)
            + "\n\nCorriger l'un ou l'autre -- une maquette qui ne decrit plus le jeu ne sert\n"
            "plus a decider quoi que ce soit. Sources :\n"
            f"  {MOCKUP.relative_to(ROOT)}\n"
            f"  {TOKENS.relative_to(ROOT)}",
            file=sys.stderr,
        )
        return 1

    print(f"Palette d'identite coherente entre les maquettes et le code ({checked} couleurs).")
    return 0


if __name__ == "__main__":
    sys.exit(main())
