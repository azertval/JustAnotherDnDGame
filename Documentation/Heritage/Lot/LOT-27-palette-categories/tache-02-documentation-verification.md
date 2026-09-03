# TACHE-02 — Documentation et vérification {#lot-27-tache-02-documentation-verification}

**Lot :** [LOT-27](epic.md) · **Emplacement :** `Documentation` · **Statut :** fait

## Contexte
Clôture le lot : nouvelle exigence tracée dans la spécification, index des lots à jour,
`CHANGELOG.md` renseigné, build et suite de tests complets vérifiés verts.

## Travail à réaliser
- **`Documentation/Specification/editeur-niveaux.md`** : nouvelle section 8 (« Palette organisée
  par catégories »), `EX-EDIT-018`, statut d'en-tête et traçabilité mis à jour.
- **`Documentation/Lot/lots.md`** : ajout de `@subpage lot-27`.
- **`CHANGELOG.md`** : entrée `### Ajouté` décrivant la réorganisation de la palette.

## Fichiers impactés
- `Documentation/Specification/editeur-niveaux.md`, `Documentation/Lot/lots.md`,
  `Documentation/Lot/LOT-27-palette-categories/*.md`, `CHANGELOG.md`.

## Tests (obligatoires)
- `python scripts/lint_exigences.py` : `EX-EDIT-018` déclarée une seule fois, aucune référence
  orpheline.
- Build `/W4 /WX` sans avertissement (déjà vérifié TACHE-01) ; suite complète verte.

## Définition de fait (DoD)
- Spécification, index des lots et `CHANGELOG.md` à jour ; lint des exigences vert.

## Exigences
`EX-EDIT-018` (documentation et vérification).
