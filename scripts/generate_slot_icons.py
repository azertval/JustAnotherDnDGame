# SPDX-FileCopyrightText: 2026 Valentin Eloy
# SPDX-License-Identifier: GPL-3.0-or-later
"""Genere les huit icones d'emplacement d'equipement, en SVG.

Les huit emplacements sont ceux de `core::Equipment` (`LOT-34`) et de la table du chassis
(`hmi::EQUIPMENT_SLOTS`, `LOT-68`) : tete, torse, mains, pieds, main directrice, main secondaire,
amulette, anneau. Un neuvieme emplacement se declare ICI et nulle part ailleurs -- la table
ci-dessous est la seule liste, et le fichier qu'elle produit porte le nom de sa cle de traduction.

## Pourquoi un generateur, et pas huit fichiers ecrits a la main

Les huit icones partagent leur grille (24 x 24), leur epaisseur de trait et leurs terminaisons.
Ecrites a la main, elles divergent a la premiere retouche : une devient plus epaisse que les sept
autres, et personne ne voit pourquoi la ligne « Mains » pese plus lourd que la ligne « Pieds ». Le
trace propre a chaque emplacement est la SEULE chose qui varie, et la seule que la table ecrit.

## Pourquoi `currentColor`, et pas la teinte

`EX-IHM-051` et le `LOT-76` : un asset ne fige pas ses couleurs hors des jetons. Une icone qui
porterait `#907030` en dur devrait etre reexportee a chaque retouche de palette, et le jour ou
quelqu'un oublierait, elle resterait juste -- mais fausse. Le trait est donc `currentColor`, que le
chargeur substitue par le role demande. Un oubli de substitution donne du NOIR, c'est-a-dire une
panne qui se voit, et non une teinte plausible qui ment.

## Pourquoi du SVG ici, alors que le LOT-76 trace ses ornements

Un ornement de cadre s'ETIRE : un cabochon sur un panneau bas s'ovalise, un bandeau deforme ses
ailes. Il doit donc etre redessine a chaque taille, et c'est ce que fait `ParchmentOrnaments`. Une
icone d'emplacement, elle, ne s'etire jamais : elle occupe un carre, et un seul. Ce qui la
distingue n'est pas sa taille mais son SUJET -- un heaume, une botte -- et un sujet se dessine une
fois. Les deux regles ne se contredisent pas, elles repondent a deux questions differentes.

Usage :
    python scripts/generate_slot_icons.py     # reecrit Source/Elements/Assets/Icons/slot/
"""

from __future__ import annotations

import pathlib
import sys

RACINE = pathlib.Path(__file__).resolve().parent.parent
DESTINATION = RACINE / "Source" / "Elements" / "Assets" / "Icons" / "slot"

# La grille est celle des icones de l'habillage : 24 unites de cote, trait d'une unite et demie,
# terminaisons et jointures arrondies. Ces trois valeurs valent pour les huit -- c'est ce qui fait
# qu'elles se lisent comme une famille.
COTE = 24
EPAISSEUR = "1.6"

# Cle de traduction -> (nom de fichier, ce que l'icone montre, traces, cercles).
#
# Le nom de fichier est celui de la cle, sans son espace de noms : `rpg.slot.main_hand` donne
# `main-hand.svg`. Le lien entre les deux ne se devine donc pas, il se lit.
#
# Deux emplacements portent un cercle en plus de leurs traces : un pendentif et un anneau se
# dessinent rond, et un arc de cercle en `<path>` serait ici moins lisible que le cercle qu'il
# imite.
EMPLACEMENTS = {
    "rpg.slot.head": (
        "head",
        "un heaume ferme, vu de face, avec sa fente de vue",
        ["M4 13a8 8 0 0 1 16 0v6H4z", "M4 13h16", "M12 13v6"],
        [],
    ),
    "rpg.slot.torso": (
        "torso",
        "une cuirasse a epaulieres, echancree au col",
        ["M8 3l4 3 4-3 4 3v6l-2 1v8H6v-8l-2-1V6z", "M9 12h6"],
        [],
    ),
    "rpg.slot.hands": (
        "hands",
        "un gantelet, quatre doigts et le pouce",
        [
            "M7 11V5a1.5 1.5 0 0 1 3 0v6",
            "M10 11V4a1.5 1.5 0 0 1 3 0v7",
            "M13 11V5a1.5 1.5 0 0 1 3 0v8",
            "M16 9a1.5 1.5 0 0 1 3 0v6a6 6 0 0 1-12 0v-2",
        ],
        [],
    ),
    "rpg.slot.feet": (
        "feet",
        "une botte de profil, semelle vers la droite",
        ["M7 3v9c0 2 1 3 3 3h7a2 2 0 0 1 2 2v3H7z", "M7 12h5"],
        [],
    ),
    "rpg.slot.main_hand": (
        "main-hand",
        "une epee en diagonale, garde en bas a droite",
        ["M4 20l3-3", "M6 18l11-11 3-3v3l-3 3-8 8z", "M15 6l3 3"],
        [],
    ),
    "rpg.slot.off_hand": (
        "off-hand",
        "un ecu, avec sa bande verticale",
        ["M12 3l8 2.5v6c0 5-3.5 8.5-8 10-4.5-1.5-8-5-8-10v-6z", "M12 7v10"],
        [],
    ),
    "rpg.slot.amulet": (
        "amulet",
        "une chaine ouverte et son pendentif rond",
        ["M6 3c1 5 3 7 6 7s5-2 6-7", "M12 10v2"],
        [("12", "16", "4.5")],
    ),
    "rpg.slot.ring": (
        "ring",
        "un anneau surmonte de son chaton",
        ["M9 7l3-3 3 3"],
        [("12", "14", "6")],
    ),
}

GABARIT = """<?xml version="1.0" encoding="UTF-8"?>
<!-- SPDX-FileCopyrightText: 2026 Valentin Eloy
     SPDX-License-Identifier: GPL-3.0-or-later

     {libelle} : {description}.

     GENERE par scripts/generate_slot_icons.py -- ne pas retoucher a la main. La retouche serait
     perdue a la prochaine execution, et l'icone divergerait des sept autres.

     Le trait est `currentColor` : le chargeur y substitue le role demande de la portee identite
     (EX-IHM-051). Rendue telle quelle, l'icone sort en NOIR -- une panne qui se voit, et non une
     teinte plausible qui ment. -->
<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 {cote} {cote}" width="{cote}" height="{cote}"
     fill="none" stroke="currentColor" stroke-width="{epaisseur}"
     stroke-linecap="round" stroke-linejoin="round">
{corps}
</svg>
"""


def ecrire() -> int:
    DESTINATION.mkdir(parents=True, exist_ok=True)
    attendus = set()

    for cle, (nom, description, traces, cercles) in EMPLACEMENTS.items():
        lignes = [f'  <path d="{d}"/>' for d in traces]
        lignes += [f'  <circle cx="{cx}" cy="{cy}" r="{r}"/>' for cx, cy, r in cercles]

        contenu = GABARIT.format(
            libelle=cle,
            description=description,
            cote=COTE,
            epaisseur=EPAISSEUR,
            corps="\n".join(lignes),
        )
        chemin = DESTINATION / f"{nom}.svg"
        chemin.write_text(contenu, encoding="utf-8", newline="\n")
        attendus.add(chemin.name)

    # Une icone restee sur le disque apres avoir quitte la table serait embarquee par le `.qrc`
    # sans que rien ne la nomme. On la SIGNALE plutot que de l'effacer : supprimer un fichier que
    # l'on n'a pas ecrit est une decision qui appartient a celui qui l'a depose.
    restes = {p.name for p in DESTINATION.glob("*.svg")} - attendus
    if restes:
        print(f"generate_slot_icons : icones sans entree dans la table : {', '.join(sorted(restes))}",
              file=sys.stderr)
        return 1

    print(f"{len(attendus)} icones d'emplacement ecrites dans {DESTINATION.relative_to(RACINE)}")
    return 0


if __name__ == "__main__":
    raise SystemExit(ecrire())
