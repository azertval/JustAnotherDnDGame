#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""Illustrations du corpus livrées comme assets d'interface (LOT-67).

Une **illustration**, et non de l'habillage. La distinction n'est pas de vocabulaire, elle décide
de l'outil : l'habillage ornemental — encadrements, cabochons, bandeaux — se **trace**
(`EX-IHM-075`, `LOT-76`), parce qu'un ornement doit se redessiner à toute taille et suivre les
jetons de la palette. Une carte du monde peinte ne se trace pas. Elle se prend telle quelle, ou
elle n'existe pas.

Ce module porte donc ce que le `LOT-76` a écarté : les images que le corpus est seul à pouvoir
donner.

Le catalogue n'en compte qu'une
------------------------------

La **carte du monde de Tanares**, double page 88-89 du `Tanares_Sourcebook`, qui porte les treize
régions de l'atlas du `LOT-37` — c'est le fond du menu principal. Une seule entrée, et ce n'est pas
un début de collection : chaque illustration livrée doit avoir un **consommateur**, sous peine
d'être un poids mort que personne ne sait dater.

Du JPEG, et c'est délibéré
--------------------------

Le reste des assets du dépôt est en PNG, qui est le bon format pour une planche de pixels exacts.
Une carte **peinte** n'en est pas une : son PNG pèse 4,4 Mo là où son JPEG en pèse 0,7, pour une
différence que personne ne voit — surtout derrière le voile qui assombrit un fond de menu. Le poids
du dépôt est un sujet du corpus depuis le début (`EX-CNT-023`) ; l'ignorer ici serait tenir le
discours et faire l'inverse.

Le filigrane et le folio sont recadrés, pas effacés
---------------------------------------------------

La page porte en bas à gauche un filigrane d'achat nominatif et son numéro de folio. Les **retirer**
demanderait de repeindre ce qu'ils recouvrent, c'est-à-dire d'inventer des pixels. La région
déclarée s'arrête donc **avant** eux : ce qui est livré est de la carte, entièrement, et rien
d'autre.
"""
from __future__ import annotations

import hashlib
import io
import json
from dataclasses import dataclass

from .corpus import Corpus
from .extraction import Extracteur, ExtractionError

SORTIE_UI = 'Source/Elements/Assets/UI'
MANIFESTE = 'illustrations.json'

# Resolution de rendu. La carte est un FOND, dessine derriere un voile et jamais lu : ses toponymes
# sont de la decoration, pas de l'information. Le jour ou la carte du monde devient un ecran qu'on
# consulte (`LOT-42`), ce lot-la extraira la sienne, a la resolution qu'il lui faut.
PPP = 96

# Qualite JPEG. 88 : au-dessus, le fichier grossit sans que rien ne change a l'oeil ; en dessous,
# les aplats d'eau prennent des blocs visibles meme assombris.
QUALITE = 88


class IllustrationsError(Exception):
    """Une illustration n'a pas pu être produite."""


@dataclass(frozen=True)
class Illustration:
    """Une illustration : d'où elle vient, à quoi elle sert."""

    cle: str
    document: str
    page: int  # index de page PDF, 0-indexé
    region: tuple  # x0, y0, x1, y1 en points PDF
    note: str

    @property
    def fichier(self) -> str:
        return self.cle + '.jpg'


CATALOGUE: list = [
    Illustration(
        cle='world-map',
        document='tanares-sourcebook',
        page=44,
        # La double page entiere, moins la marge basse : le filigrane d'achat et le folio y vivent,
        # et la region s'arrete avant eux plutot que de les recouvrir.
        region=(8.0, 6.0, 1216.0, 748.0),
        note="carte du monde de Tanares (pages imprimees 88-89), les treize regions de l'atlas"),
]


def produire(corpus: Corpus, racine, cache=None) -> dict:
    """Rendre les illustrations du catalogue et écrire le manifeste à côté d'elles."""
    try:
        from PIL import Image
    except ImportError as erreur:  # pragma: no cover - dépend de l'environnement
        raise IllustrationsError(
            'Pillow est requis pour encoder le JPEG : pip install pillow') from erreur

    racine.mkdir(parents=True, exist_ok=True)
    entrees = []
    octets = 0
    for illustration in CATALOGUE:
        document = corpus[illustration.document]
        document.verifier()  # EX-CNT-020 : jamais de rendu sur une autre édition.
        with Extracteur(document, cache=cache) as extracteur:
            try:
                rendu = extracteur.image(illustration.page, illustration.region, None, PPP)
            except ExtractionError as erreur:
                raise IllustrationsError('%s : %s' % (illustration.cle, erreur)) from erreur

        # Le rendu est un PNG (EX-CNT-022 : une image se REND, elle ne se tire pas d'un flux brut) ;
        # seul l'encodage final change. Aucun rééchantillonnage : la taille livrée est celle rendue.
        image = Image.open(io.BytesIO(rendu)).convert('RGB')
        tampon = io.BytesIO()
        image.save(tampon, format='JPEG', quality=QUALITE, optimize=True, subsampling=1)
        donnees = tampon.getvalue()

        (racine / illustration.fichier).write_bytes(donnees)
        octets += len(donnees)
        entrees.append({
            'id': illustration.cle,
            'file': illustration.fichier,
            'document': document.fichier,
            'provenance': document.provenance,
            'page': illustration.page,
            'region': list(illustration.region),
            'size': [image.width, image.height],
            'dpi': PPP,
            'quality': QUALITE,
            'bytes': len(donnees),
            'sha256': hashlib.sha256(donnees).hexdigest(),
            'note': illustration.note,
        })

    (racine / MANIFESTE).write_text(
        json.dumps({'version': 1, 'illustrations': entrees}, indent=2, ensure_ascii=False) + '\n',
        encoding='utf-8')
    return {'illustrations': len(entrees), 'octets': octets}
