# TACHE-08 — Nettoyage documentaire {#lot-15-tache-08-nettoyage-documentation}

**Lot :** [LOT-15](epic.md) · **Emplacement :** `Documentation`, `Source/Core/Level`, `Source/HMI/Editor` · **Statut :** à faire

## Contexte
Dernière tâche du lot : refermer les écarts entre documentation et implémentation accumulés depuis
LOT-14, une fois le code des tâches précédentes livré.

## Travail à réaliser
- **Suppression** de `Source/Core/Level/README.md` (dossier singulier, décrit un format hybride
  ASCII+JSON jamais implémenté, supplanté par `Source/Core/Levels/` — le dossier pluriel réellement
  utilisé). Vérifier qu'aucune référence (Doxygen, autre document) ne pointe vers ce fichier avant
  suppression.
- **Réécriture** de `Source/HMI/Editor/README.md` pour refléter le périmètre réellement livré
  (LOT-14 + LOT-15 : palette, outils, caméra, garde-fous, découvrabilité) et retirer les mentions de
  décors/pixel art comme si déjà en place — renvoyer explicitement vers `EX-EDIT-040`/`041` comme
  travail futur, non commencé.
- **`CHANGELOG.md`** : entrée `[Non publié]` décrivant les capacités livrées par LOT-15
  (`EX-EDIT-009`, `012` à `016`), à l'image de l'entrée déjà existante pour LOT-14.
- **Doxygen** : vérifier que la génération (`docs.yml`, `WARN_AS_ERROR`) reste sans avertissement
  après les suppressions/réécritures (références croisées, `@subpage` valides).
- **`scripts/lint_exigences.py`** : vérifier qu'il reste vert (toutes les nouvelles `EX-EDIT-*`
  déclarées une fois, aucune référence orpheline) une fois les tâches précédentes livrées.

## Fichiers impactés
- `Source/Core/Level/README.md` (suppression).
- `Source/HMI/Editor/README.md` (réécriture).
- `CHANGELOG.md`.
- `Documentation/Specification/editeur-niveaux.md` (déjà mis à jour en amont du lot — vérifier la
  cohérence finale avec ce qui a été réellement livré par TACHE-01 à 07, ajuster si un détail a
  changé en cours d'implémentation, comme la pratique établie en LOT-14).

## Tests (obligatoires)
- `python scripts/lint_exigences.py` retourne un code de sortie `0`.
- Génération Doxygen locale (ou CI `docs.yml`) sans avertissement.

## Points d'attention
- Ne renuméroter **aucun** identifiant `EX-…` existant, même en cas d'ajustement de formulation —
  cohérent avec la politique d'immutabilité des identifiants (`conventions.md` §12).
- Cette tâche clôt le lot : c'est l'occasion de vérifier que les critères d'acceptation de
  `epic.md` sont tous effectivement remplis avant de marquer le lot terminé.

## Définition de fait (DoD)
- Documentation cohérente avec le code livré ; lint des exigences et Doxygen verts ; `CHANGELOG.md`
  à jour ; `epic.md` marqué **terminé**, chaque tâche marquée ✅.

## Exigences
Aucune exigence propre — tâche de cohérence documentaire pour l'ensemble du lot.
