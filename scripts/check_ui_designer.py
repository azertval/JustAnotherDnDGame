#!/usr/bin/env python3
# SPDX-FileCopyrightText: 2026 Valentin Eloy
# SPDX-License-Identifier: GPL-3.0-or-later
"""Garde-fou : les `.ui` d'identite doivent rester ouvrables ET entierement visibles dans Designer.

Les ecrans a maquette gravee se modifient dans l'IHM de Qt (`LOT-85`, `EX-IHM-006`) : c'est ce que
la table nomme `RpgRendering::DesignerPlate`. Cette propriete ne tient a rien d'automatique -- elle
repose sur trois liens que rien ne relie, et qui lachent en SILENCE.

Ce que ce controle rend impossible :

- **un widget promu que le plugin n'expose pas.** Designer le rend alors comme sa classe de base :
  un rectangle gris a la place du parchemin. Le `.ui` continue de compiler, le jeu continue de bien
  s'afficher, et seule l'edition est cassee -- donc personne ne le voit avant d'ouvrir Designer ;
- **un widget declare au plugin que rien ne construit**, ou dont l'en-tete a ete deplace : le
  plugin ne se charge plus, sans message, et Designer se contente de ne rien proposer ;
- **un ecran qui garde le theme fige** que son `.ui` porte pour Designer. Cette feuille est resolue
  au facteur x2 ; posee sur le widget, elle prime sur celle de la pile d'ecrans et la mise a
  l'echelle cesse de fonctionner. Le defaut ne se voit qu'en redimensionnant la fenetre ;
- **un commentaire XML reintroduit dans un `.ui`.** Designer les efface a l'enregistrement : ce qui
  y serait ecrit disparaitrait au premier geste d'edition, et la justification doit donc vivre dans
  les en-tetes C++ ;
- **un element que le `.ui` a PERDU** alors que le C++ le nomme. Designer reecrit le fichier
  entier a l'enregistrement, et une reecriture peut laisser des morceaux en route : c'est arrive a
  `CreditsScreen.ui`, ampute de huit widgets par un enregistrement. Le compilateur finit par le
  dire -- mais seulement sur un poste qui a Qt, et longtemps apres le geste qui l'a cause.

Le theme lui-meme (fraicheur de la feuille resolue, exactitude de la tranche injectee) est verifie
par `scripts/sync_ui_theme.py --check`, qui le produit : le controler ici en aurait fait une seconde
lecture du meme fichier.

Aucune dependance, et purement textuel : le runner de CI n'a ni Qt ni compilateur, et ne pourrait
pas construire le plugin pour l'interroger.

Usage :
    python scripts/check_ui_designer.py     # code de sortie non nul si divergence
"""

from __future__ import annotations

import re
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
# La liste des ecrans d'identite est ECRITE UNE FOIS, dans le script qui leur injecte le theme.
# La recopier ici aurait cree la divergence que ces garde-fous existent pour empecher.
from sync_ui_theme import SCOPE_BY_FORM  # noqa: E402

ROOT = Path(__file__).resolve().parent.parent
UI = ROOT / "Source" / "Elements" / "UI"
INTERFACE = ROOT / "Source" / "HMI" / "Interface"
PLUGIN = ROOT / "Source" / "Tools" / "DesignerPlugin" / "IdentityWidgetsPlugin.cpp"

errors: list[str] = []


def fail(message: str) -> None:
    errors.append(message)


def plugin_catalogue(text: str):
    """Les widgets que le plugin declare, et ceux qu'il construit reellement.

    Deux relevés et non un seul : une entree du catalogue sans fabrique compilerait, et Designer
    offrirait un widget qui ne s'instancie pas.
    """
    declared = set(re.findall(r'add\(\s*"([^"]+)"', text))
    built = set(re.findall(r"new\s+(hmi::[A-Za-z_][A-Za-z0-9_]*)\s*\(", text))
    return declared, built


def main() -> int:
    if not PLUGIN.exists():
        fail("plugin introuvable ({0}).".format(PLUGIN.relative_to(ROOT)))
        return report()

    plugin_text = PLUGIN.read_text(encoding="utf-8")
    declared, built = plugin_catalogue(plugin_text)

    # Auto-controle : un releve vide rendrait tout le reste vert PAR VACUITE, et un controle qui ne
    # peut plus rien constater est pire que pas de controle -- c'est la panne du LOT-78.
    if not declared:
        fail("aucun widget releve dans {0} : le format du catalogue a change, et ce controle ne "
             "verifie plus rien.".format(PLUGIN.name))
        return report()

    for widget in sorted(declared - built):
        fail("{0} : declare au catalogue mais aucune fabrique ne le construit.".format(widget))

    promoted_seen = 0
    for form in sorted(SCOPE_BY_FORM):
        path = UI / (form + ".ui")
        if not path.exists():
            fail("{0}.ui : introuvable.".format(form))
            continue
        text = path.read_text(encoding="utf-8")

        # 1. Les widgets promus sont-ils tous offerts par le plugin ?
        for cls, header in re.findall(
                r"<customwidget>\s*<class>([^<]+)</class>.*?<header>([^<]+)</header>",
                text, re.DOTALL):
            promoted_seen += 1
            if cls not in declared:
                fail("{0}.ui promeut {1}, que le plugin Qt Designer n'expose pas : Designer le "
                     "rendra comme sa classe de base.".format(form, cls))
            if not (ROOT / "Source" / header).exists():
                fail("{0}.ui : l'en-tete {1} declare pour {2} n'existe pas.".format(
                    form, header, cls))

        # 2. Le theme fige est-il bien jete par l'ecran ?
        #
        # Les huit ecrans engendres n'ont pas de fichier a leur nom : leur comportement commun,
        # effacement compris, vit dans RpgUiScreen.cpp. Le controle regarde donc la ou le code est.
        source = INTERFACE / (form + ".cpp")
        if not source.exists():
            source = INTERFACE / "RpgUiScreen.cpp"
        if '<property name="styleSheet">' in text:
            if not source.exists():
                fail("{0}.cpp : introuvable, alors que {0}.ui porte un theme pour Designer.".format(
                    form))
            elif "setStyleSheet(QString())" not in source.read_text(encoding="utf-8"):
                fail("{0}.cpp : n'efface pas le theme que {0}.ui porte pour Designer "
                     "(setStyleSheet(QString()) apres setupUi). Fige au facteur x2, il primerait "
                     "sur la feuille de la pile d'ecrans et casserait la mise a l'echelle.".format(
                         form))

        # 3. Le .ui porte-t-il encore tout ce que le C++ y nomme ?
        #
        # Qt Designer REECRIT le fichier a l'enregistrement, et une reecriture peut perdre des
        # elements : c'est arrive a CreditsScreen.ui, ampute de huit widgets. Le compilateur le
        # dit -- mais seulement sur un poste qui a Qt, et seulement pour les widgets qu'un `_ui->`
        # nomme. Ce controle-ci le dit sur le runner, avant le build.
        propre = INTERFACE / (form + ".cpp")
        if propre.exists():
            code = propre.read_text(encoding="utf-8")
            declares = set(re.findall(r'<(?:widget|layout|spacer|action)[^>]*name="([^"]+)"', text))
            for nom in sorted(set(re.findall(r"_ui->(\w+)", code))):
                if nom in ("setupUi", "retranslateUi") or nom in declares:
                    continue
                fail("{0}.cpp nomme `_ui->{1}`, que {0}.ui ne declare pas : le formulaire a "
                     "perdu cet element -- une reecriture par Designer, le plus souvent.".format(
                         form, nom))

        # 4. Les commentaires XML ont-ils ete reintroduits ?
        if re.search(r"<!--", text):
            fail("{0}.ui contient un commentaire XML : Qt Designer l'effacera au premier "
                 "enregistrement. La justification vit dans les en-tetes C++.".format(form))

    if promoted_seen == 0:
        fail("aucun widget promu releve dans les .ui d'identite : le format a change, et ce "
             "controle ne verifie plus rien.")

    return report()


def report() -> int:
    for message in errors:
        print("check_ui_designer : " + message, file=sys.stderr)
    if errors:
        return 1
    print("check_ui_designer : les .ui d'identite sont editables dans Qt Designer.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
