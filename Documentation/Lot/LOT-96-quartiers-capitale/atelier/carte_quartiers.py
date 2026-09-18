#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# SPDX-FileCopyrightText: 2026 Valentin Eloy
# SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
"""Trace les cartes des quartiers de la Capitale (`LOT-96`).

Même méthode que le Colisée du `LOT-09` : la carte se **pose par script**, puis se retouche dans
l'éditeur (`LOT-11`). Le fichier produit est un fichier de niveau ordinaire, sans marque d'origine.

## Ce que le script relève sur le plan, et ce qu'il décide

Le plan de la ville est celui que l'auteur a peint (`city-central-empire-the-capital-city.jpg`,
`LOT-94`) ; les douze quartiers y sont placés en fractions (`world-maps.json`). Le script en tire
une seule chose : **la direction** de chaque quartier voisin, vue du quartier qu'on trace. Une
porte vers un quartier se pose sur le bord de la carte que coupe cette direction ; c'est ce qui
fait qu'on sort de Martpart vers le nord-ouest pour aller à Arenarea, comme sur le plan.

Il décide le reste, qui n'est écrit nulle part : les rues, la place, les îlots de maisons, les
ruelles, les étals. Un quartier du livre n'a pas de plan rue par rue.

## Les trois grilles, comme au Colisée

- la grille **racine** est la collision : `wall` où l'on ne passe pas ; une case franchissable qui
  porte une pièce reçoit `dirt` pour que son assignation de texture soit émise ;
- la couche **sol** porte la fente de matière de chaque case (`dirt` la rue, `solid` la place,
  `bridge` la ruelle, `stairs` le pas de porte), que `Assets/Scene/<lieu>/appearance.json` traduit ;
- la couche **décor** porte `wall` sur chaque case de relief, dont l'assignation nomme la pièce.

Usage :

    py -3.13 Documentation/Lot/LOT-96-quartiers-capitale/atelier/carte_quartiers.py [--check]
"""
from __future__ import annotations

import argparse
import json
import math
import sys
from dataclasses import dataclass, field
from pathlib import Path

RACINE = Path(__file__).resolve().parents[4]
NIVEAUX = RACINE / "Source" / "Elements" / "Levels" / "capital"
PLAN = RACINE / "Source" / "Elements" / "Maps" / "world-maps.json"

# La console Windows est en cp1252 : les messages portent des accents.
if hasattr(sys.stdout, "reconfigure"):
    sys.stdout.reconfigure(encoding="utf-8", errors="replace")

VILLE = "central-empire-the-capital-city"
# Le plan fait 1920 x 1080 : les fractions se ramènent en pixels pour que les directions ne
# soient pas écrasées par le format.
PLAN_LARGEUR, PLAN_HAUTEUR = 1920, 1080

# Écart minimal entre deux portes d'un même bord, en cases : trois de rue et deux de maison.
ECART_PORTES = 6
# Profondeur de la rue d'une porte avant qu'elle ne tourne vers le cœur du quartier.
PROFONDEUR_PORTE = 4


def rect(x1: int, y1: int, x2: int, y2: int):
    """Les cases d'un rectangle, bornes incluses."""
    for y in range(y1, y2 + 1):
        for x in range(x1, x2 + 1):
            yield x, y


def variante(x: int, y: int, n: int) -> int:
    """Un choix stable par case : la même case donne toujours la même pièce."""
    return (x * 7 + y * 13 + (x * y) % 5) % n


@dataclass
class Porte:
    """Une porte du quartier, sur un bord de la carte."""

    vers: str  # identifiant du quartier voisin (fiche d'atlas)
    nom: str  # nom court, qui nomme aussi le point d'arrivée
    x: int = 0
    y: int = 0
    bord: str = ""  # "N", "S", "O", "E"

    def interieur(self, pas: int) -> tuple[int, int]:
        """La case à @p pas cases de la porte, vers l'intérieur de la carte."""
        dx, dy = {"N": (0, 1), "S": (0, -1), "O": (1, 0), "E": (-1, 0)}[self.bord]
        return self.x + dx * pas, self.y + dy * pas


@dataclass
class Quartier:
    ident: str  # identifiant de carte : capital/<ident>
    fiche: str  # identifiant de la fiche d'atlas
    nom: str
    lieu: str  # le dossier de planches : Assets/Scene/<lieu>/
    largeur: int
    hauteur: int
    place: tuple[int, int, int, int]  # la place, cœur du quartier
    batiments: list[tuple[int, int, int, int]]  # grands bâtiments fermés (arènes, temples)
    portes: list[Porte]
    depart: Porte | None = None  # la porte de la ville où « Nouvelle partie » pose le héros
    # Rues secondaires, en L, d'un point à un autre : ce que les portes ne desservent pas.
    rues: list[tuple[tuple[int, int], tuple[int, int]]] = field(default_factory=list)
    sol: dict = field(default_factory=dict)
    relief: dict = field(default_factory=dict)
    obstacles: set = field(default_factory=set)


# --------------------------------------------------------------------------------------------------
# Le plan : directions et bords
# --------------------------------------------------------------------------------------------------


def points_du_plan() -> dict[str, tuple[float, float]]:
    plan = json.loads(PLAN.read_text(encoding="utf-8"))
    places = plan["cities"][VILLE]["places"]
    return {cle[len(VILLE) + 1:]: (x * PLAN_LARGEUR, y * PLAN_HAUTEUR) for cle, (x, y) in places.items()}


def poser_portes(q: Quartier, points: dict[str, tuple[float, float]]) -> None:
    """Pose chaque porte sur le bord que coupe la direction du quartier voisin, vue du plan."""
    ox, oy = points[q.ident]
    cx, cy = (q.largeur - 1) / 2, (q.hauteur - 1) / 2
    for porte in q.portes:
        vx, vy = points[porte.vers[len(VILLE) + 1:]]
        dx, dy = vx - ox, vy - oy
        # Le rayon depuis le centre de la carte ; le premier bord qu'il touche porte la porte.
        tx = (cx / abs(dx)) if dx else math.inf
        ty = (cy / abs(dy)) if dy else math.inf
        if tx <= ty:
            porte.bord = "E" if dx > 0 else "O"
            porte.x = q.largeur - 1 if dx > 0 else 0
            porte.y = round(cy + dy * tx)
        else:
            porte.bord = "S" if dy > 0 else "N"
            porte.y = q.hauteur - 1 if dy > 0 else 0
            porte.x = round(cx + dx * ty)
    # Deux portes d'un même bord s'écartent, dans l'ordre où le plan les met : on ne passe pas
    # deux rues dans la même.
    for bord in "NSOE":
        sur_le_bord = [p for p in q.portes + ([q.depart] if q.depart else []) if p.bord == bord]
        horizontal = bord in "NS"
        sur_le_bord.sort(key=lambda p: p.x if horizontal else p.y)
        limite = (q.largeur if horizontal else q.hauteur) - 3
        for i, porte in enumerate(sur_le_bord):
            position = porte.x if horizontal else porte.y
            position = max(position, 3 if i == 0 else sur_le_bord[i - 1].x + ECART_PORTES
                           if horizontal else sur_le_bord[i - 1].y + ECART_PORTES)
            position = min(position, limite - ECART_PORTES * (len(sur_le_bord) - 1 - i))
            if horizontal:
                porte.x = position
            else:
                porte.y = position


# --------------------------------------------------------------------------------------------------
# Le tracé
# --------------------------------------------------------------------------------------------------


def peindre(q: Quartier, cases, matiere: str) -> None:
    for x, y in cases:
        if 0 <= x < q.largeur and 0 <= y < q.hauteur:
            q.sol[(x, y)] = matiere


def rue(q: Quartier, a: tuple[int, int], b: tuple[int, int]) -> None:
    """Une rue de trois cases de large, en L : d'abord le long de x, puis le long de y."""
    (x1, y1), (x2, y2) = a, b
    peindre(q, rect(min(x1, x2) - 1, y1 - 1, max(x1, x2) + 1, y1 + 1), "dirt")
    peindre(q, rect(x2 - 1, min(y1, y2) - 1, x2 + 1, max(y1, y2) + 1), "dirt")


def rue_depuis_porte(q: Quartier, porte: Porte, anneau: tuple[int, int, int, int]) -> None:
    """La rue d'une porte : droit vers l'intérieur, puis vers l'anneau qui ceint la place."""
    coude = porte.interieur(PROFONDEUR_PORTE)
    x1, y1, x2, y2 = anneau
    # Le point de l'anneau le plus proche du coude.
    cible = (min(max(coude[0], x1 + 1), x2 - 1), min(max(coude[1], y1 + 1), y2 - 1))
    if porte.bord in "NS":
        rue(q, (porte.x, porte.y), (porte.x, coude[1]))
        # Le long de la ligne du coude jusqu'à la colonne de l'anneau, puis le long de la colonne.
        rue(q, (porte.x, coude[1]), (cible[0], cible[1]))
    else:
        rue(q, (porte.x, porte.y), (coude[0], porte.y))
        rue(q, (coude[0], porte.y), (cible[0], cible[1]))


def tracer_voirie(q: Quartier) -> tuple[int, int, int, int]:
    px1, py1, px2, py2 = q.place
    anneau = (px1 - 3, py1 - 3, px2 + 3, py2 + 3)
    ax1, ay1, ax2, ay2 = anneau
    # L'anneau : trois cases de rue autour de la place.
    for x, y in rect(*anneau):
        if not (px1 <= x <= px2 and py1 <= y <= py2):
            q.sol[(x, y)] = "dirt"
    peindre(q, rect(*q.place), "solid")
    for porte in q.portes + ([q.depart] if q.depart else []):
        rue_depuis_porte(q, porte, (ax1 + 1, ay1 + 1, ax2 - 1, ay2 - 1))
    for a, b in q.rues:
        rue(q, a, b)
    return anneau


def percer_ruelles(q: Quartier) -> None:
    """Les ruelles : une grille de passages d'une case à travers les îlots de maisons.

    Elles ne traversent ni les grands bâtiments ni le bord de la carte ; un tronçon qui ne
    rejoint aucune rue est rebouché ensuite (`reboucher`), plutôt que de laisser une cour murée.
    """
    # Une ruelle court entre deux croisements ; un tronçon sur trois manque, ce qui fait des
    # impasses et des îlots de tailles inégales plutôt qu'un damier.
    for x, y in rect(3, 3, q.largeur - 4, q.hauteur - 4):
        if (x, y) in q.sol:
            continue
        if any(bx1 - 1 <= x <= bx2 + 1 and by1 - 1 <= y <= by2 + 1
               for bx1, by1, bx2, by2 in q.batiments):
            continue
        verticale = x % 6 == 3 and variante(x // 6, y // 5, 3) != 0
        horizontale = y % 5 == 2 and variante(x // 6 + 1, y // 5 + 2, 3) != 0
        if verticale or horizontale:
            q.sol[(x, y)] = "bridge"


def reboucher(q: Quartier, depart: tuple[int, int]) -> None:
    """Ce qu'on n'atteint pas depuis la porte redevient maison : aucune cour inatteignable."""
    vus = {depart}
    pile = [depart]
    while pile:
        x, y = pile.pop()
        for voisin in ((x + 1, y), (x - 1, y), (x, y + 1), (x, y - 1)):
            if voisin in q.sol and voisin not in vus:
                vus.add(voisin)
                pile.append(voisin)
    for case in list(q.sol):
        if case not in vus:
            del q.sol[case]


def habiller(q: Quartier) -> None:
    """Les pièces : façades des maisons, grands bâtiments, étals de la place, lanternes."""

    def franchissable(x: int, y: int) -> bool:
        return (x, y) in q.sol and (x, y) not in q.obstacles

    # La place : des rangées d'étals, avec des allées entre elles ; une bannière au milieu.
    px1, py1, px2, py2 = q.place
    for y in range(py1 + 2, py2 - 1, 4):
        for x in range(px1 + 2, px2 - 1, 3):
            piece = ("prop-1", "prop-2", "prop-4", "prop-1")[variante(x, y, 4)]
            q.relief[(x, y)] = piece
            q.obstacles.add((x, y))
    centre = ((px1 + px2) // 2, (py1 + py2) // 2)
    q.relief[centre] = "prop-3"
    q.obstacles.add(centre)
    for coin in ((px1, py1), (px2, py1), (px1, py2), (px2, py2)):
        q.relief[coin] = "light"
        q.obstacles.add(coin)

    # Les grands bâtiments : un bloc de murs, une arche sur la face qui regarde la place.
    for bx1, by1, bx2, by2 in q.batiments:
        for x, y in rect(bx1, by1, bx2, by2):
            q.sol.pop((x, y), None)

    cours: list[tuple[int, int]] = []
    for y in range(q.hauteur):
        for x in range(q.largeur):
            if (x, y) in q.sol:
                continue
            nord, sud = franchissable(x, y - 1), franchissable(x, y + 1)
            ouest, est = franchissable(x - 1, y), franchissable(x + 1, y)
            if (nord or sud) and (ouest or est):
                q.relief[(x, y)] = "wall-corner"
            elif nord or sud:
                q.relief[(x, y)] = ("window-right", "wall-right", "door-right", "window-right",
                                    "wall-right")[variante(x, y, 5)]
            elif ouest or est:
                q.relief[(x, y)] = ("window-left", "wall-left", "door-left", "window-left",
                                    "wall-left")[variante(x, y, 5)]
            else:
                # Le cœur d'un îlot : ni mur (des murs pleins y dessinaient un treillis de
                # clôtures) ni pavé (voir plus bas).
                cours.append((x, y))

    # Les cours ne reçoivent rien : ni sol ni pièce. La planche n'a pas de toit, et un pavé de
    # place les faisait lire comme des places ouvertes derrière des murets.
    del cours

    # Un pas de porte devant chaque porte de maison.
    for (x, y), piece in list(q.relief.items()):
        if piece.startswith("door-"):
            for voisin in ((x, y + 1), (x, y - 1), (x + 1, y), (x - 1, y)):
                if q.sol.get(voisin) == "dirt" and voisin not in q.obstacles:
                    q.sol[voisin] = "stairs"
                    break

    # Les grands bâtiments : leurs faces portent des arches et des bannières, pas des fenêtres.
    for bx1, by1, bx2, by2 in q.batiments:
        for x, y in rect(bx1, by1, bx2, by2):
            if (x, y) in q.relief and q.relief[(x, y)].startswith(("window-", "door-")):
                q.relief[(x, y)] = "prop-3" if (x + y) % 4 == 0 else q.relief[(x, y)].replace(
                    "window", "wall").replace("door", "wall")

    # Lanternes le long des rues, du côté des maisons : une toutes les sept cases.
    for (x, y), matiere in sorted(q.sol.items()):
        if matiere != "dirt" or (x, y) in q.obstacles or (x * 3 + y) % 7:
            continue
        if sum(1 for v in ((x + 1, y), (x - 1, y), (x, y + 1), (x, y - 1)) if v in q.sol) == 3:
            q.relief[(x, y)] = "light"
            q.obstacles.add((x, y))


def tracer(q: Quartier, points: dict[str, tuple[float, float]]) -> dict:
    poser_portes(q, points)
    tracer_voirie(q)
    percer_ruelles(q)
    depart = q.depart or q.portes[0]
    reboucher(q, (depart.x, depart.y))
    habiller(q)

    entree = depart.interieur(1)
    sortie = depart.interieur(2)
    entites: list[dict] = []
    for porte in q.portes:
        entites.append({"type": "spawnPoint", "x": porte.interieur(2)[0], "y": porte.interieur(2)[1],
                        "name": porte.nom})
    if q.depart:
        entites.append({"type": "spawnPoint", "x": entree[0], "y": entree[1], "name": q.depart.nom})

    racine: list[dict] = []
    for y in range(q.hauteur):
        for x in range(q.largeur):
            case = (x, y)
            piece = q.relief.get(case)
            if case in q.sol and case not in q.obstacles:
                if case == entree:
                    type_ = "entry"
                elif case == sortie:
                    type_ = "exit"
                elif piece is not None:
                    type_ = "dirt"
                else:
                    continue
            else:
                # Toute case où l'on ne marche pas est un mur de la grille racine, cœurs d'îlots
                # compris : une case absente y vaudrait « vide », c'est-à-dire franchissable.
                type_ = "wall"
            tuile = {"x": x, "y": y, "type": type_}
            if piece is not None:
                tuile["texture"] = piece
            racine.append(tuile)

    return {
        "version": 3,
        "name": q.nom,
        "width": q.largeur,
        "height": q.hauteur,
        "tiles": racine,
        "layers": [
            {
                "name": "sol",
                "kind": "ground",
                "scene": q.lieu,
                "note": "La matière de chaque case ; la table du lieu (Assets/Scene/%s/appearance.json) la traduit en pièce de la planche du LOT-92." % q.lieu,
                "tiles": [{"x": x, "y": y, "type": m}
                          for (x, y), m in sorted(q.sol.items(), key=lambda i: (i[0][1], i[0][0]))],
            },
            {
                "name": "relief",
                "kind": "decor",
                "note": "Une case de relief par pièce dressée ; l'assignation de texture de la grille racine nomme la pièce exacte.",
                "tiles": [{"x": x, "y": y, "type": "wall"}
                          for (x, y) in sorted(q.relief, key=lambda c: (c[1], c[0]))],
            },
        ],
        "entities": entites,
        "cameraFraming": {"mode": "follow"},
    }


# --------------------------------------------------------------------------------------------------
# Les quartiers
# --------------------------------------------------------------------------------------------------


def fiche(quartier: str) -> str:
    return "%s-%s" % (VILLE, quartier)


def quartiers() -> list[Quartier]:
    return [
        Quartier(
            ident="martpart",
            fiche=fiche("martpart"),
            nom="Martpart",
            lieu="martpart",
            largeur=48,
            hauteur=40,
            # La place du marché, au cœur du quartier ; l'arène Illu Die à l'est, fermée : on
            # n'y entre qu'au LOT-27.
            place=(16, 15, 27, 24),
            batiments=[(33, 11, 42, 21)],
            portes=[
                Porte(vers=fiche("arenarea"), nom="arenarea"),
            ],
            depart=Porte(vers="", nom="porte-est", bord="E", x=47, y=30),
            # La rue des Lanternes : elle fait le tour de l'arène par le nord et rejoint la rue
            # de la porte de l'Est.
            rues=[((29, 8), (45, 8)), ((45, 8), (45, 29))],
        ),
    ]


def main() -> int:
    analyseur = argparse.ArgumentParser(description="Trace les cartes des quartiers (LOT-96).")
    analyseur.add_argument("--check", action="store_true",
                           help="ne rien écrire ; sortir en erreur si une carte commitée diffère")
    arguments = analyseur.parse_args()

    points = points_du_plan()
    code = 0
    for q in quartiers():
        texte = json.dumps(tracer(q, points), ensure_ascii=False, indent=2) + "\n"
        chemin = NIVEAUX / ("%s.json" % q.ident)
        if arguments.check:
            if not chemin.exists() or chemin.read_text(encoding="utf-8") != texte:
                print("carte_quartiers --check : %s diffère de ce que le script produit."
                      % chemin.relative_to(RACINE).as_posix())
                code = 1
            continue
        chemin.parent.mkdir(parents=True, exist_ok=True)
        chemin.write_text(texte, encoding="utf-8", newline="\n")
        marche = sum(1 for c in q.sol if c not in q.obstacles)
        print("carte_quartiers : %s écrite — %d x %d cases, %d franchissables, portes %s."
              % (chemin.relative_to(RACINE).as_posix(), q.largeur, q.hauteur, marche,
                 ", ".join("%s %s(%d,%d)" % (p.nom, p.bord, p.x, p.y) for p in q.portes)))
    if arguments.check and code == 0:
        print("carte_quartiers --check : les cartes commitées sont exactement celles du script.")
    return code


if __name__ == "__main__":
    sys.exit(main())
