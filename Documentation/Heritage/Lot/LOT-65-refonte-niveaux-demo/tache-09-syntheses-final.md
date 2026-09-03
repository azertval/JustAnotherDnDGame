# TACHE-09 — Tableaux de synthèse et final multi-salles {#lot-65-tache-09-syntheses-final}

**Lot :** [LOT-65](epic.md) · **Emplacement :** `Source/Elements/Levels` · **Statut :** non commencé

## Contexte
C'est le critère d'acceptation n°4 du lot — « puis les combine dans des tableaux de synthèse » — et
c'est celui qui n'a pas été tenu. Le `demo-final` livré mesure 62×10 pour **trois** tuiles non
solides (un interrupteur, une porte, une pente) ; ses segments recopient quatre tableaux existants
séparés par une trentaine de cases de couloir plat ; l'interrupteur est posé sur le trajet direct
vers la porte, de sorte qu'aucune mécanique n'en croise une autre et qu'il n'y a **aucune énigme**.

Il n'est même pas dernier : `demo-salles` — 272 tuiles, **zéro mécanique**, dont environ 40 % de la
surface est scellée sous un sol plein et inatteignable — est joué après lui. La séquence se termine
donc sur une démonstration structurelle où l'on marche puis l'on tombe.

`demo-salles` est **absorbé** par le final : son rôle (couvrir le cadrage par salle) est repris par
un tableau qui, lui, a un contenu.

## Travail à réaliser

### 21 — `demo-synthese` (nouveau)
Marche d'approche du final : mécanismes, terrain et dangers **entrelacés** plutôt que juxtaposés,
sans énigme longue. Porte le croisement « deux déclencheurs — interrupteur *et* plaque — sur la même
porte » établi par la `TACHE-06`. Budgets serrés.

### 22 — `demo-final` (reconstruit, dernier de la séquence)
- **Multi-salles**, cadrage `perRoom`, absorbant `demo-salles.json` (supprimé).
- Emploie les deux variantes de cadrage livrées par le `LOT-64` que **zéro tableau** n'utilise
  aujourd'hui : les `zones` de caméra dessinées à la main (`EX-LVL-007`) et une taille de salle
  explicite (`roomWidthTiles`/`roomHeightTiles`, `EX-REN-017`). Le final multi-salles est l'endroit
  naturel pour mélanger plusieurs tailles de caméra dans un même niveau.
- Chaque salle porte une **énigme composée**, pas un segment de couloir : le bloc sur plaque
  débloqué par la `TACHE-06` en est la colonne vertébrale (poser un poids, repartir, franchir).
- La porte finale s'ouvre par une **clé** placée derrière un parcours de dangers cadencés.
- Budgets serrés, choisis pour interdire le contournement au double saut ou au dash.
- **Aucune plateforme mobile dans ce fichier** tant que le défaut plateforme + pente n'est pas
  corrigé ; la plateforme reste couverte isolément par `demo-plateforme`.

### Séquence
`sequence-demo.json` mis à jour : `demo-arrondi.json` (fusionné en `TACHE-08`) et `demo-salles.json`
(absorbé ici) retirés, `demo-mouvement.json` (`TACHE-07`) et `demo-synthese.json` ajoutés,
`demo-final.json` **en dernier**. Total inchangé : vingt-deux tableaux.

## Fichiers impactés
- `Source/Elements/Levels/demo-synthese.json` (nouveau), `demo-final.json` (reconstruit).
- `Source/Elements/Levels/demo-salles.json` — **supprimé**.
- `Source/Elements/Levels/sequence-demo.json`.
- `Source/Test/Systeme/test_parcours_complet.cpp`.

## Tests (obligatoires)
- Les deux tableaux sont franchis par un script déterministe ; aucun ne l'est en maintenant
  « droite ».
- Le garde-fou de couverture est vert **y compris** sur les variantes de cadrage : `zones` et taille
  de salle explicite apparaissent enfin.
- Le mode `perRoom` reste couvert après la disparition de `demo-salles.json`.
- `scripts/check_demo_sequence.py` vert.

## Points d'attention
- **Une énigme n'est pas un détour.** Un interrupteur posé sur le trajet vers sa porte n'est pas une
  énigme, c'est un ornement : c'est l'erreur exacte du `demo-final` livré, à ne pas reproduire.
- **Un tableau final long est un tableau final fragile.** Chaque salle doit être franchissable
  indépendamment de la précédente une fois atteinte, pour qu'un échec ne renvoie pas à dix minutes
  de rejeu — `EX-GP-032` recharge le tableau entier.
- **Aucune impasse** (`niveaux.md` § Conception) : un bloc poussé au mauvais endroit ne doit jamais
  rendre la sortie inatteignable sans échec possible. C'est le risque principal du bloc sur plaque,
  et il se vérifie par test, pas par relecture.
- Les zones de caméra sont une donnée du niveau : les dessiner dans l'éditeur puis relire le
  fichier produit, plutôt que d'écrire les rectangles à la main.

## Définition de fait (DoD)
- `demo-synthese` et `demo-final` sont livrés et franchissables ; `demo-salles.json` et
  `demo-arrondi.json` ont disparu ; la séquence compte vingt-deux tableaux, `demo-final` en dernier ;
  les `zones` de caméra et une taille de salle explicite sont employées ; tous les garde-fous verts.

## Exigences
`EX-LVL-012` (synthèses), `EX-LVL-015` (couverture), `EX-LVL-006` et `EX-LVL-007` (cadrage, zones),
`EX-REN-017` (taille de suivi), `EX-GP-023` (clé), `EX-GP-025` (plaque et poids), `EX-GP-024`
(budgets), `EX-GP-050` à `EX-GP-053` (dangers), `EX-VIS-005` (niveaux enchaînés).
