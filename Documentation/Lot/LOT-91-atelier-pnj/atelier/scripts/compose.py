"""Assemble la planche finale d'un PNJ (méthode standard, poc.md §4bis, étape A).
Usage : py -3.13 compose.py <slug> <tour_planche> [<tour_marche>] [<anim>=<tour> ...]
Prend <slug>/tour<P>/norm (planche normalisée) ; chaque passe séparée remplace ses rangées et sa bande :
  <tour_marche>  <slug>/tour<M>/norm/marche.* (rangée 2, walk) ; équivaut à walk=<M> ;
  attack=4       <slug>/tour4/norm/attack.* (rangées 5-6) ; de même idle, hit, death, cast.
Écrit <slug>/final/ : planche.png (1536x2048), bandes/*.png, planche.json. Les erreurs de la planche qui
portent sur une animation remplacée ne comptent plus (Nakral tour 3 : walk illisible, refait en M).
Sans passe, la planche est gardée telle quelle (Lizz, tour 1)."""
import json
import shutil
import sys
import numpy as np
from PIL import Image
from prompts import ROOT

RANGEES = {"idle": [0], "walk": [1], "hit": [2], "death": [3], "attack": [4, 5], "cast": [6, 7]}
slug, tp = sys.argv[1], int(sys.argv[2])
passes = {}
for a in sys.argv[3:]:
    if "=" in a: anim, t = a.split("="); passes[anim] = int(t)
    else: passes["walk"] = int(a)
P, F = ROOT / slug / f"tour{tp}" / "norm", ROOT / slug / "final"
rp = json.load(open(P / "planche.json", encoding="utf-8"))
planche = np.asarray(Image.open(P / "planche.png").convert("RGBA")).copy()
(F / "bandes").mkdir(parents=True, exist_ok=True)
for b in (P / "bandes").glob("*.png"):
    if b.stem not in passes: shutil.copy(b, F / "bandes" / b.name)


def concerne(message, anim):
    return message.startswith(f"{anim} ") or f"({anim})" in message or (anim == "walk" and message.startswith("walk bouge peu"))


erreurs = [e for e in rp["erreurs"] if not any(concerne(e, a) for a in passes)]
avert = [e for e in rp["avertissements"] if not any(concerne(e, a) for a in passes)]
images = [i for i in rp["images"] if i["animation"] not in passes]
sources, facteurs = [f"{slug}/tour{tp}/norm/planche.png"], {f"tour{tp}": rp["facteur"]}
for anim, t in passes.items():
    dispo = "marche" if anim == "walk" else anim
    M = ROOT / slug / f"tour{t}" / "norm"
    rm = json.load(open(M / f"{dispo}.json", encoding="utf-8"))
    rangs = RANGEES[anim]
    planche[256 * rangs[0]:256 * (rangs[-1] + 1)] = np.asarray(Image.open(M / f"{dispo}.png").convert("RGBA"))
    shutil.copy(M / "bandes" / f"{anim}.png", F / "bandes" / f"{anim}.png")
    images += [dict(i, rangee=i["rangee"] + rangs[0]) for i in rm["images"]]
    erreurs += rm["erreurs"]; avert += rm["avertissements"]
    sources.append(f"{slug}/tour{t}/norm/{dispo}.png ({anim}, rangées {', '.join(str(r + 1) for r in rangs)})")
    facteurs[f"tour{t}:{anim}"] = rm["facteur"]
Image.fromarray(planche).save(F / "planche.png")
json.dump({"source": " + ".join(sources), "disposition": "planche", "facteurs": facteurs, "erreurs": erreurs,
           "avertissements": avert, "images": sorted(images, key=lambda i: (i["rangee"], i["image"]))},
          open(F / "planche.json", "w", encoding="utf-8"), indent=2, ensure_ascii=False)
print(f"{F.relative_to(ROOT)} : {len(images)} images ; erreurs {len(erreurs)}")
for e in erreurs: print("ERREUR", e)
sys.exit(1 if erreurs else 0)
