#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""Manifeste du corpus : lecture, vérification d'empreinte, correspondance de pagination.

Ce module ne connaît rien du contenu des PDF. Il répond à trois questions, et rien d'autre :

1. *Le document présent est-il bien celui que le manifeste décrit ?* — par empreinte SHA-256 et
   nombre de pages. Une réponse négative **arrête** l'extraction (`EX-CNT-020`). C'est le point
   le plus important du lot : une extraction qui continue sur une autre édition produit des
   données silencieusement décalées, et rien dans la sortie ne le trahit.
2. *Quelle page PDF porte la page imprimée N ?* — les deux livres Tanares sont paginés en
   **double page**, une page PDF en portant deux côte à côte. Un sommaire qui annonce « p. 100 »
   désigne la page PDF 50, et ignorer ce facteur 2 vise systématiquement le mauvais chapitre.
3. *D'où vient cette donnée ?* — la provenance (`srd`, `tanares`, `phb-fr`) que chaque donnée
   produite reportera dans son champ `source` (`EX-CNT-001`).

Aucune dépendance externe : le manifeste est lu par ``tomllib`` (bibliothèque standard depuis
Python 3.11). PyMuPDF n'est requis que par ``extraction``.
"""
from __future__ import annotations

import hashlib
import tomllib
from dataclasses import dataclass
from pathlib import Path

RACINE = Path(__file__).resolve().parent.parent.parent
MANIFESTE = Path(__file__).resolve().parent / 'corpus.toml'
CORPUS_DEFAUT = RACINE / 'Documentation' / 'SourceBook'

# Les valeurs admises pour le champ `source` des données produites (EX-CNT-001). `original` n'est
# pas dans cette liste : aucun PDF ne le porte, il désigne ce que le projet écrit lui-même.
PROVENANCES = ('srd', 'tanares', 'phb-fr')

PAGINATIONS = ('simple', 'double', 'aucune')

VERSION_MANIFESTE = 1


class CorpusError(Exception):
    """Le corpus présent ne correspond pas au manifeste, ou le manifeste est mal formé.

    Toujours fatale, jamais rattrapée : c'est le refus qu'`EX-CNT-020` exige, et le rattraper
    reviendrait à produire les données décalées que l'exigence interdit.
    """


@dataclass(frozen=True)
class Document:
    """Une entrée du manifeste, et le PDF qu'elle décrit."""

    cle: str
    fichier: str
    sha256: str
    pages: int
    pagination: str
    decalage: int
    provenance: str
    langue: str
    ocr: bool
    resume: str
    hors_perimetre: bool
    racine: Path

    @property
    def chemin(self) -> Path:
        return self.racine / self.fichier

    # -- Vérification ---------------------------------------------------------------------

    def empreinte_reelle(self) -> str:
        """SHA-256 du fichier présent, lu par blocs (le plus gros pèse 83 Mo)."""
        digest = hashlib.sha256()
        with self.chemin.open('rb') as flux:
            for bloc in iter(lambda: flux.read(1 << 20), b''):
                digest.update(bloc)
        return digest.hexdigest()

    def verifier(self) -> None:
        """Lève ``CorpusError`` si le document présent n'est pas celui du manifeste."""
        if not self.chemin.is_file():
            raise CorpusError(
                f"{self.cle} : document absent — {self.chemin}. Le corpus n'est pas versionné "
                f"(EX-CNT-023) : déposer les PDF dans {self.racine}, ou passer --corpus-root."
            )
        reelle = self.empreinte_reelle()
        if reelle != self.sha256:
            raise CorpusError(
                f"{self.cle} : empreinte différente de celle du manifeste.\n"
                f"  attendue : {self.sha256}\n"
                f"  présente : {reelle}\n"
                f"  Ce n'est pas le document sur lequel la pagination et les repères de page ont "
                f"été relevés. Poursuivre produirait des données décalées sans le dire "
                f"(EX-CNT-020). Rétablir le document d'origine, ou relever les repères sur "
                f"celui-ci puis mettre le manifeste à jour."
            )

    # -- Pagination -----------------------------------------------------------------------

    def index_pdf(self, page_imprimee: int) -> int:
        """Index de page PDF (0-indexé) portant la page **imprimée** donnée.

        En pagination double, deux pages imprimées partagent le même index : c'est voulu, et
        c'est ce que l'appelant doit savoir avant de découper une région.
        """
        if self.pagination == 'aucune':
            raise CorpusError(f'{self.cle} : document sans pagination imprimée.')
        if self.pagination == 'double':
            index = (page_imprimee - self.decalage) // 2
        else:
            index = page_imprimee - self.decalage
        if not 0 <= index < self.pages:
            raise CorpusError(
                f'{self.cle} : page imprimée {page_imprimee} hors du document '
                f'(index PDF {index}, {self.pages} pages).'
            )
        return index

    def pages_imprimees(self, index_pdf: int) -> tuple[int, ...]:
        """Page(s) imprimée(s) portée(s) par un index de page PDF. Vide si non paginé."""
        if self.pagination == 'aucune':
            return ()
        if self.pagination == 'double':
            gauche = 2 * index_pdf + self.decalage
            return (gauche, gauche + 1)
        return (index_pdf + self.decalage,)

    @property
    def double_page(self) -> bool:
        return self.pagination == 'double'


class Corpus:
    """Le manifeste chargé, et l'accès aux documents qu'il décrit."""

    def __init__(self, documents: dict[str, Document], manifeste: Path) -> None:
        self.documents = documents
        self.manifeste = manifeste

    @classmethod
    def charger(cls, racine_corpus: Path | None = None, manifeste: Path | None = None) -> 'Corpus':
        chemin = manifeste or MANIFESTE
        racine = racine_corpus or CORPUS_DEFAUT
        try:
            donnees = tomllib.loads(chemin.read_text(encoding='utf-8'))
        except FileNotFoundError as erreur:
            raise CorpusError(f'manifeste introuvable : {chemin}') from erreur
        except tomllib.TOMLDecodeError as erreur:
            raise CorpusError(f'{chemin} : TOML mal formé — {erreur}') from erreur

        version = donnees.get('version')
        if version != VERSION_MANIFESTE:
            raise CorpusError(
                f'{chemin} : version de manifeste {version!r}, attendue {VERSION_MANIFESTE}.'
            )

        documents: dict[str, Document] = {}
        for cle, entree in donnees.get('documents', {}).items():
            documents[cle] = cls._document(cle, entree, racine, chemin)
        if not documents:
            raise CorpusError(f'{chemin} : aucun document déclaré.')
        return cls(documents, chemin)

    @staticmethod
    def _document(cle: str, entree: dict, racine: Path, manifeste: Path) -> Document:
        def requis(champ: str):
            if champ not in entree:
                raise CorpusError(f'{manifeste} : {cle} — champ « {champ} » manquant.')
            return entree[champ]

        pagination = requis('pagination')
        if pagination not in PAGINATIONS:
            raise CorpusError(
                f'{manifeste} : {cle} — pagination {pagination!r} inconnue '
                f'(admises : {", ".join(PAGINATIONS)}).'
            )
        provenance = requis('provenance')
        if provenance not in PROVENANCES:
            raise CorpusError(
                f'{manifeste} : {cle} — provenance {provenance!r} inconnue '
                f'(admises : {", ".join(PROVENANCES)}). Elle est reportée telle quelle dans le '
                f'champ `source` des données produites (EX-CNT-001).'
            )
        return Document(
            cle=cle,
            fichier=requis('fichier'),
            sha256=requis('sha256'),
            pages=int(requis('pages')),
            pagination=pagination,
            decalage=int(requis('decalage')),
            provenance=provenance,
            langue=requis('langue'),
            ocr=bool(entree.get('ocr', False)),
            resume=entree.get('resume', ''),
            hors_perimetre=bool(entree.get('hors_perimetre', False)),
            racine=racine,
        )

    def __getitem__(self, cle: str) -> Document:
        try:
            return self.documents[cle]
        except KeyError:
            connus = ', '.join(sorted(self.documents))
            raise CorpusError(
                f'document {cle!r} inconnu du manifeste (connus : {connus}).'
            ) from None

    def __iter__(self):
        return iter(self.documents.values())

    def __len__(self) -> int:
        return len(self.documents)
