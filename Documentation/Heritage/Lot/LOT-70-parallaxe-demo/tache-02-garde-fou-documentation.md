# TACHE-02 — Garde-fou de non-régression, documentation et référentiel {#lot-70-tache-02-garde-fou-documentation}

**Lot :** [LOT-70](epic.md) · **Emplacement :** `Source/Test/Systeme`, `Documentation` ·
**Statut :** fait

## Contexte
Rien, aujourd'hui, ne distingue « un tableau à parallaxe active porte trois profondeurs bien
réglées » de « il en porte deux, ou trois mal ordonnées » : `test_plans_livres.cpp` (`LOT-69`
TACHE-10) vérifie seulement que chaque plan référencé existe aux bonnes dimensions, jamais combien
il y en a ni comment leurs facteurs se comparent. Sans garde-fou, un niveau réédité dans l'éditeur
pourrait perdre son plan lointain, ou un facteur mal tapé pourrait inverser l'ordre des profondeurs,
sans qu'aucun test ne le remarque — exactement le defaut que ce lot corrige à l'écran, réintroduit
en silence.

## Travail à réaliser
- Étendre `test_plans_livres.cpp` (même fichier que les contrôles de plans existants, pas un
  nouveau : le lecteur qui cherche « que vérifie-t-on sur les plans livrés » y trouve tout) d'un
  test système : pour `demo-mouvement.json` et `demo-final.json`, au moins **trois** plans, dont les
  `parallaxX` sont **strictement croissants** dans l'ordre de la liste.
- Cas négatif à figer dans le même test (ou un test dédié voisin) : retirer le plan lointain de la
  liste **en mémoire** (pas dans le fichier livré) fait échouer l'assertion — la preuve que le
  garde-fou refuse réellement quelque chose, pas seulement qu'il passe sur l'état actuel.
- `Documentation/Guide/guide-niveaux.md` (ou `guide-rendu.md`, selon où vit déjà la description des
  plans/parallaxe du `LOT-69`) : un paragraphe court sur la convention à trois profondeurs
  (lointain/fond/devant) et le fait qu'elle ne s'applique qu'aux niveaux à parallaxe active.
- `CHANGELOG.md`, section `[Non publié]` : nouvelle entrée pour ce lot.
- `Documentation/Lot/lots.md` : ajouter `@subpage lot-70` à l'index et une entrée dans la section
  « Après le programme 0.1.0 », sur le modèle de celles de `LOT-67`/`LOT-69`.
- Régénérer `Documentation/CahierTest.md` (`python scripts/generate_cahier_test.py`).

## Fichiers impactés
`Source/Test/Systeme/test_plans_livres.cpp`, `Documentation/Guide/guide-rendu.md`,
`Documentation/Lot/lots.md`, `CHANGELOG.md`, `Documentation/CahierTest.md`.

## Tests (obligatoires)
- Le nouveau test échoue si un tableau à parallaxe active porte moins de trois plans, ou des
  facteurs non strictement croissants.
- `scripts/lint_exigences.py` : vert (aucune exigence nouvelle à déclarer, ce lot n'en consomme que
  d'existantes — voir l'epic).
- `scripts/generate_cahier_test.py --check` : vert.
- `scripts/check_demo_sequence.py` : vert.

## Points d'attention
Ce lot ne déclare **aucune** exigence nouvelle (voir « Exigences couvertes » de l'epic) : le lint
d'exigences n'a donc rien à valider de neuf ici, seulement à rester vert malgré les fichiers
modifiés.

## Definition de fait (DoD)
Garde-fou vert au sens positif et négatif, les quatre linters passent, le cahier de test est
régénéré, les statuts de `lots.md`/`epic.md` sont figés à « fait ». `ctest` à 100 %.

## Exigences
`EX-DEC-043`, `EX-DEC-040`.
