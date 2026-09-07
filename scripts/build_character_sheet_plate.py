# SPDX-FileCopyrightText: 2026 Valentin Eloy
# SPDX-License-Identifier: GPL-3.0-or-later
"""Grave la planche de la fiche de personnage : le trait du livre, SANS son lettrage.

La planche 1 des feuilles de Tanares a ete vectorisee (`Character_Sheets_Tanares.svg`, corpus,
non versionne -- `EX-CNT-023`). Elle porte tout : l'encadrement, les rinceaux, la rose des vents,
la roue des caracteristiques, les cartouches... et **son lettrage, grave en anglais**.

Livrer la planche telle quelle aurait fige << Acrobatics >> et << BACKGROUND >> dans l'image, donc
dans les deux langues a la fois : l'ecran serait redevenu anglais le jour ou le lexique du
`LOT-30` a justement ete etendu pour le traduire. Ce script retire donc le lettrage du trace, et
ne garde que l'ornement ; les intitules sont ensuite reposes par Qt, traduits, aux MEMES
rectangles.

## Comment le lettrage se retire d'un trace

Le fichier du corpus est un **calque d'encre** : un unique `<path>` noir de 21906 sous-chemins,
issu d'une vectorisation. Il n'y a donc ni balise `<text>` a supprimer, ni calque a masquer -- une
lettre y est un sous-chemin comme un filet de cadre.

Ce qui les distingue est leur **etendue** : une lettre tient dans le rectangle de son intitule, un
filet de cadre le traverse de part en part. Le script retire donc les sous-chemins **entierement
contenus** dans l'un des rectangles `erase` de la table, et garde tous les autres. Un contre-poinson
(le trou du << o >>) est dans le meme rectangle que sa lettre, et part avec elle.

## La table est la meme des deux cotes

Les rectangles ne sont pas ecrits ici : ils viennent de `character-sheet-plate.json`, la table que
l'ecran lit aussi pour **poser** ses intitules. Une seule source, donc, pour ce qui est efface et ce
qui est reecrit : un rectangle deplace deplace les deux, et il est impossible d'effacer un intitule
sans le reposer, ni de reposer un intitule sur une gravure restee en place.

## Reconstruire un chemin dont on retire des morceaux

Un sous-chemin s'ouvre par `m dx dy` -- un deplacement **relatif** au point courant, c'est-a-dire
au dernier point du sous-chemin precedent. Retirer un sous-chemin decalerait donc tous les
suivants. Chaque sous-chemin conserve est donc reouvert en **absolu** (`M x y`), ce que le parcours
connait deja puisqu'il suit le point courant pour calculer les etendues.

Usage :
    python scripts/build_character_sheet_plate.py            # regenere la planche
    python scripts/build_character_sheet_plate.py --check    # verifie sans ecrire
"""

from __future__ import annotations

import argparse
import json
import os
import re
import shutil
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
SOURCE = ROOT / "Documentation" / "SourceBook" / "Character_Sheets_Tanares.svg"
UI = ROOT / "Source" / "Elements" / "Assets" / "UI"
TABLE = UI / "character-sheet-plate.json"
TARGET = UI / "character-sheet-plate.png"

# Facteur de rendu du masque d'encre, en multiples de la maquette (816 x 1056). A trois fois, la
# planche fait 2448 px de large : au-dela de ce qu'un ecran en montre, meme en plein ecran, et
# l'ecran la reduit ensuite avec un filtrage lisse.
PLATE_SCALE = 3

# Borne par defaut : plus haute que toute lettre de texte courant, plus basse que tout evidement.
DEFAULT_GLYPH = (30.0, 26.0)

NUMBER = re.compile(r"[-+]?(?:\d*\.\d+|\d+\.?)(?:[eE][-+]?\d+)?")
COMMAND = re.compile(r"[MmCcHhVvLlZzSsQqTtAa]")


def trouver_inkscape() -> Path | None:
    """Inkscape, dans le PATH ou a son emplacement d'installation habituel sous Windows."""
    trouve = shutil.which("inkscape")
    if trouve:
        return Path(trouve)
    for racine in (os.environ.get("ProgramFiles", ""), os.environ.get("ProgramFiles(x86)", "")):
        if not racine:
            continue
        chemin = Path(racine) / "Inkscape" / "bin" / "inkscape.exe"
        if chemin.is_file():
            return chemin
    return None


def read_ink_path(svg: str) -> str:
    """Le `d` du calque d'encre : le seul chemin peint du fichier (les deux autres decoupent)."""
    for match in re.finditer(r"<path\b(.*?)/>", svg, re.S):
        attributes = match.group(1)
        if "fill:#000000" in attributes:
            return re.search(r'\bd="(.*?)"', attributes, re.S).group(1)
    raise SystemExit("planche : aucun chemin d'encre (fill:#000000) dans %s" % SOURCE)


def commands(data: str):
    """Decoupe `d` en (lettre, nombres). Le fichier n'emploie que M C H V m c h l v z."""
    position = 0
    while True:
        match = COMMAND.search(data, position)
        if not match:
            return
        following = COMMAND.search(data, match.end())
        end = following.start() if following else len(data)
        yield match.group(0), [float(n) for n in NUMBER.findall(data[match.end():end])], \
            match.start(), end
        position = end


def subpaths(data: str):
    """Parcourt `d` et rend un dictionnaire par sous-chemin.

    `box` est calculee sur les points de controle des courbes : le polygone de controle enveloppe
    la courbe, donc un sous-chemin juge << dans le rectangle >> y est reellement.
    """
    current = [0.0, 0.0]
    start = [0.0, 0.0]
    opened: dict | None = None

    def touch(x: float, y: float) -> None:
        box = opened["box"]
        if box[0] is None:
            box[0], box[1], box[2], box[3] = x, y, x, y
        else:
            box[0] = min(box[0], x)
            box[1] = min(box[1], y)
            box[2] = max(box[2], x)
            box[3] = max(box[3], y)

    for letter, numbers, begin, end in commands(data):
        if letter in "Mm":
            if opened is not None:
                opened["end"] = begin
                yield opened
            opened = {"begin": begin, "end": end, "box": [None, None, None, None],
                      "origin": None, "tail": None}
            for index in range(0, len(numbers) - 1, 2):
                if letter == "M":
                    current = [numbers[index], numbers[index + 1]]
                else:
                    current = [current[0] + numbers[index], current[1] + numbers[index + 1]]
                if index == 0:
                    start = list(current)
                    opened["origin"] = list(current)
                    # Les paires SUIVANTES d'un `m` sont des lignes relatives implicites : elles
                    # sont conservees telles quelles, apres le `M` absolu qu'on reecrit.
                    opened["tail"] = numbers[2:]
                touch(current[0], current[1])
        elif letter in "Cc":
            for index in range(0, len(numbers) - 5, 6):
                if letter == "C":
                    points = [(numbers[index], numbers[index + 1]),
                              (numbers[index + 2], numbers[index + 3]),
                              (numbers[index + 4], numbers[index + 5])]
                else:
                    points = [(current[0] + numbers[index], current[1] + numbers[index + 1]),
                              (current[0] + numbers[index + 2], current[1] + numbers[index + 3]),
                              (current[0] + numbers[index + 4], current[1] + numbers[index + 5])]
                for x, y in points:
                    touch(x, y)
                current = list(points[2])
        elif letter in "Ll":
            for index in range(0, len(numbers) - 1, 2):
                if letter == "L":
                    current = [numbers[index], numbers[index + 1]]
                else:
                    current = [current[0] + numbers[index], current[1] + numbers[index + 1]]
                touch(current[0], current[1])
        elif letter in "Hh":
            for value in numbers:
                current[0] = value if letter == "H" else current[0] + value
                touch(current[0], current[1])
        elif letter in "Vv":
            for value in numbers:
                current[1] = value if letter == "V" else current[1] + value
                touch(current[0], current[1])
        elif letter in "Zz":
            current = list(start)
        else:
            raise SystemExit("planche : commande %r non geree" % letter)
    if opened is not None:
        opened["end"] = len(data)
        yield opened


def _decoupe_impossible() -> None:
    """Pourquoi la planche est livree en PNG, et non en SVG.

    ## Qt ne sait pas rendre ce trace

    `QSvgHandler` refuse un `<path>` de plus de **32768** elements. Il ne le tronque pas malgre son
    message (`qt.svg: Invalid path data; path truncated`) : il le **jette**, et la planche s'affiche
    entierement vide -- une panne muette, qui ne ressemble a rien d'autre qu'a un asset manquant.
    Le trace de la planche en demande 540094, soit **seize fois et demie** la limite. Inkscape le
    rend sans broncher ; la limite est propre a Qt, et c'est Qt qui affiche la fiche.

    ## Et on ne peut pas le decouper

    Decouper le trace en plusieurs `<path>` semble evident, et ne marche pas. Un remplissage --
    non-nul comme pair-impair -- se calcule sur l'ENSEMBLE des contours d'un meme chemin : le blanc
    d'un cartouche, le trou d'un anneau, l'interieur du cadre de page n'existent que parce qu'un
    autre contour du meme chemin y annule le premier. Separer deux contours qui se recouvrent
    change donc ce qui est plein et ce qui est vide.

    Trois decoupes ont ete essayees -- dans l'ordre du fichier, par arbre quaternaire sur le centre,
    puis en mariant chaque forme aux contours qu'elle contient. Les trois rendent la meme image
    fausse : bandeau d'identite, anneaux de la roue, tableau d'attaques et cadres de personnalite
    virent au pave noir. La raison de fond est visible dans la donnee : le contour du CADRE DE PAGE
    fait a lui seul 24535 points -- 75 % du budget d'un chemin -- et il enveloppe toute la feuille.
    Aucune coupe ne peut le laisser du meme cote que tout ce qu'il contient.

    ## Ce qui est livre a la place

    Un **masque d'encre** en PNG, rendu une fois depuis le trace nettoye. Il ne perd rien de ce qui
    comptait : la planche est monochrome, le PNG porte son alpha, et l'ecran le teinte au jeton
    d'encre exactement comme il aurait teinte le SVG. Il est rendu a trois fois la taille de la
    maquette, soit largement au-dela de ce qu'un ecran en montre.
    """


def number(value: float) -> str:
    """Nombre court : la planche en porte des centaines de milliers."""
    text = ("%.4f" % value).rstrip("0").rstrip(".")
    return text if text not in ("", "-0") else "0"


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--check", action="store_true",
                        help="verifie que la planche livree est celle que la table produit")
    arguments = parser.parse_args()

    if not SOURCE.exists():
        print("planche : corpus absent (%s).\n"
              "Le corpus n'est pas versionne (EX-CNT-023) : deposer le SVG a la main pour "
              "regenerer la planche. La planche LIVREE, elle, est versionnee et son empreinte "
              "est verifiee par scripts/check_ui_assets.py." % SOURCE, file=sys.stderr)
        return 0 if arguments.check else 1

    table = json.loads(TABLE.read_text(encoding="utf-8"))
    # Un champ porte une LISTE de rectangles : un intitule grave sur deux lignes (<< SAVING /
    # THROWS >>) se retire par deux rectangles serres, la ou un seul, assez grand pour couvrir les
    # deux, mordrait sur le bandeau entre elles.
    # Un champ porte, avec ses rectangles, la taille maximale de ce qui peut en etre retire.
    rectangles = [(r, tuple(field.get("maxGlyph", DEFAULT_GLYPH)))
                  for field in table["fields"] for r in field.get("erase", [])]

    svg = SOURCE.read_text(encoding="utf-8")
    data = read_ink_path(svg)

    pieces = list(subpaths(data))

    # --- Ce qui est trop grand pour etre du lettrage ne part pas ---
    #
    # Le trace est peint en `evenodd` : une plaque de nom, un cartouche, un bandeau sont un contour
    # EXTERIEUR et un contour INTERIEUR dont la superposition creuse le champ clair. Retirer le
    # contour INTERIEUR ne retire donc rien -- cela BOUCHE la forme, qui devient un pave noir.
    #
    # Or ce contour interieur a, a peu de chose pres, la LARGEUR de l'intitule qu'il encadre : la
    # largeur ne les separe donc pas. Leur HAUTEUR, si. Un intitule grave fait 7 a 8 unites de
    # haut (16 pour les capitales des bandeaux) ; l'evidement qui le porte en fait 10 a 24, parce
    # qu'il doit laisser une marge autour des lettres. Chaque champ porte donc sa borne, reglee
    # au-dessus de son lettrage et au-dessous de son evidement.
    #
    # Une borne par champ, et non une borne globale : sur la roue, l'evidement d'une plaque fait
    # 14 unites quand le bandeau des competences en fait 24, et une valeur unique laisserait
    # forcement l'un des deux du mauvais cote. Ce qui est epargne est COMPTE, pour qu'un intitule
    # reellement plus grand que sa borne ne quitte pas le perimetre en silence.
    def doom(box, rect, cap) -> bool:
        x0, y0, x1, y1 = rect
        return (box[0] >= x0 and box[2] <= x1 and box[1] >= y0 and box[3] <= y1
                and box[2] - box[0] <= cap[0] and box[3] - box[1] <= cap[1])

    doomed = []
    spared = 0
    for piece in pieces:
        box = piece["box"]
        if box[0] is None:
            doomed.append(False)
            continue
        hit = any(doom(box, rect, cap) for rect, cap in rectangles)
        if not hit and any(box[0] >= r[0] and box[2] >= r[0] and box[2] <= r[2]
                           and box[1] >= r[1] and box[3] <= r[3] for r, _ in rectangles):
            spared += 1
        doomed.append(hit)

    kept: list[str] = []
    removed = 0
    for index, piece in enumerate(pieces):
        if doomed[index]:
            removed += 1
            continue
        body = data[piece["begin"]:piece["end"]]
        # Reouverture en absolu : le sous-chemin precedent a pu disparaitre.
        head = "M%s,%s" % (number(piece["origin"][0]), number(piece["origin"][1]))
        tail = piece["tail"] or []
        if tail:
            head += "l" + ",".join(number(v) for v in tail)
        rest = COMMAND.search(body, 1)
        texte = head + (body[rest.start():] if rest else "")
        # Cout en ELEMENTS de `QPainterPath` : un `moveTo` et chaque `lineTo` en valent un, une
        # cubique en vaut trois. C'est l'unite que Qt plafonne -- pas le nombre de caracteres.
        cout = 1 + len(tail) // 2
        for lettre, nombres, _, _ in commands(texte):
            if lettre in "Cc":
                cout += 3 * (len(nombres) // 6)
            elif lettre in "Ll":
                cout += len(nombres) // 2
            elif lettre in "HhVv":
                cout += len(nombres)
        kept.append({"d": texte, "box": piece["box"], "cost": cout})

    total = removed + len(kept)
    print("planche : %d sous-chemins, %d retires (lettrage), %d conserves (ornement), "
          "%d epargnes (evidements)" % (total, removed, len(kept), spared))
    if removed == 0:
        print("planche : AUCUN sous-chemin retire -- la table ne recouvre plus le trace",
              file=sys.stderr)
        return 1

    plate = (
        '<?xml version="1.0" encoding="UTF-8"?>\n'
        "<!-- Planche 1 de la feuille de personnage de Tanares, SANS son lettrage.\n"
        "     Genere par scripts/build_character_sheet_plate.py depuis le corpus\n"
        "     (Character_Sheets_Tanares.svg) et la table character-sheet-plate.json.\n"
        "     Ne pas retoucher a la main : le trait et les rectangles de la table doivent\n"
        "     rester d'accord, et c'est le script qui le garantit. -->\n"
        '<svg xmlns="http://www.w3.org/2000/svg" version="1.1" '
        'width="%d" height="%d" viewBox="0 0 %d %d">\n'
        "%s"
        "</svg>\n"
    ) % (table["page"][0], table["page"][1], table["page"][0], table["page"][1],
         '<path fill="#000000" fill-rule="evenodd" d="%s"/>\n' % "".join(
             forme["d"] for forme in kept))

    # Le SVG nettoye n'est qu'une ETAPE : Qt ne sait pas le rendre (cf. `_decoupe_impossible`).
    # Il est ecrit a cote de la planche, rasterise, puis efface.
    intermediaire = TARGET.with_suffix(".svg")
    intermediaire.write_text(plate, encoding="utf-8")

    inkscape = trouver_inkscape()
    if inkscape is None:
        intermediaire.unlink(missing_ok=True)
        print("planche : Inkscape introuvable -- c'est lui qui rasterise le trace nettoye en "
              "masque d'encre. Installer Inkscape, puis relancer.", file=sys.stderr)
        return 1

    largeur = table["page"][0] * PLATE_SCALE
    rendu = subprocess.run(
        [str(inkscape), str(intermediaire), "--export-type=png",
         "--export-filename=%s" % TARGET, "--export-width=%d" % largeur],
        capture_output=True, text=True, check=False)
    intermediaire.unlink(missing_ok=True)
    if rendu.returncode != 0 or not TARGET.exists():
        print("planche : Inkscape a echoue :\n%s" % (rendu.stderr or rendu.stdout),
              file=sys.stderr)
        return 1
    print("planche : %s ecrite, %d px de large (%.1f Mo)"
          % (TARGET.name, largeur, TARGET.stat().st_size / 1e6))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
