# SPDX-FileCopyrightText: 2026 Valentin Eloy
# SPDX-License-Identifier: GPL-3.0-or-later
"""Genere les seize icones d'emplacement d'equipement, en SVG.

Les seize emplacements sont ceux de `core::EquipmentSlot` (`LOT-34`), et chaque fichier porte le
nom que `core::equipmentSlotName` leur donne : `ring-left.svg` pour `EquipmentSlot::RingLeft`. Le
lien entre le code et le fichier ne se devine donc pas, il se LIT -- une correspondance obtenue par
transformation de chaine casse en silence au premier emplacement hors regle.

Un dix-septieme emplacement se declare ICI et nulle part ailleurs : la table ci-dessous est la
seule liste.

## Pourquoi un generateur, et pas seize fichiers ecrits a la main

Les seize icones partagent leur grille (24 x 24), leur epaisseur de trait et leurs terminaisons.
Ecrites a la main, elles divergent a la premiere retouche : une devient plus epaisse que les
quinze autres, et personne ne voit pourquoi la ligne << Mains >> pese plus lourd que << Pieds >>.
Le trace propre a chaque emplacement est la SEULE chose qui varie, et la seule que la table ecrit.

Deux emplacements -- les deux anneaux -- partagent leur trace : un anneau gauche et un anneau droit
se dessinent pareil. La table le dit explicitement plutot que de laisser un fichier en copier un
autre, et les deux fichiers restent produits, parce que le code les demande par leur nom.

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
# terminaisons et jointures arrondies. Ces trois valeurs valent pour les seize -- c'est ce qui fait
# qu'elles se lisent comme une famille.
COTE = 24
EPAISSEUR = "1.6"

# Traces partages, nommes pour que la table dise qu'ils le sont.
ANNEAU = (
    "un anneau surmonte de son chaton",
    ["M9 7l3-3 3 3"],
    [("12", "14", "6")],
)

# Nom d'emplacement (`core::equipmentSlotName`) -> (description, traces, cercles).
#
# L'ordre est celui du modele. Les cercles sont a part parce qu'un arc de cercle en `<path>` serait
# ici moins lisible que le cercle qu'il imite.
EMPLACEMENTS = {
    "head": (
        "un heaume ferme, vu de face, avec sa fente de vue",
        ["M4 13a8 8 0 0 1 16 0v6H4z", "M4 13h16", "M12 13v6"],
        [],
    ),
    "neck": (
        "une chaine ouverte et son pendentif rond",
        ["M6 3c1 5 3 7 6 7s5-2 6-7", "M12 10v2"],
        [("12", "16", "4.5")],
    ),
    "cloak": (
        "une cape agrafee au col, retombant en deux pans",
        ["M8 4l4 2 4-2", "M8 4C5 6 4 11 4 20h6l1-9", "M16 4c3 2 4 7 4 16h-6l-1-9"],
        [],
    ),
    "torso": (
        "une cuirasse a epaulieres, echancree au col",
        ["M8 3l4 3 4-3 4 3v6l-2 1v8H6v-8l-2-1V6z", "M9 12h6"],
        [],
    ),
    "belt": (
        "une ceinture et sa boucle carree",
        ["M2 10h7", "M15 10h7", "M2 14h7", "M15 14h7", "M9 8h6v8H9z", "M12 10v4"],
        [],
    ),
    "hands": (
        "un gantelet, quatre doigts et le pouce",
        [
            "M7 11V5a1.5 1.5 0 0 1 3 0v6",
            "M10 11V4a1.5 1.5 0 0 1 3 0v7",
            "M13 11V5a1.5 1.5 0 0 1 3 0v8",
            "M16 9a1.5 1.5 0 0 1 3 0v6a6 6 0 0 1-12 0v-2",
        ],
        [],
    ),
    "ring-left": ANNEAU,
    "ring-right": ANNEAU,
    "main-hand": (
        "une epee en diagonale, garde en bas a droite",
        ["M4 20l3-3", "M6 18l11-11 3-3v3l-3 3-8 8z", "M15 6l3 3"],
        [],
    ),
    "off-hand": (
        "un ecu, avec sa bande verticale",
        ["M12 3l8 2.5v6c0 5-3.5 8.5-8 10-4.5-1.5-8-5-8-10v-6z", "M12 7v10"],
        [],
    ),
    "ranged": (
        "un arc bande, fleche encochee",
        ["M7 3a16 16 0 0 1 0 18", "M7 4l0 16", "M4 12h16", "M16 9l4 3-4 3"],
        [],
    ),
    "ammunition": (
        "trois fleches dans leur carquois",
        ["M8 3v9", "M12 2v10", "M16 3v9", "M6 12h12l-1 9H7z", "M6 16h12"],
        [],
    ),
    "feet": (
        "une botte de profil, semelle vers la droite",
        ["M7 3v9c0 2 1 3 3 3h7a2 2 0 0 1 2 2v3H7z", "M7 12h5"],
        [],
    ),
    "bracers": (
        "un brassard lace sur l'avant-bras",
        ["M7 4h10l-1 16H8z", "M9 8h6", "M9 12h6", "M9 16h6"],
        [],
    ),
    "pouch": (
        "une bourse fermee par son cordon",
        ["M9 3l-1 4", "M15 3l1 4", "M8 7h8c2 3 3 6 3 9a4 4 0 0 1-4 4H9a4 4 0 0 1-4-4c0-3 1-6 3-9z"],
        [],
    ),
    "trinket": (
        "un talisman a trois branches, suspendu",
        ["M12 3v4", "M12 11l4 7H8z"],
        [("12", "9", "2.4")],
    ),
}

GABARIT = """<?xml version="1.0" encoding="UTF-8"?>
<!-- SPDX-FileCopyrightText: 2026 Valentin Eloy
     SPDX-License-Identifier: GPL-3.0-or-later

     Emplacement `{nom}` (core::equipmentSlotName) : {description}.

     GENERE par scripts/generate_slot_icons.py. Ne pas retoucher a la main : la retouche serait
     perdue a la prochaine execution, et l'icone divergerait des quinze autres.

     Le trait est `currentColor` ; le chargeur y substitue le role demande de la portee identite
     (EX-IHM-051). Rendue telle quelle, l'icone sort en NOIR : une panne qui se voit, plutot
     qu'une teinte plausible qui ment.

     AUCUN double tiret dans ce commentaire. XML l'interdit, QSvgRenderer refuse alors le fichier
     en silence, et l'icone sort vide sans qu'aucune erreur ne soit levee. -->
<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 {cote} {cote}" width="{cote}" height="{cote}">
  <!-- Les attributs de presentation sont sur un GROUPE, jamais sur la racine : le moteur SVG de
       Qt (SVG Tiny 1.2) ne les herite pas depuis la racine. -->
  <g fill="none" stroke="currentColor" stroke-width="{epaisseur}"
     stroke-linecap="round" stroke-linejoin="round">
{corps}
  </g>
</svg>
"""


def ecrire() -> int:
    DESTINATION.mkdir(parents=True, exist_ok=True)
    attendus = set()

    for nom, (description, traces, cercles) in EMPLACEMENTS.items():
        lignes = [f'    <path d="{d}"/>' for d in traces]
        lignes += [f'    <circle cx="{cx}" cy="{cy}" r="{r}"/>' for cx, cy, r in cercles]

        contenu = GABARIT.format(
            nom=nom,
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
        print(
            "generate_slot_icons : icones sans entree dans la table : "
            + ", ".join(sorted(restes)),
            file=sys.stderr,
        )
        return 1

    print(f"{len(attendus)} icones d'emplacement ecrites dans {DESTINATION.relative_to(RACINE)}")
    return 0


if __name__ == "__main__":
    raise SystemExit(ecrire())
