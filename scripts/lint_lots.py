#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""Lint du graphe de lots de JustAnotherDnDGame.

La feuille de route ``Documentation/Lot/roadmap-0.1.0.md`` est l'**unique source de vérité** des
lots à venir. Un document de cette taille dérive : les comptes vieillissent, un lien de dépendance
s'écrit d'un seul côté, un lot disparaît du tableau d'ordre, deux lots revendiquent la même
exigence, le diagramme trace une flèche que plus rien ne justifie. L'audit qui a produit la version
courante de cette page y a trouvé six incohérences de cette forme — toutes mécaniquement
détectables, aucune détectée.

Ce lint les refuse en CI. Il vérifie :

1. chaque lot de la filière porte une ligne ``*Prérequis*`` et une ligne d'acceptation ;
2. chaque lot de la filière porte une **ancre Doxygen** ``{#lot-NN}``, unique ;
3. le graphe de prérequis est **acyclique** ;
4. tout prérequis désigne un lot **existant** ;
5. les liens sont **symétriques** : si A déclare « Alimente B », alors B déclare A en prérequis ;
6. chaque lot de la filière apparaît dans le **tableau d'ordre** de la section 6 ;
7. les **numéros manquants** de la plage sont soit retirés par fusion et recensés comme tels, soit
   livrés et pourvus de leur dossier ; un numéro retiré n'est plus cité comme prérequis ;
8. les **comptes annoncés en toutes lettres** correspondent au décompte réel ;
9. aucune **exigence** n'est revendiquée en retrait par deux lots à la fois ;
10. le **tableau récapitulatif** de la section 6 correspond au graphe déclaré ;
11. toute arête du **diagramme** de la section 6 correspond à un lien déclaré.

Les lots livrés (``LOT-01`` à ``LOT-07``) et absorbés (``LOT-08`` à ``LOT-29``) sont exclus des
contrôles 1, 2 et 5 : leur texte est repris tel quel de leurs epics d'origine, et les sections 5, 9
et 10 de la feuille de route font foi sur eux — c'est écrit en tête de la section 11.

Sortie : liste des violations, code de retour 1 si au moins une. Aucune dépendance externe.
"""
from __future__ import annotations

import re
import sys
from pathlib import Path

# La console Windows est en cp1252 : les messages contiennent des flèches et des guillemets.
if hasattr(sys.stdout, 'reconfigure'):
    sys.stdout.reconfigure(encoding='utf-8', errors='replace')

RACINE = Path(__file__).resolve().parent.parent
DOSSIER_LOTS = RACINE / 'Documentation' / 'Lot'
ROADMAP = DOSSIER_LOTS / 'roadmap-0.1.0.md'

PREMIER_LOT_FILIERE = 30

# Sections de lot : "### `LOT-30` — Titre {#lot-30}" ou la plage "### `LOT-51` à `LOT-65` — …"
SECTION_RE = re.compile(r'^### `LOT-(\d+)`(?: à `LOT-(\d+)`)? — (.+)$', re.M)
# Sections des lots absorbés (LOT-08 à LOT-29) : "### LOT-08 — Titre {#lot-08}"
SECTION_ABSORBEE_RE = re.compile(r'^### LOT-(\d+) — (.+)$', re.M)
ANCRE_RE = re.compile(r'\{#lot-(\d+)\}')
LOT_RE = re.compile(r'LOT-(\d+)')
TITRE_RE = re.compile(r'— (.+?)(?: \{#lot-\d+\})?$')

# Ce qui, dans la ligne « Prérequis », introduit le sens inverse de la dépendance.
# « Prérequis de chacun » est une exception : elle déclare bien de l'amont.
SENS_INVERSE_RE = re.compile(r'Alimente|Prérequis de(?! chacun)|Débloque|Contrôlé par|Réoriente')

# Comptes écrits en toutes lettres que le lint sait vérifier.
NOMBRES_FR = {
    1: 'un', 2: 'deux', 3: 'trois', 4: 'quatre', 5: 'cinq', 6: 'six', 7: 'sept', 8: 'huit',
    9: 'neuf', 10: 'dix', 11: 'onze', 12: 'douze', 13: 'treize', 14: 'quatorze', 15: 'quinze',
    16: 'seize', 20: 'vingt', 30: 'trente', 40: 'quarante', 50: 'cinquante',
    51: 'cinquante et un', 52: 'cinquante-deux', 53: 'cinquante-trois',
    54: 'cinquante-quatre', 55: 'cinquante-cinq',
}


class Rapport:
    def __init__(self) -> None:
        self.violations: list[str] = []

    def erreur(self, message: str) -> None:
        self.violations.append(message)

    def bilan(self) -> int:
        if not self.violations:
            print('lint_lots : OK')
            return 0
        print('lint_lots : %d violation(s)\n' % len(self.violations))
        for v in self.violations:
            print('  - ' + v)
        return 1


def numero(n) -> str:
    return 'LOT-%s' % n


def lots_livres():
    """Les lots qui ont leur dossier : livrés, ils ont quitté la feuille de route."""
    livres = set()
    for chemin in DOSSIER_LOTS.glob('LOT-*'):
        if chemin.is_dir():
            m = re.match(r'LOT-(\d+)', chemin.name)
            if m:
                livres.add('LOT-' + m.group(1))
    return livres


def lire_sections(texte: str) -> dict:
    """{lot: {titre, ancre, corps}} ; une plage « LOT-51 à LOT-65 » couvre ses intermédiaires."""
    sections: dict = {}
    bornes = []
    for m in SECTION_RE.finditer(texte):
        bornes.append((m.start(), m.group(0), m.group(1), m.group(2)))
    for m in SECTION_ABSORBEE_RE.finditer(texte):
        bornes.append((m.start(), m.group(0), m.group(1), None))
    bornes.sort()

    for i, (debut, entete, premier, dernier) in enumerate(bornes):
        fin = bornes[i + 1][0] if i + 1 < len(bornes) else len(texte)
        ancres = ANCRE_RE.findall(entete)
        lot = numero(premier)
        sections[lot] = {
            'titre': entete,
            'ancre': ancres[0] if ancres else None,
            'corps': texte[debut:fin],
        }
        if dernier:
            for n in range(int(premier) + 1, int(dernier) + 1):
                sections[numero(n)] = dict(sections[lot], couvert_par=lot)
    return sections


def bloc_prerequis(corps: str):
    """Le paragraphe « Prérequis », qui peut courir sur plusieurs lignes."""
    m = re.search(r'^\*Prérequis[^\n]*(?:\n(?!\n)[^\n]*)*', corps, re.M)
    return m.group(0) if m else None


def prerequis_de(corps: str):
    """(prérequis déclarés, lots que ce lot déclare alimenter)."""
    bloc = bloc_prerequis(corps)
    if not bloc:
        return [], []
    coupe = SENS_INVERSE_RE.search(bloc)
    amont = bloc[:coupe.start()] if coupe else bloc
    aval = bloc[coupe.start():] if coupe else ''
    return ([numero(n) for n in LOT_RE.findall(amont)],
            [numero(n) for n in LOT_RE.findall(aval)])


def graphe(texte: str):
    """(prérequis, alimente déclaré, alimente effectif, sections)."""
    sections = lire_sections(texte)
    amont: dict = {}
    aval_declare: dict = {}
    for lot, s in sections.items():
        if s.get('couvert_par'):
            continue
        a, b = prerequis_de(s['corps'])
        amont[lot] = [x for x in a if x != lot]
        aval_declare[lot] = [x for x in b if x != lot]

    aval: dict = {lot: set() for lot in amont}
    for lot, deps in amont.items():
        for d in deps:
            aval.setdefault(d, set()).add(lot)
    for lot, cibles in aval_declare.items():
        aval.setdefault(lot, set()).update(cibles)
    return amont, aval_declare, aval, sections


def tableau_recapitulatif(texte: str) -> str:
    """Le tableau de la section 6, dérivé du graphe : une ligne par lot de la filière."""
    amont, _, aval, sections = graphe(texte)
    filiere = sorted(
        (l for l in sections
         if int(l[4:]) >= PREMIER_LOT_FILIERE and not sections[l].get('couvert_par')),
        key=lambda x: int(x[4:]))

    def titre(lot: str) -> str:
        m = TITRE_RE.search(sections[lot]['titre'])
        return m.group(1) if m else ''

    def liste(valeurs) -> str:
        ordonnees = sorted(valeurs, key=lambda x: int(x[4:]))
        return ', '.join('`%s`' % v for v in ordonnees) if ordonnees else '—'

    return '\n'.join(
        '| `%s` | %s | %s | %s |' % (lot, titre(lot), liste(amont.get(lot, [])),
                                     liste(aval.get(lot, set())))
        for lot in filiere)


def main() -> int:
    r = Rapport()
    texte = ROADMAP.read_text(encoding='utf-8')
    amont, aval_declare, aval, sections = graphe(texte)

    filiere = set(l for l in sections if int(l[4:]) >= PREMIER_LOT_FILIERE)
    livres = lots_livres()
    tous = set(sections) | livres

    # ---- 1, 2 : rubriques obligatoires et ancres ----
    ancres_vues: dict = {}
    for lot in sorted(filiere):
        s = sections[lot]
        if s.get('couvert_par'):
            continue  # couvert par la plage « LOT-51 à LOT-65 », qui porte les rubriques
        if not bloc_prerequis(s['corps']):
            r.erreur('%s : aucune ligne « Prérequis »' % lot)
        if 'cceptation' not in s['corps']:
            r.erreur('%s : aucune ligne « Acceptation »' % lot)
        if not s['ancre']:
            r.erreur('%s : aucune ancre Doxygen {#lot-NN} dans le titre' % lot)
        else:
            if s['ancre'] in ancres_vues:
                r.erreur('ancre {#lot-%s} en double : %s et %s'
                         % (s['ancre'], ancres_vues[s['ancre']], lot))
            ancres_vues[s['ancre']] = lot

    # ---- 4 : prérequis vers un lot existant ----
    for lot, deps in sorted(amont.items()):
        for d in deps:
            if d not in tous:
                r.erreur("%s déclare le prérequis %s, qui n'existe pas" % (lot, d))

    # ---- 3 : acyclicité ----
    etat: dict = {}

    def visiter(n: str, chemin: list) -> None:
        if etat.get(n) == 1:
            r.erreur('cycle de prérequis : %s' % ' → '.join(chemin + [n]))
            return
        if etat.get(n) == 2:
            return
        etat[n] = 1
        for d in amont.get(n, []):
            visiter(d, chemin + [n])
        etat[n] = 2

    for n in sorted(amont):
        visiter(n, [])

    # ---- 5 : symétrie des liens, entre lots de la filière ----
    for lot in sorted(filiere):
        for cible in aval_declare.get(lot, []):
            if cible not in filiere:
                continue
            # Un lot couvert par une plage (« LOT-51 à LOT-65 ») n'a pas de ligne propre :
            # ses prérequis sont ceux de la section qui le couvre.
            porteur = sections[cible].get('couvert_par', cible)
            if lot not in amont.get(porteur, []):
                r.erreur('%s déclare alimenter %s, mais %s ne le cite pas en prérequis'
                         % (lot, cible, porteur))

    # ---- 6 : présence au tableau d'ordre de la section 6 ----
    au_tableau = set()
    bloc6 = texte.split('## 6. Ordre')[1].split('Quatre lignes méritent')[0]
    for ligne in bloc6.split('\n'):
        if ligne.startswith('|') and 'Pourquoi' not in ligne:
            cellules = ligne.split('|')
            if len(cellules) > 3:
                nums = [int(n) for n in LOT_RE.findall(cellules[2])]
                if '→' in cellules[2] and len(nums) == 2:
                    nums = list(range(nums[0], nums[1] + 1))
                au_tableau |= set(numero(n) for n in nums)
    for lot in sorted(filiere):
        if lot not in au_tableau:
            r.erreur("%s n'apparaît dans aucune ligne du tableau d'ordre (§6)" % lot)

    # ---- 7 : numéros retirés ----
    presents = set(int(l[4:]) for l in filiere)
    manquants = set(range(min(presents), max(presents) + 1)) - presents
    # Un numéro manquant est légitime de deux façons : il est retiré par fusion, ou il est livré
    # et a donc quitté cette page pour son dossier.
    numeros_livres = set(int(l[4:]) for l in livres)
    retires_reels = manquants - numeros_livres
    bloc_retires = texte.split('numéros retirés.**')[1].split('---')[0]
    retires_declares = set(int(n) for n in LOT_RE.findall(bloc_retires))
    if retires_reels != retires_declares:
        r.erreur('numéros retirés : le tableau déclare %s, la plage en manque %s '
                 '(hors lots livrés %s)'
                 % (sorted(retires_declares), sorted(retires_reels),
                    sorted(numeros_livres & manquants)))
    for lot, deps in sorted(amont.items()):
        for d in deps:
            if int(d[4:]) in retires_reels:
                r.erreur('%s cite %s, un numéro retiré par fusion' % (lot, d))

    # ---- 8 : comptes annoncés ----
    mot = NOMBRES_FR.get(len(filiere))
    if mot:
        attendu = '%s lots, `LOT-%d` à `LOT-%d`' % (mot.capitalize(), min(presents), max(presents))
        if attendu not in texte:
            r.erreur('le compte annoncé en §5 ne correspond pas : attendu « %s »' % attendu)
    mot_retires = NOMBRES_FR.get(len(retires_reels))
    if mot_retires and ('**%s numéros retirés.**' % mot_retires.capitalize()) not in texte:
        r.erreur('le nombre de numéros retirés annoncé ne correspond pas : %d attendu'
                 % len(retires_reels))

    # ---- 9 : une exigence retirée par un seul lot ----
    proprietaire: dict = {}
    for lot in sorted(filiere):
        for phrase in re.findall(r'[^.]*\*\*retire\*\*[^.]*\.', sections[lot]['corps']):
            if 'ne touche pas' in phrase or 'ne se les approprie' in phrase:
                continue
            for ex in re.findall(r'EX-[A-Z]+-\d+', phrase):
                if ex in proprietaire and proprietaire[ex] != lot:
                    r.erreur('%s est déclarée retirée par %s et par %s'
                             % (ex, proprietaire[ex], lot))
                proprietaire[ex] = lot

    # ---- 10 : tableau récapitulatif à jour ----
    attendu_tab = tableau_recapitulatif(texte)
    if attendu_tab not in texte:
        manquantes = [l for l in attendu_tab.split('\n') if l not in texte]
        r.erreur("le tableau récapitulatif (§6) ne correspond plus au graphe déclaré ; "
                 "%d ligne(s) à corriger, à commencer par : %s"
                 % (len(manquantes), manquantes[0] if manquantes else '(ordre des lignes)'))

    # ---- 11 : le diagramme ne trace que des liens déclarés ----
    m_dot = re.search(r'```dot\n(.*?)\n```', texte, re.S)
    if m_dot:
        corps_dot = m_dot.group(1)
        etiquettes = dict(re.findall(r'(L\d+)\s*\[label="(LOT-\d+)', corps_dot))
        for a, b in re.findall(r'(L\d+)\s*->\s*(L\d+)', corps_dot):
            src, dst = etiquettes.get(a), etiquettes.get(b)
            if not src or not dst:
                continue
            if src not in amont.get(dst, []) and dst not in aval.get(src, set()):
                r.erreur('le diagramme (§6) trace %s → %s, que rien ne déclare' % (src, dst))

    print('lots de la filière : %d (LOT-%d à LOT-%d ; %d retiré(s), %d livré(s) hors page)'
          % (len(filiere), min(presents), max(presents), len(retires_reels),
             len(numeros_livres & manquants)))
    return r.bilan()


if __name__ == '__main__':
    sys.exit(main())
