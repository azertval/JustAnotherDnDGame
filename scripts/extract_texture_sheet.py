#!/usr/bin/env python3
# SPDX-FileCopyrightText: 2026 Valentin Eloy
# SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
"""Atelier des textures (LOT-92, T3) : une planche de scène par lieu, déclarée par sa disposition.

`extract_coliseum_atlas.py` (LOT-50) découpait UNE planche à des coordonnées écrites à la main,
relevées sur l'image après coup. Ici la disposition vient AVANT l'image : un JSON
(`atelier/dispositions/<id>.json`) déclare chaque cellule -- son nom, sa classe (sol, pièce haute,
grande pièce), son emprise en cases et la hauteur qu'elle prend au-dessus du sol -- et tout le reste
s'en déduit : la grille de la planche, le gabarit envoyé au générateur, le bloc C du prompt, la
découpe et la clé de chaque texture. Aucune coordonnée n'est écrite à la main.

La géométrie est celle d'`IsoProjection` : un losange de 68 × 42 pixels d'art (rapport 0,62), dessiné
à 2 pixels d'écran par pixel d'art. Une emprise de a × b cases est un parallélogramme de
(a + b) · 34 × (a + b) · 21 pixels d'art ; son sommet haut est le coin (0, 0) de la grille, un pas
en x descend de (34, 21), un pas en y de (−34, 21). Une pièce haute ajoute sa hauteur au-dessus.

Commandes :
    py -3.13 scripts/extract_texture_sheet.py commande <id> <tour>
        prépare l'envoi à la main : prompt (blocs A, B, C), maquette et gabarit, sous
        <TEXTURE_ATELIER>/chatgpt/<id>-tour<K>/ ;
    py -3.13 scripts/extract_texture_sheet.py decoupe <id> <candidat.png>
        met la planche reçue au format, la découpe et l'installe sous `installRoot` avec son manifeste ;
    py -3.13 scripts/extract_texture_sheet.py --check <id>
        refait la découpe en mémoire depuis la planche installée et compare, SANS rien écrire ;
    py -3.13 scripts/extract_texture_sheet.py valider
        contrôle toutes les dispositions (sans Pillow : c'est ce que `check_assets_brief.py` appelle).

Dépendances : aucune pour `valider` ; Pillow et numpy pour le reste (outil de production).
"""

from __future__ import annotations

import hashlib
import io
import json
import os
import re
import shutil
import sys
from pathlib import Path

RACINE = Path(__file__).resolve().parent.parent
ATELIER = RACINE / "Documentation" / "Lot" / "LOT-92-atelier-textures" / "atelier"
DISPOSITIONS = ATELIER / "dispositions"
STYLE = ATELIER / "prompts" / "style.txt"
MAQUETTE = ATELIER / "ancres" / "maquette.png"
LIEUX = RACINE / "Source" / "Elements" / "World" / "locations"
TRAVAIL = Path(os.environ.get("TEXTURE_ATELIER", r"D:/JustAnotherDnDGame-textures"))

PLANCHE = (1536, 1024)      # pixels d'écran
PAS = 2                     # pixels d'écran par pixel d'art
DEMI_L, DEMI_H = 34, 21     # demi-losange, pixels d'art (68 × 42, IsoProjection 0,62)
MARGE = 8                   # pixels d'écran autour de chaque cellule
COULEURS = 64               # palette commune de la planche
COUVERTURE_SOL = 0.97       # part minimale du losange qu'un sol doit remplir
ALPHA = 127                 # seuil de binarisation, comme au LOT-91

CLE_RE = re.compile(r"^[a-z0-9]+(-[a-z0-9]+)*(/[a-z0-9]+(-[a-z0-9]+)*)+$")   # LOT-39
NOM_RE = re.compile(r"^[a-z0-9]+(-[a-z0-9]+)*$")


class DispositionError(ValueError):
    """Une disposition qui ne se lit pas comme ce module l'attend."""


# -- La disposition, sans image ------------------------------------------------------------------

def charger(identifiant: str) -> dict:
    chemin = DISPOSITIONS / f"{identifiant}.json"
    if not chemin.is_file():
        raise DispositionError(f"disposition « {identifiant} » introuvable ({chemin}).")
    return json.loads(chemin.read_text(encoding="utf-8"))


def emprise(disposition: dict, cellule: dict) -> tuple[int, int]:
    a, b = cellule.get("footprint", disposition["classes"][cellule["class"]]["footprint"])
    return int(a), int(b)


def canevas(disposition: dict, cellule: dict) -> tuple[int, int]:
    """Taille de la texture, en pixels d'art : l'emprise, et la hauteur au-dessus."""
    a, b = emprise(disposition, cellule)
    hausse = disposition["classes"][cellule["class"]]["rise"]
    return (a + b) * DEMI_L, (a + b) * DEMI_H + hausse


def sommets(disposition: dict, cellule: dict) -> list[tuple[int, int]]:
    """Haut, droite, bas, gauche de l'emprise, en pixels d'art dans le canevas."""
    a, b = emprise(disposition, cellule)
    hausse = disposition["classes"][cellule["class"]]["rise"]
    haut = (b * DEMI_L, hausse)
    droite = (haut[0] + a * DEMI_L, hausse + a * DEMI_H)
    gauche = (haut[0] - b * DEMI_L, hausse + b * DEMI_H)
    bas = (haut[0] + (a - b) * DEMI_L, hausse + (a + b) * DEMI_H)
    return [haut, droite, bas, gauche]


def grille(disposition: dict) -> list[dict]:
    """Les cellules posées sur la planche, dans l'ordre de lecture (rangées remplies à gauche).

    Chaque entrée : la cellule, `boite` (x0, y0, x1, y1) marge comprise et `origine`, le coin
    haut-gauche du canevas, en pixels d'écran. Une rangée prend la hauteur de sa plus haute cellule ;
    le canevas est posé en bas de la sienne.
    """
    poses, x, y, hauteur_rangee = [], 0, 0, 0
    rangee: list[dict] = []

    def fermer():
        for pose in rangee:
            w, h = pose["taille"]
            x0 = pose["x"]
            pose["boite"] = (x0, y, x0 + w, y + hauteur_rangee)
            cw, ch = canevas(disposition, pose["cellule"])
            pose["origine"] = (x0 + MARGE, y + hauteur_rangee - MARGE - ch * PAS)
            poses.append(pose)

    for cellule in disposition["cells"]:
        cw, ch = canevas(disposition, cellule)
        w, h = cw * PAS + 2 * MARGE, ch * PAS + 2 * MARGE
        if x + w > PLANCHE[0]:
            fermer()
            y += hauteur_rangee
            x, hauteur_rangee, rangee = 0, 0, []
        rangee.append({"cellule": cellule, "x": x, "taille": (w, h)})
        x += w
        hauteur_rangee = max(hauteur_rangee, h)
    fermer()
    if y + hauteur_rangee > PLANCHE[1]:
        raise DispositionError(f"{disposition['id']} : la grille fait {y + hauteur_rangee} px de haut, "
                               f"la planche {PLANCHE[1]}.")
    for pose in poses:
        del pose["x"], pose["taille"]
    return poses


def cle(disposition: dict, cellule: dict) -> str:
    return f"{disposition['keyPrefix']}/{cellule['name']}"


def valider(disposition: dict) -> list[str]:
    """Les fautes d'une disposition ; vide si elle est conforme."""
    fautes: list[str] = []
    ident = disposition.get("id", "?")
    if disposition.get("version") != 1:
        fautes.append(f"{ident} : version {disposition.get('version')!r}, 1 attendue.")
    for champ in ("id", "title", "location", "subject", "keyPrefix", "installRoot", "classes", "cells"):
        if champ not in disposition:
            fautes.append(f"{ident} : champ « {champ} » absent.")
    if fautes:
        return fautes
    if not CLE_RE.match(disposition["keyPrefix"] + "/x"):
        fautes.append(f"{ident} : préfixe de clé « {disposition['keyPrefix']} » invalide (LOT-39).")
    if not disposition["keyPrefix"].startswith("scene/"):
        fautes.append(f"{ident} : une texture de scène a une clé « scene/… ».")
    if not (LIEUX / f"{disposition['location']}.json").is_file():
        fautes.append(f"{ident} : lieu « {disposition['location']} » absent de l'atlas : le bloc B "
                      "se rédige depuis sa fiche.")
    for nom, classe in disposition["classes"].items():
        a, b = classe.get("footprint", (0, 0))
        if a < 1 or b < 1 or classe.get("rise", -1) < 0:
            fautes.append(f"{ident} : classe « {nom} » : emprise ou hauteur invalide.")
    vus: set[str] = set()
    for cellule in disposition["cells"]:
        nom = cellule.get("name", "")
        if not NOM_RE.match(nom):
            fautes.append(f"{ident} : nom de cellule « {nom} » invalide.")
        if nom in vus:
            fautes.append(f"{ident} : cellule « {nom} » déclarée deux fois.")
        vus.add(nom)
        if cellule.get("class") not in disposition["classes"]:
            fautes.append(f"{ident} : cellule « {nom} » : classe « {cellule.get('class')} » inconnue.")
        if not cellule.get("prompt", "").strip():
            fautes.append(f"{ident} : cellule « {nom} » sans description pour le générateur.")
        if "footprint" in cellule and min(cellule["footprint"]) < 1:
            fautes.append(f"{ident} : cellule « {nom} » : emprise invalide.")
    if not fautes:
        try:
            grille(disposition)
        except DispositionError as erreur:
            fautes.append(str(erreur))
    return fautes


def valider_tout() -> tuple[int, list[str]]:
    """Toutes les dispositions de l'atelier ; clés uniques entre elles."""
    fautes: list[str] = []
    cles: dict[str, str] = {}
    fichiers = sorted(DISPOSITIONS.glob("*.json"))
    for fichier in fichiers:
        disposition = json.loads(fichier.read_text(encoding="utf-8"))
        if disposition.get("id") != fichier.stem:
            fautes.append(f"{fichier.name} : id « {disposition.get('id')} », le nom du fichier attendu.")
        fautes += valider(disposition)
        for cellule in disposition.get("cells", []):
            if "keyPrefix" in disposition and "name" in cellule:
                k = cle(disposition, cellule)
                if k in cles:
                    fautes.append(f"clé « {k} » déclarée par {cles[k]} et {fichier.name}.")
                cles[k] = fichier.name
    return len(cles), fautes


def bloc_b(disposition: dict) -> str:
    fiche = json.loads((LIEUX / f"{disposition['location']}.json").read_text(encoding="utf-8"))
    return (f"PLACE: {fiche['name']}, as the world atlas describes it: {fiche['description']}\n\n"
            f"SUBJECT OF THIS SHEET: {disposition['subject']}.")


def bloc_c(disposition: dict) -> str:
    poses = grille(disposition)
    lignes = [
        f"LAYOUT — ONE TEXTURE SHEET: one {PLANCHE[0]}x{PLANCHE[1]} image, fully transparent "
        "background. Reference image 2 is the layout template: it shows "
        f"{len(poses)} cells as faint grey boxes, each with the blue outline of the floor "
        f"footprint its piece stands on (diamonds {2 * DEMI_L * PAS}x{2 * DEMI_H * PAS} px). Draw "
        "exactly one piece in each cell, standing exactly on its footprint outline, never crossing "
        "its box. Do not draw the boxes, the outlines or anything between the cells.",
        "Floor pieces fill their diamond exactly, edge to edge, and nothing outside it. Standing "
        "pieces rest on their footprint and include no floor under them.",
        "Cells, read left to right, then top to bottom:",
    ]
    for rang, pose in enumerate(poses, 1):
        cellule = pose["cellule"]
        classe = disposition["classes"][cellule["class"]]
        hausse = classe["rise"] * PAS
        detail = f"at most {hausse} px above its footprint" if hausse else "flat"
        lignes.append(f"{rang}. {cellule['prompt']} ({classe['description']}; {detail}).")
    return "\n".join(lignes)


def prompt(disposition: dict) -> str:
    return "\n\n".join([STYLE.read_text(encoding="utf-8").strip(), bloc_b(disposition), bloc_c(disposition)])


# -- Les images ----------------------------------------------------------------------------------

def _pil():
    import numpy as np
    from PIL import Image, ImageDraw
    return np, Image, ImageDraw


def gabarit(disposition: dict):
    """Le gabarit de la planche : boîtes grises, emprises en contour bleu, fond transparent."""
    _, Image, ImageDraw = _pil()
    image = Image.new("RGBA", PLANCHE, (0, 0, 0, 0))
    trait = ImageDraw.Draw(image)
    for pose in grille(disposition):
        trait.rectangle([pose["boite"][0], pose["boite"][1], pose["boite"][2] - 1, pose["boite"][3] - 1],
                        outline=(150, 150, 150, 255))
        ox, oy = pose["origine"]
        points = [(ox + x * PAS, oy + y * PAS) for x, y in sommets(disposition, pose["cellule"])]
        trait.polygon(points, outline=(60, 90, 200, 255))
    return image


def mettre_au_format(candidat):
    """La planche reçue, à la taille du prompt, en RGBA à alpha binaire."""
    np, Image, _ = _pil()
    image = candidat.convert("RGBA")
    if image.size != PLANCHE:
        image = image.resize(PLANCHE, Image.LANCZOS)
    pixels = np.asarray(image).copy()
    alpha = pixels[..., 3]
    if alpha.min() >= 250:
        raise DispositionError("la planche reçue n'a pas de fond transparent : le prompt le demande, "
                               "la génération est à refaire.")
    opaque = alpha > ALPHA
    pixels[..., 3] = np.where(opaque, 255, 0)
    pixels[~opaque] = 0
    return Image.fromarray(pixels)


def _masque_emprise(disposition: dict, cellule: dict):
    np, Image, ImageDraw = _pil()
    w, h = canevas(disposition, cellule)
    masque = Image.new("L", (w, h), 0)
    ImageDraw.Draw(masque).polygon(sommets(disposition, cellule), fill=255, outline=255)
    return np.asarray(masque) > 0


def decouper(disposition: dict, planche) -> tuple[dict, dict, list[str], list[str]]:
    """Les textures d'une planche au format : {clé: image}, manifeste, erreurs, avertissements."""
    np, Image, _ = _pil()
    source = np.asarray(planche)
    brutes, erreurs, avertissements = {}, [], []
    for pose in grille(disposition):
        cellule = pose["cellule"]
        k = cle(disposition, cellule)
        w, h = canevas(disposition, cellule)
        ox, oy = pose["origine"]
        x0, y0, x1, y1 = pose["boite"]
        boite = source[y0:y1, x0:x1, 3] > 0
        interieur = np.zeros_like(boite)
        interieur[oy - y0:oy - y0 + h * PAS, ox - x0:ox - x0 + w * PAS] = True
        if (boite & ~interieur).sum() > 0:
            avertissements.append(f"{k} : {int((boite & ~interieur).sum())} px d'écran hors du canevas, "
                                  "rognés.")
        bloc = source[oy:oy + h * PAS, ox:ox + w * PAS].astype(np.float32)
        # 2 × 2 -> 1 : couleur moyenne des pixels opaques du bloc, opaque si la moitié l'est
        bloc = bloc.reshape(h, PAS, w, PAS, 4)
        opaques = bloc[..., 3] > 0
        compte = opaques.sum(axis=(1, 3))
        somme = (bloc[..., :3] * opaques[..., None]).sum(axis=(1, 3))
        art = np.zeros((h, w, 4), dtype=np.uint8)
        pleins = compte * 2 >= PAS * PAS
        art[..., :3] = np.where(pleins[..., None], somme / np.maximum(compte, 1)[..., None], 0)
        art[..., 3] = np.where(pleins, 255, 0)
        if disposition["classes"][cellule["class"]]["rise"] == 0:
            masque = _masque_emprise(disposition, cellule)
            art[~masque] = 0
            couverture = (art[..., 3] > 0)[masque].mean()
            if couverture < COUVERTURE_SOL:
                erreurs.append(f"{k} : le sol couvre {couverture:.0%} de son losange, "
                               f"{COUVERTURE_SOL:.0%} attendus.")
        if not art[..., 3].any():
            erreurs.append(f"{k} : cellule vide.")
        brutes[k] = (pose, art)

    # une palette commune : quantifier cellule par cellule donnerait 36 palettes voisines
    opaques = [art[art[..., 3] > 0][:, :3] for _, art in brutes.values()]
    textures, fichiers = {}, {}
    if any(len(o) for o in opaques):
        mosaique = np.concatenate([o for o in opaques if len(o)]).reshape(-1, 1, 3)
        palette = Image.fromarray(mosaique.astype(np.uint8), "RGB").quantize(COULEURS, Image.Quantize.MEDIANCUT)
        for k, (pose, art) in brutes.items():
            rgb = Image.fromarray(art[..., :3], "RGB").quantize(palette=palette, dither=Image.Dither.NONE)
            finale = np.dstack([np.asarray(rgb.convert("RGB")), art[..., 3]])
            finale[finale[..., 3] == 0] = 0
            textures[k] = Image.fromarray(finale.astype(np.uint8), "RGBA")
            cellule = pose["cellule"]
            fichiers[k] = {
                "file": f"{cellule['name']}.png",
                "class": cellule["class"],
                "footprint": list(emprise(disposition, cellule)),
                "size": list(canevas(disposition, cellule)),
                # le sommet haut de l'emprise : le coin (0, 0) de la case qui porte la pièce
                "anchor": list(sommets(disposition, cellule)[0]),
                "sheet": list(pose["boite"]),
            }
    manifeste = {
        "version": 1,
        "disposition": disposition["id"],
        "source": "planche.png",
        "sourceSha256": hashlib.sha256(_png(planche)).hexdigest(),
        "tile": [2 * DEMI_L, 2 * DEMI_H],
        "colors": COULEURS,
        "textures": fichiers,
    }
    return textures, manifeste, erreurs, avertissements


def _png(image) -> bytes:
    tampon = io.BytesIO()
    image.save(tampon, format="PNG", optimize=True)
    return tampon.getvalue()


def _json(donnee: dict) -> str:
    return json.dumps(donnee, ensure_ascii=False, indent=2) + "\n"


# -- Commandes -----------------------------------------------------------------------------------

def commande(identifiant: str, tour: int) -> int:
    disposition = charger(identifiant)
    fautes = valider(disposition)
    if fautes:
        print("\n".join("ERREUR " + f for f in fautes))
        return 1
    dossier = TRAVAIL / "chatgpt" / f"{identifiant}-tour{tour}"
    if dossier.exists():
        shutil.rmtree(dossier)
    dossier.mkdir(parents=True)
    (dossier / "prompt.txt").write_text(prompt(disposition) + "\n", encoding="utf-8")
    shutil.copy(MAQUETTE, dossier / "1_maquette.png")
    gabarit(disposition).save(dossier / "2_gabarit.png")
    cible = TRAVAIL / identifiant / f"tour{tour}" / "candidat1.png"
    cible.parent.mkdir(parents=True, exist_ok=True)
    (dossier / "A_ENREGISTRER_SOUS.txt").write_text(f"{cible}\n", encoding="utf-8")
    print(f"{dossier} : 2 pièces jointes, {len(prompt(disposition))} caractères -> {cible}")
    return 0


def decoupe(identifiant: str, candidat: Path) -> int:
    _, Image, _ = _pil()
    disposition = charger(identifiant)
    planche = mettre_au_format(Image.open(candidat))
    textures, manifeste, erreurs, avertissements = decouper(disposition, planche)
    for a in avertissements:
        print("AVERTISSEMENT " + a)
    if erreurs:
        print("\n".join("ERREUR " + e for e in erreurs))
        print(f"{identifiant} : {len(erreurs)} faute(s), rien n'est installé.")
        return 1
    racine = RACINE / disposition["installRoot"]
    racine.mkdir(parents=True, exist_ok=True)
    for ancien in racine.glob("*.png"):
        ancien.unlink()
    (racine / "planche.png").write_bytes(_png(planche))
    for k, image in textures.items():
        (racine / manifeste["textures"][k]["file"]).write_bytes(_png(image))
    (racine / "manifest.json").write_text(_json(manifeste), encoding="utf-8", newline="\n")
    print(f"{identifiant} : {len(textures)} textures installées sous {racine}.")
    return 0


def verifier(identifiant: str) -> int:
    """Refait la découpe depuis la planche installée et compare, sans rien écrire."""
    np, Image, _ = _pil()
    disposition = charger(identifiant)
    racine = RACINE / disposition["installRoot"]
    fautes = []
    planche = Image.open(racine / "planche.png").convert("RGBA")
    textures, manifeste, erreurs, _ = decouper(disposition, planche)
    fautes += erreurs
    installe = (racine / "manifest.json").read_text(encoding="utf-8") if (racine / "manifest.json").is_file() else ""
    if installe != _json(manifeste):
        fautes.append("manifest.json diffère de la découpe.")
    attendus = {manifeste["textures"][k]["file"] for k in textures}
    presents = {p.name for p in racine.glob("*.png")} - {"planche.png"}
    for nom in sorted(presents - attendus):
        fautes.append(f"{nom} : installé, qu'aucune cellule ne produit.")
    for k, image in textures.items():
        chemin = racine / manifeste["textures"][k]["file"]
        if not chemin.is_file():
            fautes.append(f"{chemin.name} : absent.")
        elif not np.array_equal(np.asarray(Image.open(chemin).convert("RGBA")), np.asarray(image)):
            fautes.append(f"{chemin.name} : pixels différents de la découpe.")
    for f in fautes:
        print("ERREUR " + f)
    print(f"{identifiant} : " + (f"{len(fautes)} écart(s)." if fautes else f"{len(textures)} textures à jour."))
    return 1 if fautes else 0


def main(argv: list[str] | None = None) -> int:
    args = sys.argv[1:] if argv is None else argv
    try:
        if args[:1] == ["valider"] and len(args) == 1:
            nombre, fautes = valider_tout()
            for f in fautes:
                print("ERREUR " + f)
            print(f"dispositions : {nombre} clés de scène" + (f", {len(fautes)} faute(s)." if fautes else ", conformes."))
            return 1 if fautes else 0
        if args[:1] == ["--check"] and len(args) == 2:
            return verifier(args[1])
        if args[:1] == ["commande"] and len(args) == 3 and args[2].isdigit():
            return commande(args[1], int(args[2]))
        if args[:1] == ["decoupe"] and len(args) == 3:
            return decoupe(args[1], Path(args[2]))
    except DispositionError as erreur:
        print("ERREUR " + str(erreur))
        return 1
    print(__doc__)
    return 2


if __name__ == "__main__":
    sys.exit(main())
