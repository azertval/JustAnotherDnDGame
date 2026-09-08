# SPDX-FileCopyrightText: 2026 Valentin Eloy
# SPDX-License-Identifier: GPL-3.0-or-later
"""Garde-fou : les cles d'assets d'entite, et l'etat d'avancement de leur illustration.

Ce controle fait DEUX choses de nature differente, et c'est deliberé :

- il **echoue** sur une cle orpheline -- une derogation malformee, ou qui nomme une famille
  inconnue. C'est une faute de donnee : la creature n'aurait aucune illustration, et rien ne le
  dirait (`EX-CNT-040`).
- il **liste**, sans echouer, les cles encore servies par un marqueur genere. C'est un etat
  d'avancement, pas un defaut (`EX-CNT-041`) : la production graphique est un remplacement
  progressif, jamais un prealable bloquant. Un lint qui echouerait ici rendrait le depot rouge
  jusqu'a la derniere illustration livree, et personne ne regarderait plus sa sortie.

Le manifeste n'est pas un fichier : il est **derive** des catalogues a chaque appel. Un manifeste
commite divergerait du catalogue au premier ajout de creature, et c'est la copie oubliee qu'on lit
six mois plus tard.

Aucune dependance : la table des familles et les catalogues sont du JSON, et la syntaxe d'une cle
tient en une expression reguliere -- la meme que `common.schema.json` et que `core::isValidAssetKey`.

Usage :
    python scripts/check_asset_keys.py            # code de sortie non nul si cle orpheline
    python scripts/check_asset_keys.py --quiet    # sans la liste des cles sans art
"""

from __future__ import annotations

import argparse
import json
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
RPG = ROOT / "Source" / "Elements" / "Rpg"
ENTITIES = ROOT / "Source" / "Elements" / "Assets" / "Entities"
FAMILIES = ENTITIES / "families.json"

# La MEME expression que `common.schema.json` ($defs/assetKey) et que `core::isValidAssetKey` : une
# donnee refusee a la validation doit etre exactement celle que le moteur refuse.
CLE = re.compile(r"^[a-z0-9]+(-[a-z0-9]+)*(/[a-z0-9]+(-[a-z0-9]+)*)+$")

# Extensions admises pour une illustration livree.
IMAGES = (".png", ".jpg", ".jpeg")

erreurs: list[str] = []


def fail(message: str) -> None:
    erreurs.append(message)


def charger_familles() -> list[dict]:
    if not FAMILIES.is_file():
        fail("%s absent : rien ne dit quelles familles existent." % FAMILIES.relative_to(ROOT))
        return []
    try:
        contenu = json.loads(FAMILIES.read_text(encoding="utf-8"))
    except json.JSONDecodeError as erreur:
        fail("%s illisible : %s" % (FAMILIES.relative_to(ROOT), erreur))
        return []
    familles = contenu.get("families", [])
    for famille in familles:
        nom = famille.get("name", "")
        if not CLE.match(nom + "/essai"):
            fail("famille au nom invalide : '%s'" % nom)
        if famille.get("width", 0) <= 0 or famille.get("height", 0) <= 0:
            fail("famille '%s' sans dimensions attendues" % nom)
        if not famille.get("catalogues"):
            fail("famille '%s' sans dossier de catalogue" % nom)
    return familles


def cles_attendues(familles: list[dict]) -> list[dict]:
    """Le manifeste, derive des catalogues. Une cle par entree, sauf derogation ecrite."""
    noms = {famille["name"] for famille in familles}
    attendues: list[dict] = []
    for famille in familles:
        for dossier in famille.get("catalogues", []):
            chemin = RPG / dossier
            if not chemin.is_dir():
                fail("%s : dossier de catalogue absent (famille '%s')"
                     % (chemin.relative_to(ROOT), famille["name"]))
                continue
            for fichier in sorted(chemin.glob("*.json")):
                try:
                    entree = json.loads(fichier.read_text(encoding="utf-8"))
                except json.JSONDecodeError as erreur:
                    fail("%s illisible : %s" % (fichier.name, erreur))
                    continue
                identifiant = entree.get("id", "")
                if not identifiant:
                    fail("%s : entree sans identifiant" % fichier.name)
                    continue

                ecrite = entree.get("asset", "")
                if ecrite:
                    # Une DEROGATION : deux entrees peuvent partager une illustration. Elle doit
                    # etre bien formee et nommer une famille connue -- c'est la seule chose qui
                    # fasse echouer ce controle.
                    if not CLE.match(ecrite):
                        fail("%s : cle d'asset malformee ('%s')" % (fichier.name, ecrite))
                        continue
                    if ecrite.split("/", 1)[0] not in noms:
                        fail("%s : cle d'asset ORPHELINE, famille inconnue ('%s')"
                             % (fichier.name, ecrite.split("/", 1)[0]))
                        continue
                    attendues.append({"cle": ecrite, "famille": ecrite.split("/", 1)[0],
                                      "fichier": fichier.name, "derogation": True})
                    continue

                derivee = "%s/%s" % (famille["name"], identifiant)
                if not CLE.match(derivee):
                    fail("%s : l'identifiant '%s' ne peut pas former une cle d'asset"
                         % (fichier.name, identifiant))
                    continue
                attendues.append({"cle": derivee, "famille": famille["name"],
                                  "fichier": fichier.name, "derogation": False})
    return attendues


def illustration_livree(cle: str) -> bool:
    """Vrai si une image porte cette cle sous `Assets/Entities/<famille>/<identifiant>.<ext>`."""
    famille, identifiant = cle.split("/", 1)
    return any((ENTITIES / famille / (identifiant + extension)).is_file() for extension in IMAGES)


def orphelines_du_dossier(attendues: list[dict]) -> None:
    """Une image livree qu'aucune donnee n'attend : elle ne sera jamais affichee."""
    connues = {entree["cle"] for entree in attendues}
    for fichier in sorted(ENTITIES.rglob("*")):
        if not fichier.is_file() or fichier.suffix.lower() not in IMAGES:
            continue
        relatif = fichier.relative_to(ENTITIES)
        if len(relatif.parts) != 2:
            fail("%s : une illustration vit sous <famille>/<identifiant>.<ext>" % relatif)
            continue
        cle = "%s/%s" % (relatif.parts[0], fichier.stem)
        if cle not in connues:
            fail("%s : illustration ORPHELINE, aucune donnee n'attend la cle '%s'"
                 % (relatif, cle))


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--quiet", action="store_true",
                        help="ne pas lister les cles encore servies par un marqueur")
    arguments = parser.parse_args()

    familles = charger_familles()
    attendues = cles_attendues(familles) if familles else []

    doublons = {}
    for entree in attendues:
        if entree["derogation"]:
            continue
        doublons.setdefault(entree["cle"], []).append(entree["fichier"])
    for cle, fichiers in sorted(doublons.items()):
        if len(fichiers) > 1:
            fail("cle derivee en double : '%s' (%s)" % (cle, ", ".join(fichiers)))

    if attendues:
        orphelines_du_dossier(attendues)

    sans_art = [entree for entree in attendues if not illustration_livree(entree["cle"])]

    if erreurs:
        print("check_asset_keys : %d violation(s)\n" % len(erreurs), file=sys.stderr)
        for erreur in erreurs:
            print("  - %s" % erreur, file=sys.stderr)
        return 1

    # L'etat d'avancement, et non un defaut (EX-CNT-041).
    par_famille: dict[str, int] = {}
    for entree in sans_art:
        par_famille[entree["famille"]] = par_famille.get(entree["famille"], 0) + 1

    print("check_asset_keys : OK (%d cle(s) attendue(s), %d famille(s), aucune orpheline)."
          % (len(attendues), len(familles)))
    if sans_art:
        print("  %d cle(s) encore servie(s) par un marqueur genere -- etat d'avancement, "
              "pas un defaut (EX-CNT-041) :" % len(sans_art))
        for famille in sorted(par_famille):
            print("    %-12s %d" % (famille, par_famille[famille]))
        if not arguments.quiet:
            for entree in sans_art[:10]:
                print("      %s" % entree["cle"])
            if len(sans_art) > 10:
                print("      ... et %d autres" % (len(sans_art) - 10))
    else:
        print("  toutes les cles ont une illustration definitive.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
