#!/usr/bin/env python3
# SPDX-FileCopyrightText: 2026 Valentin Eloy
# SPDX-License-Identifier: GPL-3.0-or-later
"""Pose dans chaque `.ui` d'identite le theme que Qt Designer doit appliquer (LOT-85, EX-IHM-006).

Qt Designer n'applique JAMAIS une feuille de style d'application : `theme-identity.qss` est posee a
l'execution sur la pile d'ecrans (`MainWindow.cpp`), et Designer, qui n'execute pas le jeu, affiche
donc des planches sans typographie ni couleurs de texte. Une maquette qu'on ne voit qu'a moitie ne
s'edite pas.

Ce que Designer applique, en revanche, c'est la propriete `styleSheet` d'un formulaire. Ce script y
depose la tranche de la feuille RESOLUE qui cadre cet ecran.

## Les trois pieges, et ce qui les tient

1. **La feuille du jeu est un gabarit** : 108 marqueurs d'identite a substituer. La resolution est
   faite par `Source/Tools/IdentityQss` -- du C++, qui appelle le code du jeu -- et non ici : une
   seconde implementation de la substitution aurait derive de la premiere en silence, exactement ce
   que `check_design_tokens.py` rend impossible entre la maquette et le code.

2. **Le facteur d'agrandissement**. La feuille resolue est figee a x2, celui des maquettes. Le jeu
   recalcule le sien avec la hauteur de la fenetre : laisser cette propriete en place la ferait
   PRIMER sur la feuille de la pile (une feuille de widget l'emporte sur celle d'un ancetre) et
   casserait la mise a l'echelle. Chaque ecran l'efface donc juste apres `setupUi` -- ce que
   `check_ui_designer.py` verifie.

3. **La portee de la fiche n'est pas son nom**. Les regles du RPG sont cadrees par `#RpgScreenHost`,
   le conteneur qui accueille l'ecran dans le jeu. Dans Designer, la planche EST la racine du
   formulaire, et `#RpgScreenHost` n'y designerait rien. Le prefixe est donc reecrit vers le nom du
   formulaire -- la seule transformation que ce script s'autorise sur les regles.

Usage :
    python scripts/sync_ui_theme.py            # ecrit les .ui
    python scripts/sync_ui_theme.py --check    # code de sortie non nul si divergence
"""

from __future__ import annotations

import hashlib
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
THEMES = ROOT / "Source" / "Elements" / "Themes"
TEMPLATE = THEMES / "theme-identity.qss"
RESOLVED = THEMES / "theme-identity.resolved.qss"
TOKENS = ROOT / "Source" / "HMI" / "Interface" / "DesignTokens.cpp"
UI = ROOT / "Source" / "Elements" / "UI"

# Formulaire -> portee dans la feuille du jeu. Egale au nom du formulaire partout SAUF pour la
# planche : dans le jeu elle vit DANS `RpgScreenHost`, qui porte ses regles ; dans Designer elle est
# la racine. C'est la seule exception, et l'ecrire ici l'empeche d'etre devinee.
SCOPE_BY_FORM = {
    "MainMenu": "MainMenu",
    "OptionsPage": "OptionsPage",
    "CreditsScreen": "CreditsScreen",
    "PauseScreen": "PauseScreen",
    "RpgCharacterSheetPlate": "RpgScreenHost",
    "RpgInventoryScreen": "RpgScreenHost",
    "RpgJournalScreen": "RpgScreenHost",
    "RpgWorldMapScreen": "RpgScreenHost",
    "RpgDialogueScreen": "RpgScreenHost",
    "RpgMerchantScreen": "RpgScreenHost",
    "RpgGuildBoardScreen": "RpgScreenHost",
    "RpgCombatHudScreen": "RpgScreenHost",
    "RpgTeamSheetScreen": "RpgScreenHost",
}

errors: list[str] = []


def fail(message: str) -> None:
    errors.append(message)


def sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


def read_resolved():
    """Corps de la feuille resolue et les champs de son en-tete, ou None si elle est absente."""
    if not RESOLVED.exists():
        fail("{0} absent : lancer `powershell -File scripts/designer.ps1`, qui le produit par "
             "Source/Tools/IdentityQss.".format(RESOLVED.relative_to(ROOT)))
        return None
    text = RESOLVED.read_text(encoding="utf-8")
    header = re.match(r"/\*(.*?)\*/", text, re.DOTALL)
    if not header:
        fail("{0} : en-tete genere introuvable.".format(RESOLVED.name))
        return None
    fields = dict(re.findall(r"^\s*([a-z-]+):\s*(\S+)\s*$", header.group(1), re.MULTILINE))
    return text[header.end():], fields


def check_freshness(fields) -> None:
    """La feuille resolue decrit-elle encore le gabarit et les jetons d'aujourd'hui ?

    C'est le controle qui compte, et le seul que la CI puisse faire : le runner n'a pas Qt et ne
    pourrait pas rejouer la resolution. Sans lui, une retouche de teinte laisserait la feuille
    resolue en arriere et Designer montrerait un jeu qui n'existe plus.
    """
    for field, source in (("source-template", TEMPLATE), ("source-tokens", TOKENS)):
        expected = sha256(source)
        found = fields.get(field)
        if found != expected:
            fail("{0} : perime ({1} vaut {2}, {3} a pour empreinte {4}). Relancer "
                 "`powershell -File scripts/designer.ps1`.".format(
                     RESOLVED.name, field, found, source.name, expected))


def strip_comments(qss: str) -> str:
    return re.sub(r"/\*.*?\*/", "", qss, flags=re.DOTALL)


def rules(qss: str):
    """Les regles de premier niveau, en couples (selecteurs, corps).

    QSS n'imbrique pas les accolades : un decoupage sur les blocs suffit, et un analyseur complet
    serait du luxe ici.
    """
    found = []
    for match in re.finditer(r"([^{}]+)\{([^{}]*)\}", qss, re.DOTALL):
        selectors = " ".join(match.group(1).split())
        if selectors:
            found.append((selectors, match.group(2)))
    return found


def slice_for(qss: str, scope: str, form: str) -> str:
    """Les regles cadrees par @p scope, reecrites vers le nom du formulaire."""
    kept = []
    boundary = r"#" + re.escape(scope) + r"(?![A-Za-z0-9_])"
    for selectors, body in rules(qss):
        # Une regle peut servir plusieurs ecrans (`#MainMenu, #OptionsPage, ...`) : on ne garde que
        # les selecteurs de CET ecran, sans quoi le formulaire porterait les regles des autres.
        mine = [s.strip() for s in selectors.split(",") if re.match(boundary, s.strip())]
        if not mine:
            continue
        if scope != form:
            mine = [re.sub(r"^#" + re.escape(scope), "#" + form, s) for s in mine]
        kept.append(", ".join(mine) + " {" + body.rstrip() + "\n}")
    return "\n\n".join(kept)


def xml_escape(text: str) -> str:
    return text.replace("&", "&amp;").replace("<", "&lt;").replace(">", "&gt;")


PROPERTY_RE = r"\n[ \t]*<property name=\"styleSheet\">.*?</property>"


def inject(path: Path, form: str, sheet: str, text: str) -> str:
    """Le contenu du .ui avec sa propriete `styleSheet` a jour, posee sur le widget racine.

    Une retouche TEXTUELLE, et non une reecriture par un analyseur XML : celui-ci effacerait les
    commentaires que les `.ui` portent encore, et ce script n'a pas a decider de leur sort.
    """
    opening = '<widget class="QWidget" name="{0}">'.format(form)
    if opening not in text:
        fail("{0} : widget racine `{1}` introuvable.".format(path.name, form))
        return text

    prop = ('\n  <property name="styleSheet">\n'
            '   <string notr="true">' + xml_escape(sheet) + "</string>\n"
            "  </property>")

    head, _, tail = text.partition(opening)
    existing = re.match(PROPERTY_RE, tail, re.DOTALL)
    if existing:
        tail = tail[existing.end():]
    return head + opening + prop + tail


def main() -> int:
    check = "--check" in sys.argv[1:]

    resolved = read_resolved()
    if resolved is None:
        return report()
    body, fields = resolved
    check_freshness(fields)
    body = strip_comments(body)

    for form, scope in sorted(SCOPE_BY_FORM.items()):
        path = UI / (form + ".ui")
        if not path.exists():
            fail("{0}.ui : introuvable.".format(form))
            continue
        sheet = slice_for(body, scope, form)
        if not sheet:
            fail("{0}.ui : aucune regle cadree par #{1} dans la feuille resolue.".format(form,
                                                                                         scope))
            continue
        current = path.read_text(encoding="utf-8")
        updated = inject(path, form, sheet, current)
        if updated == current:
            continue
        if check:
            fail("{0}.ui : propriete styleSheet absente ou perimee. Lancer "
                 "`python scripts/sync_ui_theme.py`.".format(form))
        else:
            path.write_text(updated, encoding="utf-8", newline="\n")
            print("sync_ui_theme : {0}.ui mis a jour ({1} caracteres).".format(form, len(sheet)))

    return report()


def report() -> int:
    for message in errors:
        print("sync_ui_theme : " + message, file=sys.stderr)
    return 1 if errors else 0


if __name__ == "__main__":
    sys.exit(main())
