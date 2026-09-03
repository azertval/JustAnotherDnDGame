# TACHE-08 — Documentation, exigences, CHANGELOG {#lot-72-tache-08-documentation}

**Lot :** [LOT-72](epic.md) · **Emplacement :** `Documentation/Specification`, `CHANGELOG.md` ·
**Statut :** fait

## Contexte
Comme tout lot, LOT-72 doit laisser une trace traçable : exigences dans les spécifications, entrée
CHANGELOG, et référencement dans `Documentation/Lot/lots.md`. Cette tâche est volontairement
distincte des six tâches de mécanique (TACHE-01 à TACHE-06) pour que la documentation avance même si
l'implémentation est encore partielle.

## Travail à réaliser
- `Documentation/Specification/gameplay.md` : ajouter `EX-GP-056` à `EX-GP-061` (fait — voir le
  cadrage de l'épic).
- `Documentation/Lot/lots.md` : ajouter `@subpage lot-72` à la liste des lots, et un paragraphe dans
  la section « Après le programme `0.1.0` » expliquant le déclencheur du lot (enrichir le nuancier de
  mouvement en composant le dash avec les autres systèmes) et son statut.
- `CHANGELOG.md` : entrée sous `[Non publié]` décrivant les six mécaniques (fait — voir le cadrage de
  l'épic) ; à mettre à jour/déplacer vers une entrée de version une fois le lot livré.
- Une fois TACHE-01 à TACHE-07 terminées : repasser sur `epic.md` (statut « fait », cases du
  découpage), et sur cette entrée CHANGELOG pour refléter la livraison réelle plutôt que le seul
  cadrage.

## Fichiers impactés
- `Documentation/Specification/gameplay.md`.
- `Documentation/Lot/lots.md`.
- `Documentation/Lot/LOT-72-mouvement-avance/epic.md` (mise à jour de statut en fin de lot).
- `CHANGELOG.md`.

## Tests (obligatoires)
- **Lint d'exigences** (script existant du dépôt) : `EX-GP-056` à `EX-GP-061` correctement
  formées et référencées.
- Vérification manuelle que chaque exigence nouvelle référence bien le fichier de tâche qui la
  concrétise, une fois l'implémentation terminée.

## Points d'attention
- Ne pas marquer l'épic « fait » avant que TACHE-01 à TACHE-07 le soient réellement (cases ✅ du
  découpage) : le cadrage documentaire ne vaut pas livraison.
- Suivre le format exact des exigences déjà en place (`\anchor` suivi de l'identifiant en gras, puis
  le texte, puis « Concrétisé en `LOT-72`. »), sans en dévier — voir `EX-GP-056` pour un exemple.

## Définition de fait (DoD)
- Documentation, exigences et CHANGELOG alignés sur l'état réel du lot ; lint d'exigences vert.

## Exigences
`EX-GP-056`, `EX-GP-057`, `EX-GP-058`, `EX-GP-060`, `EX-GP-061`.
