# TACHE-04 — Documentation et vérification {#lot-25-tache-04-documentation-verification}

**Lot :** [LOT-25](epic.md) · **Emplacement :** `Documentation` · **Statut :** fait

## Contexte
Dernière tâche du lot : aligner la documentation sur la séquence de niveaux finale.

## Travail à réaliser
- **`Documentation/Manuel/jouer.md`** : si la progression ou les mécaniques mises en avant par la
  nouvelle séquence changent ce qu'un joueur découvre en jouant, ajuster la description.
- **`Documentation/Guide/guide-niveaux.md`** ou **`guide-ecrans.md`** : mentionner, le cas échéant,
  la convention de nommage des fichiers de niveaux adoptée en `TACHE-02`.
- **`Documentation/Lot/lots.md`** : ajoute `@subpage lot-25`.
- **`CHANGELOG.md`** : entrée `[Non publié]` décrivant LOT-25 (liste des niveaux ajoutés/modifiés,
  correction de la divergence `demo5.json`/test système).
- **Doxygen**, **Cahier de test**, **lint des exigences** : mêmes vérifications que les lots
  précédents.

## Fichiers impactés
- `Documentation/Manuel/jouer.md` (si pertinent).
- `Documentation/Guide/guide-niveaux.md` ou `guide-ecrans.md` (si pertinent).
- `Documentation/Lot/lots.md`.
- `Documentation/CahierTest.md` (régénéré).
- `CHANGELOG.md`.

## Tests (obligatoires)
- `python scripts/lint_exigences.py` retourne un code de sortie `0`.
- `python scripts/generate_cahier_test.py --check` confirme que le Cahier de test est à jour.
- `doxygen Doxyfile` (depuis `Documentation/`) se termine avec un code de sortie `0` et **aucune**
  ligne de sortie.
- **Vérification visuelle** de la séquence complète dans l'application compilée (critère
  d'acceptation 5 de l'épic) : **substituée** par le test système
  `ParcoursCompletSysteme.FranchitTouteLaSequence`, qui rejoue les 13 niveaux dans l'ordre avec des
  scripts d'entrées déterministes exploitant réellement chaque mécanique visée (pas un simple
  franchissement par chance) — une session interactive dans l'application compilée est hors
  périmètre de ce qu'un agent doit faire de façon autonome dans cette conversation (cohérent avec
  la substitution déjà retenue pour `LOT-24`, `Documentation/Lot/LOT-24-blocs-taille-fractionnaire/
  tache-03-editeur-rendu.md`).

## Points d'attention
- Cette tâche clôt le lot : vérifier que les 6 critères d'acceptation d'`epic.md` sont
  effectivement remplis avant de le marquer terminé.

## Définition de fait (DoD)
- Documentation cohérente avec la séquence de niveaux livrée ; `epic.md` marqué **terminé**, chaque
  tâche marquée ✅.

## Exigences
Aucune exigence propre — tâche de cohérence documentaire pour l'ensemble du lot.
