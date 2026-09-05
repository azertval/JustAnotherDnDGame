#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""Extraction du corpus : texte, tableaux par coordonnée, images par rendu clippé, cache.

Trois constats de l'analyse du corpus (§4 de la feuille de route) sont **câblés ici**, et non
laissés à la discipline de l'appelant. C'est la raison d'être du module : chacun a été vérifié sur
le corpus, chacun produit sans lui une donnée fausse et silencieuse.

**Un tableau ne s'extrait pas en flux de texte** (`EX-CNT-021`). Sur la table des armes des
*Basic Rules*, un rendu en flux attribue le poids et le prix à l'arme de la ligne suivante ; sur la
table du barbare du *Manuel des Joueurs*, il décale toute la progression d'un cran, si bien que le
niveau 5 reçoit la capacité du niveau 4. ``tableau()`` regroupe donc les mots par **coordonnée** :
lignes par ordonnée, colonnes par profil de projection horizontale. Aucune fonction de ce module
ne renvoie un tableau autrement.

**Une image ne s'extrait pas de son flux brut** (`EX-CNT-022`). Tirer un objet image par son
``xref`` produit sur ce corpus des zones de bruit vert et cyan — décodage raté d'un flux JPX ou
d'un masque alpha. ``image()`` **rend** une région de page, ce qui passe par la composition
complète et donne un résultat exact. Corollaire assumé, et c'est pourquoi ``regions_candidates()``
existe : le rendu embarque tout ce qui est dessiné dans la région, texte compris. L'extraction
d'illustrations est donc **semi-automatique** — l'outil propose, l'humain recadre.

**Les livres Tanares sont paginés en double page.** ``moitie='gauche'|'droite'`` découpe la page
PDF en deux pages imprimées ; ``corpus.Document`` porte la correspondance.

Le **cache** disque (``--cache``) est indexé par empreinte du document : il ne peut donc pas
servir une réponse issue d'une autre édition. Deux exécutions successives produisent des sorties
identiques, avec ou sans lui (`EX-CNT-020`).

Dépendance : **PyMuPDF** (``pip install pymupdf``). Le poste dispose de `pdftotext` mais ni de
`pdfimages` ni de `pdftoppm` ; PyMuPDF couvre les trois besoins en une bibliothèque, et lui seul
donne accès aux coordonnées de mots dont ``tableau()`` dépend.
"""
from __future__ import annotations

import hashlib
from dataclasses import dataclass
from pathlib import Path

from .corpus import CorpusError, Document

try:  # PyMuPDF n'est pas installé sur le runner de CI : la CI ne touche pas au corpus.
    import pymupdf
except ImportError as _erreur:  # pragma: no cover - dépend de l'environnement
    pymupdf = None
    _IMPORT_ERREUR = _erreur
else:
    _IMPORT_ERREUR = None

# Écart horizontal, en points PDF, à partir duquel deux colonnes sont tenues pour séparées.
# 6 pt ≈ deux espaces au corps du texte de ce corpus : en dessous, deux mots d'une même cellule
# se retrouveraient dans deux colonnes ; bien au-dessus, deux colonnes serrées fusionnent.
ECART_COLONNE_DEFAUT = 6.0

# Résolution de rendu par défaut. 300 ppp est la résolution des planches de personnage du corpus
# (2 668 × 3 418 pour une page A4) : rendre au-delà n'ajoute aucune information.
PPP_DEFAUT = 300


class ExtractionError(Exception):
    """Une extraction n'a pas pu être menée à bien."""


@dataclass(frozen=True)
class Region:
    """Une région de page proposée par ``regions_candidates()``, en points PDF."""

    x0: float
    y0: float
    x1: float
    y1: float
    couvre_du_texte: bool

    @property
    def largeur(self) -> float:
        return self.x1 - self.x0

    @property
    def hauteur(self) -> float:
        return self.y1 - self.y0

    def __str__(self) -> str:
        marque = ' (recouvre du texte)' if self.couvre_du_texte else ''
        return (
            f'{self.x0:.0f},{self.y0:.0f},{self.x1:.0f},{self.y1:.0f} '
            f'[{self.largeur:.0f}×{self.hauteur:.0f}]{marque}'
        )


class Extracteur:
    """Ouvre un document du manifeste, après en avoir vérifié l'empreinte.

    La vérification est faite **à l'ouverture** et non à la demande : un extracteur construit est
    un extracteur dont on sait qu'il lit le bon document. La désactiver (``verifier=False``) n'a
    qu'un usage, le calcul de l'empreinte elle-même lors d'une régénération du manifeste.
    """

    def __init__(self, document: Document, cache: Path | None = None, verifier: bool = True) -> None:
        if pymupdf is None:  # pragma: no cover - dépend de l'environnement
            raise ExtractionError(
                'PyMuPDF est requis pour extraire le corpus : pip install pymupdf '
                f'({_IMPORT_ERREUR}).'
            )
        self.document = document
        if verifier:
            document.verifier()
        elif not document.chemin.is_file():
            raise CorpusError(f'{document.cle} : document absent — {document.chemin}.')
        self._pdf = pymupdf.open(document.chemin)
        if self._pdf.page_count != document.pages:
            raise CorpusError(
                f'{document.cle} : {self._pdf.page_count} pages, {document.pages} annoncées au '
                f'manifeste.'
            )
        self._cache = cache / document.sha256[:12] if cache else None
        if self._cache:
            self._cache.mkdir(parents=True, exist_ok=True)

    def __enter__(self) -> 'Extracteur':
        return self

    def __exit__(self, *_) -> None:
        self.fermer()

    def fermer(self) -> None:
        self._pdf.close()

    # -- Découpe en double page ------------------------------------------------------------

    def rectangle(self, index: int, moitie: str | None = None, region: tuple | None = None):
        """Rectangle de travail : la page, une de ses moitiés, ou une région explicite.

        ``moitie`` n'a de sens que sur un document en pagination double ; le demander ailleurs
        est une erreur d'appel, pas un cas à absorber en silence.
        """
        page = self._page(index)
        rect = page.rect
        if moitie is not None:
            if not self.document.double_page:
                raise ExtractionError(
                    f'{self.document.cle} : pagination {self.document.pagination}, la notion de '
                    f'moitié de page ne s\'y applique pas.'
                )
            milieu = (rect.x0 + rect.x1) / 2
            if moitie == 'gauche':
                rect = pymupdf.Rect(rect.x0, rect.y0, milieu, rect.y1)
            elif moitie == 'droite':
                rect = pymupdf.Rect(milieu, rect.y0, rect.x1, rect.y1)
            else:
                raise ExtractionError(f'moitié {moitie!r} inconnue (gauche, droite).')
        if region is not None:
            rect = rect & pymupdf.Rect(*region)
            if rect.is_empty:
                raise ExtractionError(f'région {region} hors de la page {index}.')
        return rect

    # -- Texte -----------------------------------------------------------------------------

    def texte(
        self,
        index: int,
        moitie: str | None = None,
        region: tuple | None = None,
        tri: bool = True,
    ) -> str:
        """Texte d'une page, d'une moitié de page ou d'une région, en ordre de lecture.

        Convient au corps de texte. **Jamais à un tableau** : voir ``tableau()``.

        ``tri=False`` rend le texte dans l'ordre du document plutôt que par coordonnée. C'est ce
        qu'il faut sur une page à **deux colonnes** : le tri par ordonnée entrelace les colonnes,
        et un lexique alphabétique en sortirait alterné une entrée sur deux. Le tri, lui, rattrape
        les pages dont l'ordre interne est incohérent — la majorité de ce corpus.
        """
        cle = self._cle_cache('texte' if tri else 'texte-brut', index, moitie, region)
        if (cachee := self._lire_cache(cle, '.txt')) is not None:
            return cachee.decode('utf-8')
        rect = self.rectangle(index, moitie, region)
        brut = self._page(index).get_text('text', clip=rect, sort=tri)
        self._ecrire_cache(cle, '.txt', brut.encode('utf-8'))
        return brut

    # -- Tableaux --------------------------------------------------------------------------

    def tableau(
        self,
        index: int,
        moitie: str | None = None,
        region: tuple | None = None,
        ecart_colonne: float = ECART_COLONNE_DEFAUT,
        bornes: list[float] | None = None,
    ) -> list[list[str]]:
        """Tableau d'une région, par regroupement des mots selon leur coordonnée (`EX-CNT-021`).

        Deux passes, dans cet ordre :

        1. **les lignes**, par ordonnée : deux mots dont les boîtes se chevauchent verticalement
           de plus de la moitié de leur hauteur appartiennent à la même ligne. Le seuil est
           relatif à la hauteur des mots, non absolu, pour tenir sur un tableau dont le corps
           change (titre de section, note de bas de table) ;
        2. **les colonnes**, par profil de projection horizontale sur *toute* la région : les
           intervalles d'abscisse que ne couvre aucun mot, larges d'au moins ``ecart_colonne``,
           séparent les colonnes. Projeter sur la région entière, et non ligne par ligne, est ce
           qui empêche une ligne courte de redéfinir les colonnes pour elle seule — la panne
           exacte du mode en flux.

        Une cellule vide reste vide : la grille est rectangulaire, et une valeur ne glisse jamais
        dans la colonne voisine.

        ``bornes`` **impose** les séparations, en abscisses croissantes, pour le cas que la
        projection ne peut pas trancher : deux colonnes qui se touchent parce que l'entrée la plus
        longue de la première mord sur la seconde. Sur la table des armes des *Basic Rules*,
        « Épée à deux mains » ferme l'écart entre le nom et le dégât, et les deux colonnes
        fusionnent — la valeur reste juste, mais dans la mauvaise case. C'est la seule ambiguïté
        qu'un humain doive lever, et elle se voit à l'œil sur la première ligne.
        """
        mots = self.mots(index, moitie, region)
        if not mots:
            return []
        colonnes = (
            self._colonnes_imposees(bornes) if bornes
            else self._bornes_colonnes(mots, ecart_colonne)
        )
        lignes: list[list[str]] = []
        for mots_ligne in self._grouper_lignes(mots):
            cellules = [[] for _ in colonnes]
            for mot in mots_ligne:
                centre = (mot[0] + mot[2]) / 2
                cellules[self._colonne(centre, colonnes)].append(mot)
            lignes.append([
                ' '.join(m[4] for m in sorted(cellule, key=lambda m: m[0]))
                for cellule in cellules
            ])
        return lignes

    def mots(
        self, index: int, moitie: str | None = None, region: tuple | None = None
    ) -> list[tuple]:
        """Mots d'une région avec leurs coordonnées : ``(x0, y0, x1, y1, texte, ...)``."""
        rect = self.rectangle(index, moitie, region)
        mots = self._page(index).get_text('words', clip=rect)
        return sorted(mots, key=lambda m: (m[1], m[0]))

    @staticmethod
    def _grouper_lignes(mots: list[tuple]) -> list[list[tuple]]:
        """Groupe les mots (triés par ordonnée) en lignes, par chevauchement vertical."""
        lignes: list[list[tuple]] = []
        courante: list[tuple] = []
        bas = 0.0
        for mot in mots:
            hauteur = max(mot[3] - mot[1], 1.0)
            chevauche = courante and mot[1] < bas - hauteur / 2
            if not chevauche and courante:
                lignes.append(sorted(courante, key=lambda m: m[0]))
                courante = []
                bas = 0.0
            courante.append(mot)
            bas = max(bas, mot[3])
        if courante:
            lignes.append(sorted(courante, key=lambda m: m[0]))
        return lignes

    @staticmethod
    def _bornes_colonnes(mots: list[tuple], ecart_colonne: float) -> list[tuple[float, float]]:
        """Intervalles d'abscisse des colonnes, par profil de projection horizontale."""
        intervalles = sorted((m[0], m[2]) for m in mots)
        fusionnes: list[list[float]] = []
        for debut, fin in intervalles:
            if fusionnes and debut - fusionnes[-1][1] < ecart_colonne:
                fusionnes[-1][1] = max(fusionnes[-1][1], fin)
            else:
                fusionnes.append([debut, fin])
        return [(debut, fin) for debut, fin in fusionnes]

    @staticmethod
    def _colonnes_imposees(bornes: list[float]) -> list[tuple[float, float]]:
        if sorted(bornes) != list(bornes):
            raise ExtractionError(f'bornes de colonnes non croissantes : {bornes}.')
        precedente = float('-inf')
        colonnes = []
        for borne in bornes:
            colonnes.append((precedente, borne))
            precedente = borne
        colonnes.append((precedente, float('inf')))
        return colonnes

    @staticmethod
    def _colonne(centre: float, colonnes: list[tuple[float, float]]) -> int:
        for indice, (_, fin) in enumerate(colonnes):
            if centre <= fin:
                return indice
        return len(colonnes) - 1

    # -- Images ----------------------------------------------------------------------------

    def image(
        self,
        index: int,
        region: tuple | None = None,
        moitie: str | None = None,
        ppp: int = PPP_DEFAUT,
    ) -> bytes:
        """PNG d'une région de page, obtenu par **rendu** et jamais par extraction de flux brut.

        Le flux brut de ce corpus se décode mal — zones de bruit vert et cyan (`EX-CNT-022`) ; le
        rendu passe par la composition complète, et est exact. Il embarque en revanche tout ce
        qui est dessiné dans la région, texte compris.
        """
        cle = self._cle_cache(f'image-{ppp}', index, moitie, region)
        if (cachee := self._lire_cache(cle, '.png')) is not None:
            return cachee
        rect = self.rectangle(index, moitie, region)
        pixmap = self._page(index).get_pixmap(clip=rect, dpi=ppp)
        donnees = pixmap.tobytes('png')
        self._ecrire_cache(cle, '.png', donnees)
        return donnees

    def regions_candidates(
        self, index: int, moitie: str | None = None, cote_minimal: float = 64.0
    ) -> list[Region]:
        """Régions à illustration proposées à l'humain — l'étape « l'outil propose ».

        Les emplacements des objets image donnent les **coordonnées** de ce qu'il faut rendre ;
        c'est leur seul usage ici, le contenu venant du rendu. Chaque région est signalée quand
        elle recouvre du texte : celles qui n'en recouvrent pas — panneaux de parchemin, cadres,
        bordures — sortent parfaitement seules et alimentent l'habillage d'interface, les autres
        demandent un recadrage.
        """
        page = self._page(index)
        limite = self.rectangle(index, moitie)
        boites_texte = [
            pymupdf.Rect(mot[:4]) for mot in page.get_text('words', clip=limite)
        ]
        regions: list[Region] = []
        for info in page.get_images(full=True):
            for rect in page.get_image_rects(info[0]):
                rect = rect & limite
                if rect.is_empty or rect.width < cote_minimal or rect.height < cote_minimal:
                    continue
                recouvre = any(rect.intersects(boite) for boite in boites_texte)
                regions.append(Region(rect.x0, rect.y0, rect.x1, rect.y1, recouvre))
        return sorted(regions, key=lambda r: (-r.largeur * r.hauteur, r.y0))

    # -- Statistiques ----------------------------------------------------------------------

    def statistiques(self) -> dict:
        """Quelques chiffres sur le document, pour l'inventaire de la feuille de route."""
        caracteres = 0
        images = 0
        grandes = 0
        for numero in range(self._pdf.page_count):
            page = self._pdf[numero]
            caracteres += len(page.get_text('text'))
            for info in page.get_images(full=True):
                images += 1
                if info[2] >= 512 and info[3] >= 512:
                    grandes += 1
        return {
            'pages': self._pdf.page_count,
            'caracteres': caracteres,
            'caracteres_par_page': round(caracteres / max(self._pdf.page_count, 1)),
            'images': images,
            'images_512': grandes,
        }

    # -- Interne ---------------------------------------------------------------------------

    def _page(self, index: int):
        if not 0 <= index < self._pdf.page_count:
            raise ExtractionError(
                f'{self.document.cle} : page {index} hors du document ({self._pdf.page_count} '
                f'pages, index 0-based).'
            )
        return self._pdf[index]

    def _cle_cache(self, operation: str, index: int, moitie, region) -> str:
        signature = f'{operation}|{index}|{moitie}|{region}'
        return f'{operation}-{index}-{hashlib.sha256(signature.encode()).hexdigest()[:8]}'

    def _lire_cache(self, cle: str, extension: str) -> bytes | None:
        if not self._cache:
            return None
        chemin = self._cache / (cle + extension)
        return chemin.read_bytes() if chemin.is_file() else None

    def _ecrire_cache(self, cle: str, extension: str, donnees: bytes) -> None:
        if self._cache:
            (self._cache / (cle + extension)).write_bytes(donnees)
