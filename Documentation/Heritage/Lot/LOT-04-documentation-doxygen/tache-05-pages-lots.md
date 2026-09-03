# TACHE-05 — Pages de lots navigables {#lot-04-tache-05-pages-lots}

**Lot :** [LOT-04](epic.md) · **Emplacement :** `Documentation/Lot/` · **Statut :** fait

## Contexte
Les lots (epics + tâches) décrivent le plan de travail et son avancement. Ils doivent former une rubrique « Lots » navigable, chaque lot exposant ses tâches en sous-pages.

## Travail à réaliser
- Créer une page d'entrée « Lots » (par ex. `Lot/README.md` transformé en page avec ancre `{#lots}`) listant les epics en `@subpage`.
- Ajouter à chaque `epic.md` un identifiant de page `{#lot-xx}` et exposer ses tâches en `@subpage`.
- Donner à chaque `tache-*.md` un identifiant de page et un titre exploitables par la navigation.
- Vérifier que le statut de chaque lot/tâche (à faire / fait / terminé) reste lisible sur la page.

## Fichiers impactés
- `Documentation/Lot/README.md` (index de la rubrique).
- `Documentation/Lot/LOT-*/epic.md` et `tache-*.md` (en-têtes de page).

## Vérifications (obligatoires)
- La rubrique « Lots » liste LOT-01→04 et s'ouvre depuis l'accueil.
- Depuis un lot, on accède à ses tâches via la navigation (`@subpage`).
- Les liens epic ↔ tâches (déjà présents) résolvent aussi en pages Doxygen.
- Aucun avertissement Doxygen.

## Points d'attention
- Volume : privilégier une convention d'en-tête **homogène** applicable à tous les lots, y compris les futurs.
- Ne pas dupliquer le contenu ; réutiliser les fichiers existants en y ajoutant les ancres/`@subpage`.

## Définition de fait (DoD)
- Rubrique Lots hiérarchique (lot → tâches), navigable et sans avertissement.

## Exigences
`EX-NFR-012`.
