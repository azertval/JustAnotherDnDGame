#!/usr/bin/env python3
# SPDX-FileCopyrightText: 2026 Valentin Eloy
# SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
"""Atelier des textures (LOT-92, T3) : les planches de scène d'un lieu, déclarées par sa disposition.

`extract_coliseum_atlas.py` (LOT-50) découpait UNE planche à des coordonnées écrites à la main,
relevées sur l'image après coup. Ici la disposition vient AVANT l'image : un JSON
(`atelier/dispositions/<id>.json`) déclare la taille de ses planches et leur pas de pixel, puis
chaque cellule -- son nom, sa classe (sol, pièce haute, grande pièce), son emprise en cases et la
hauteur qu'elle prend au-dessus du sol -- et tout le reste s'en déduit : la répartition des cellules
sur une ou plusieurs planches, le gabarit de chaque planche, le bloc C du prompt, la découpe et la
clé de chaque texture. Aucune coordonnée n'est écrite à la main.

La géométrie est celle d'`IsoProjection` : un losange de 68 × 42 pixels d'art (rapport 0,62). Une
emprise de a × b cases est un parallélogramme de (a + b) · 34 × (a + b) · 21 pixels d'art ; son
sommet haut est le coin (0, 0) de la grille, un pas en x descend de (34, 21), un pas en y de
(−34, 21). Une pièce haute ajoute sa hauteur au-dessus. Le pas (`sheet.scale`, pixels d'écran par
pixel d'art) ne change pas la texture installée, qui est en pixels d'art : il donne au générateur
plus de pixels pour dessiner chacun d'eux, et la réduction en est plus propre.

La taille d'une planche suit les contraintes du générateur (`gpt-image-2` et suivants) : côtés
multiples de 16, au plus 3840 px, rapport au plus 3 : 1, entre 655 360 et 8 294 400 pixels ; au-delà
de 2560 × 1440 (3 686 400 pixels), le rendu est donné pour expérimental.

Commandes :
    py -3.13 scripts/extract_texture_sheet.py commande <id> <tour>
        prépare l'envoi à la main de chaque planche : prompt (blocs A, B, C), maquette et gabarit,
        sous <TEXTURE_ATELIER>/chatgpt/<id>-tour<K>/planche-<N>/ ;
    py -3.13 scripts/extract_texture_sheet.py decoupe <id> <planche-1.png> [<planche-2.png> ...]
        met les planches reçues au format, les découpe et les installe sous `installRoot` ;
    py -3.13 scripts/extract_texture_sheet.py --check <id>
        refait la découpe en mémoire depuis les planches installées et compare, SANS rien écrire ;
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
PAS_MAQUETTE = 2
LIEUX = RACINE / "Source" / "Elements" / "World" / "locations"
TRAVAIL = Path(os.environ.get("TEXTURE_ATELIER", r"D:/JustAnotherDnDGame-textures"))

DEMI_L, DEMI_H = 34, 21     # demi-losange, pixels d'art (68 × 42, IsoProjection 0,62)
MARGE = 4                   # pixels d'art autour de chaque cellule
COULEURS = 64               # palette commune à toutes les planches d'un lieu
COUVERTURE_SOL = 0.97       # part minimale du losange qu'un sol doit remplir
ALPHA = 127                 # seuil de binarisation, comme au LOT-91

# Le générateur (documentation d'OpenAI, gpt-image-2 et suivants).
COTE_MULTIPLE, COTE_MAX, RAPPORT_MAX = 16, 3840, 3
PIXELS_MIN, PIXELS_MAX, PIXELS_FIABLES = 655_360, 8_294_400, 3_686_400

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


def taille_planche(disposition: dict) -> tuple[int, int]:
    w, h = disposition["sheet"]["size"]
    return int(w), int(h)


def pas(disposition: dict) -> int:
    return int(disposition["sheet"]["scale"])


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
    """Les cellules posées sur les planches, dans l'ordre de la disposition.

    Rangées remplies de gauche à droite ; une rangée prend la hauteur de sa plus haute cellule ; une
    rangée qui ne tient plus sur la planche ouvre la suivante. Chaque entrée : la cellule,
    `planche` (numéro à partir de 1), `boite` (x0, y0, x1, y1) marge comprise et `origine`, le coin
    haut-gauche du canevas, en pixels d'écran ; le canevas est posé en bas de sa boîte.
    """
    largeur, hauteur = taille_planche(disposition)
    s = pas(disposition)
    poses: list[dict] = []
    numero, x, y, h_rangee, rangee = 1, 0, 0, 0, []

    def fermer():
        nonlocal numero, y
        if not rangee:
            return
        if y + h_rangee > hauteur:
            numero, y = numero + 1, 0
        for x0, w, cellule in rangee:
            _, ch = canevas(disposition, cellule)
            poses.append({
                "cellule": cellule,
                "planche": numero,
                "boite": (x0, y, x0 + w, y + h_rangee),
                "origine": (x0 + MARGE * s, y + h_rangee - MARGE * s - ch * s),
            })
        y += h_rangee

    for cellule in disposition["cells"]:
        cw, ch = canevas(disposition, cellule)
        w, h = (cw + 2 * MARGE) * s, (ch + 2 * MARGE) * s
        if w > largeur or h > hauteur:
            raise DispositionError(f"{disposition['id']} : la cellule « {cellule['name']} » "
                                   f"({w} × {h} px) ne tient pas dans une planche de "
                                   f"{largeur} × {hauteur} au pas {s}.")
        if x + w > largeur:
            fermer()
            x, h_rangee, rangee = 0, 0, []
        rangee.append((x, w, cellule))
        x += w
        h_rangee = max(h_rangee, h)
    fermer()
    return poses


def planches(disposition: dict) -> int:
    return max(p["planche"] for p in grille(disposition))


def cle(disposition: dict, cellule: dict) -> str:
    return f"{disposition['keyPrefix']}/{cellule['name']}"


def fautes_de_taille(largeur: int, hauteur: int) -> list[str]:
    fautes = []
    if largeur % COTE_MULTIPLE or hauteur % COTE_MULTIPLE:
        fautes.append(f"côtés multiples de {COTE_MULTIPLE} attendus")
    if max(largeur, hauteur) > COTE_MAX:
        fautes.append(f"côté au plus {COTE_MAX}")
    if max(largeur, hauteur) > RAPPORT_MAX * min(largeur, hauteur):
        fautes.append(f"rapport au plus {RAPPORT_MAX} : 1")
    if not PIXELS_MIN <= largeur * hauteur <= PIXELS_MAX:
        fautes.append(f"entre {PIXELS_MIN} et {PIXELS_MAX} pixels")
    return fautes


def valider(disposition: dict) -> list[str]:
    """Les fautes d'une disposition ; vide si elle est conforme."""
    fautes: list[str] = []
    ident = disposition.get("id", "?")
    if disposition.get("version") != 1:
        fautes.append(f"{ident} : version {disposition.get('version')!r}, 1 attendue.")
    for champ in ("id", "title", "location", "subject", "keyPrefix", "installRoot", "sheet",
                  "classes", "cells"):
        if champ not in disposition:
            fautes.append(f"{ident} : champ « {champ} » absent.")
    if fautes:
        return fautes
    try:
        largeur, hauteur = taille_planche(disposition)
        if pas(disposition) < 1:
            fautes.append(f"{ident} : pas de pixel {pas(disposition)}, au moins 1.")
        for f in fautes_de_taille(largeur, hauteur):
            fautes.append(f"{ident} : planche {largeur} × {hauteur} refusée par le générateur ({f}).")
    except (KeyError, TypeError, ValueError):
        fautes.append(f"{ident} : « sheet » attend « size » [largeur, hauteur] et « scale ».")
        return fautes
    if not CLE_RE.match(disposition["keyPrefix"] + "/x"):
        fautes.append(f"{ident} : préfixe de clé « {disposition['keyPrefix']} » invalide (LOT-39).")
    if not disposition["keyPrefix"].startswith("scene/"):
        fautes.append(f"{ident} : une texture de scène a une clé « scene/… ».")
    fiche = LIEUX / f"{disposition['location']}.json"
    if not fiche.is_file():
        fautes.append(f"{ident} : lieu « {disposition['location']} » absent de l'atlas : le bloc B "
                      "se rédige depuis sa fiche.")
    else:
        description = json.loads(fiche.read_text(encoding="utf-8"))["description"]
        extraits = disposition.get("locationExcerpt", [])
        if not isinstance(extraits, list) or not all(isinstance(e, str) and e for e in extraits):
            fautes.append(f"{ident} : « locationExcerpt » attend une liste de phrases.")
        else:
            for extrait in extraits:
                if extrait not in description:
                    fautes.append(f"{ident} : extrait absent de la fiche « {disposition['location']} » "
                                  f"(le bloc B cite le livre, il ne le récrit pas) : « {extrait[:60]}… »")
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
    for fichier in sorted(DISPOSITIONS.glob("*.json")):
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


def bloc_a(disposition: dict) -> str:
    s = pas(disposition)
    lignes = [ligne for ligne in STYLE.read_text(encoding="utf-8").splitlines()
              if not ligne.startswith("#")]
    return ("\n".join(lignes).strip()
            .replace("{PAS}", str(s))
            .replace("{LOSANGE_L}", str(2 * DEMI_L * s))
            .replace("{LOSANGE_H}", str(2 * DEMI_H * s)))


def bloc_b(disposition: dict) -> str:
    """Le lieu, cité du livre : la fiche d'atlas entière, ou les seules phrases de `locationExcerpt`.

    Une planche peut ne montrer qu'une partie de son lieu -- l'Arène du Destin n'est qu'une phrase de
    la fiche d'Arenarea. Recopier toute la fiche ferait dessiner au générateur les casinos et les
    fontaines du quartier ; les extraits restent des citations, vérifiées mot pour mot par `valider`.
    """
    fiche = json.loads((LIEUX / f"{disposition['location']}.json").read_text(encoding="utf-8"))
    extraits = disposition.get("locationExcerpt")
    texte = " ".join(extraits) if extraits else fiche["description"]
    return (f"PLACE: {fiche['name']}, as the world atlas describes it: {texte}\n\n"
            f"SUBJECT OF THIS SHEET: {disposition['subject']}.")


def bloc_c(disposition: dict, numero: int = 1) -> str:
    largeur, hauteur = taille_planche(disposition)
    s = pas(disposition)
    poses = [p for p in grille(disposition) if p["planche"] == numero]
    lignes = [
        f"LAYOUT — ONE TEXTURE SHEET: one {largeur}x{hauteur} image, fully transparent "
        "background. Reference image 2 is the layout template: it shows "
        f"{len(poses)} cells as faint grey boxes, each with the blue outline of the floor "
        f"footprint its piece stands on (diamonds {2 * DEMI_L * s}x{2 * DEMI_H * s} px). Draw "
        "exactly one piece in each cell, standing exactly on its footprint outline, never crossing "
        "its box. Do not draw the boxes, the outlines or anything between the cells.",
        f"Reference image 1 is drawn at {PAS_MAQUETTE} screen pixels per art pixel; this sheet is "
        f"drawn at {s}: keep its style, not its pixel size.",
        "Floor pieces fill their diamond exactly, edge to edge, and nothing outside it. Standing "
        "pieces rest on their footprint and include no floor under them.",
        "Cells, read left to right, then top to bottom:",
    ]
    for rang, pose in enumerate(poses, 1):
        cellule = pose["cellule"]
        classe = disposition["classes"][cellule["class"]]
        hausse = classe["rise"] * s
        detail = f"at most {hausse} px above its footprint" if hausse else "flat"
        lignes.append(f"{rang}. {cellule['prompt']} ({classe['description']}; {detail}).")
    return "\n".join(lignes)


def prompt(disposition: dict, numero: int = 1) -> str:
    return "\n\n".join([bloc_a(disposition), bloc_b(disposition), bloc_c(disposition, numero)])


# -- Les images ----------------------------------------------------------------------------------

def _pil():
    import numpy as np
    from PIL import Image, ImageDraw
    return np, Image, ImageDraw


def gabarit(disposition: dict, numero: int = 1):
    """Le gabarit d'une planche : boîtes grises, emprises en contour bleu, fond transparent."""
    _, Image, ImageDraw = _pil()
    s = pas(disposition)
    image = Image.new("RGBA", taille_planche(disposition), (0, 0, 0, 0))
    trait = ImageDraw.Draw(image)
    for pose in grille(disposition):
        if pose["planche"] != numero:
            continue
        x0, y0, x1, y1 = pose["boite"]
        trait.rectangle([x0, y0, x1 - 1, y1 - 1], outline=(150, 150, 150, 255))
        ox, oy = pose["origine"]
        points = [(ox + x * s, oy + y * s) for x, y in sommets(disposition, pose["cellule"])]
        trait.polygon(points, outline=(60, 90, 200, 255))
    return image


def mettre_au_format(disposition: dict, candidat):
    """Une planche reçue, à la taille de la disposition, en RGBA à alpha binaire."""
    np, Image, _ = _pil()
    image = candidat.convert("RGBA")
    if image.size != taille_planche(disposition):
        image = image.resize(taille_planche(disposition), Image.LANCZOS)
    pixels = np.asarray(image).copy()
    alpha = pixels[..., 3]
    if alpha.min() >= 250:
        raise DispositionError("une planche reçue n'a pas de fond transparent : le prompt le "
                               "demande, la génération est à refaire.")
    opaque = alpha > ALPHA
    pixels[..., 3] = np.where(opaque, 255, 0)
    pixels[~opaque] = 0
    return Image.fromarray(pixels)


def _masque_emprise(disposition: dict, cellule: dict):
    np, Image, ImageDraw = _pil()
    masque = Image.new("L", canevas(disposition, cellule), 0)
    ImageDraw.Draw(masque).polygon(sommets(disposition, cellule), fill=255, outline=255)
    return np.asarray(masque) > 0


def decouper(disposition: dict, planches_recues: list) -> tuple[dict, dict, list[str], list[str]]:
    """Les textures des planches au format : {clé: image}, manifeste, erreurs, avertissements."""
    np, Image, _ = _pil()
    s = pas(disposition)
    attendu = planches(disposition)
    if len(planches_recues) != attendu:
        raise DispositionError(f"{disposition['id']} : {len(planches_recues)} planche(s) reçue(s), "
                               f"{attendu} attendue(s).")
    sources = [np.asarray(p) for p in planches_recues]
    brutes, erreurs, avertissements = {}, [], []
    for pose in grille(disposition):
        cellule = pose["cellule"]
        source = sources[pose["planche"] - 1]
        k = cle(disposition, cellule)
        w, h = canevas(disposition, cellule)
        ox, oy = pose["origine"]
        x0, y0, x1, y1 = pose["boite"]
        boite = source[y0:y1, x0:x1, 3] > 0
        interieur = np.zeros_like(boite)
        interieur[oy - y0:oy - y0 + h * s, ox - x0:ox - x0 + w * s] = True
        if (boite & ~interieur).sum() > 0:
            avertissements.append(f"{k} : {int((boite & ~interieur).sum())} px d'écran hors du "
                                  "canevas, rognés.")
        # s × s -> 1 : couleur moyenne des pixels opaques du bloc, opaque si la moitié l'est
        bloc = source[oy:oy + h * s, ox:ox + w * s].astype(np.float32).reshape(h, s, w, s, 4)
        opaques = bloc[..., 3] > 0
        compte = opaques.sum(axis=(1, 3))
        somme = (bloc[..., :3] * opaques[..., None]).sum(axis=(1, 3))
        art = np.zeros((h, w, 4), dtype=np.uint8)
        pleins = compte * 2 >= s * s
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

    # une palette commune au lieu : quantifier cellule par cellule, ou planche par planche, donnerait
    # des palettes voisines qui ne s'accordent pas une fois les tuiles posées côte à côte
    opaques = [art[art[..., 3] > 0][:, :3] for _, art in brutes.values()]
    textures, fichiers = {}, {}
    if any(len(o) for o in opaques):
        mosaique = np.concatenate([o for o in opaques if len(o)]).reshape(-1, 1, 3)
        palette = Image.fromarray(mosaique.astype(np.uint8), "RGB").quantize(
            COULEURS, Image.Quantize.MEDIANCUT)
        for k, (pose, art) in brutes.items():
            rgb = Image.fromarray(art[..., :3], "RGB").quantize(palette=palette,
                                                                dither=Image.Dither.NONE)
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
                "sheet": pose["planche"],
                "box": list(pose["boite"]),
            }
    manifeste = {
        "version": 1,
        "disposition": disposition["id"],
        "sheets": [{"file": nom_planche(n), "sha256": hashlib.sha256(_png(p)).hexdigest()}
                   for n, p in enumerate(planches_recues, 1)],
        "sheetSize": list(taille_planche(disposition)),
        "scale": s,
        "tile": [2 * DEMI_L, 2 * DEMI_H],
        "colors": COULEURS,
        "textures": fichiers,
    }
    return textures, manifeste, erreurs, avertissements


def nom_planche(numero: int) -> str:
    return f"planche-{numero}.png"


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
    largeur, hauteur = taille_planche(disposition)
    if largeur * hauteur > PIXELS_FIABLES:
        print(f"AVERTISSEMENT {identifiant} : {largeur} × {hauteur} dépasse 2560 × 1440, rendu "
              "donné pour expérimental par le générateur.")
    racine = TRAVAIL / "chatgpt" / f"{identifiant}-tour{tour}"
    if racine.exists():
        shutil.rmtree(racine)
    for numero in range(1, planches(disposition) + 1):
        dossier = racine / f"planche-{numero}"
        dossier.mkdir(parents=True)
        texte = prompt(disposition, numero)
        (dossier / "prompt.txt").write_text(texte + "\n", encoding="utf-8")
        shutil.copy(MAQUETTE, dossier / "1_maquette.png")
        gabarit(disposition, numero).save(dossier / "2_gabarit.png")
        cible = TRAVAIL / identifiant / f"tour{tour}" / nom_planche(numero)
        cible.parent.mkdir(parents=True, exist_ok=True)
        (dossier / "A_ENREGISTRER_SOUS.txt").write_text(f"{cible}\n", encoding="utf-8")
        cellules = sum(1 for p in grille(disposition) if p["planche"] == numero)
        print(f"{dossier} : {cellules} cellules, {len(texte)} caractères -> {cible}")
    return 0


def decoupe(identifiant: str, candidats: list[Path]) -> int:
    _, Image, _ = _pil()
    disposition = charger(identifiant)
    recues = [mettre_au_format(disposition, Image.open(c)) for c in candidats]
    textures, manifeste, erreurs, avertissements = decouper(disposition, recues)
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
    for numero, planche in enumerate(recues, 1):
        (racine / nom_planche(numero)).write_bytes(_png(planche))
    for k, image in textures.items():
        (racine / manifeste["textures"][k]["file"]).write_bytes(_png(image))
    (racine / "manifest.json").write_text(_json(manifeste), encoding="utf-8", newline="\n")
    print(f"{identifiant} : {len(textures)} textures installées sous {racine}.")
    return 0


def verifier(identifiant: str) -> int:
    """Refait la découpe depuis les planches installées et compare, sans rien écrire."""
    np, Image, _ = _pil()
    disposition = charger(identifiant)
    racine = RACINE / disposition["installRoot"]
    noms = [nom_planche(n) for n in range(1, planches(disposition) + 1)]
    manquantes = [n for n in noms if not (racine / n).is_file()]
    if manquantes:
        print(f"ERREUR planches absentes : {', '.join(manquantes)}.")
        return 1
    recues = [Image.open(racine / n).convert("RGBA") for n in noms]
    textures, manifeste, fautes, _ = decouper(disposition, recues)
    chemin = racine / "manifest.json"
    if not chemin.is_file() or chemin.read_text(encoding="utf-8") != _json(manifeste):
        fautes.append("manifest.json diffère de la découpe.")
    attendus = {manifeste["textures"][k]["file"] for k in textures} | set(noms)
    for nom in sorted({p.name for p in racine.glob("*.png")} - attendus):
        fautes.append(f"{nom} : installé, qu'aucune cellule ne produit.")
    for k, image in textures.items():
        fichier = racine / manifeste["textures"][k]["file"]
        if not fichier.is_file():
            fautes.append(f"{fichier.name} : absent.")
        elif not np.array_equal(np.asarray(Image.open(fichier).convert("RGBA")), np.asarray(image)):
            fautes.append(f"{fichier.name} : pixels différents de la découpe.")
    for f in fautes:
        print("ERREUR " + f)
    print(f"{identifiant} : "
          + (f"{len(fautes)} écart(s)." if fautes else f"{len(textures)} textures à jour."))
    return 1 if fautes else 0


def main(argv: list[str] | None = None) -> int:
    args = sys.argv[1:] if argv is None else argv
    try:
        if args == ["valider"]:
            nombre, fautes = valider_tout()
            for f in fautes:
                print("ERREUR " + f)
            print(f"dispositions : {nombre} clés de scène"
                  + (f", {len(fautes)} faute(s)." if fautes else ", conformes."))
            return 1 if fautes else 0
        if args[:1] == ["--check"] and len(args) == 2:
            return verifier(args[1])
        if args[:1] == ["commande"] and len(args) == 3 and args[2].isdigit():
            return commande(args[1], int(args[2]))
        if args[:1] == ["decoupe"] and len(args) >= 3:
            return decoupe(args[1], [Path(a) for a in args[2:]])
    except DispositionError as erreur:
        print("ERREUR " + str(erreur))
        return 1
    print(__doc__)
    return 2


if __name__ == "__main__":
    sys.exit(main())
