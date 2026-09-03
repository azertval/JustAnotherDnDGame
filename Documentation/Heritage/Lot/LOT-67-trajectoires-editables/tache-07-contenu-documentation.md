# TACHE-07 — Contenu, documentation et referentiel {#lot-67-tache-07-contenu-documentation}

**Lot :** [LOT-67](epic.md) · **Emplacement :** `Source/Elements/Levels`, `Documentation` · **Statut :** fait

## Contexte
Un lot n'est livre que si le referentiel dit la verite sur ce qu'il apporte, et si le contenu livre
illustre le format qu'il introduit.

## Travail a realiser
- Declarer les exigences nouvelles (`EX-GP-054`, `EX-GP-055`, `EX-LVL-008`, `EX-EDIT-032`,
  `EX-EDIT-033`) et **amender** `EX-GP-026`, dont la limite a deux points disparait.
- Migrer `demo-plateforme.json` vers `waypoints`, en laissant **volontairement** une plateforme au
  format `endX`/`endY` : la retrocompatibilite est ainsi prouvee par l'exemple, dans le contenu
  livre, et pas seulement par un test.
- Documenter le format (`Source/Elements/Levels/README.md`) et le comportement
  (`Documentation/Guide/guide-niveaux.md`), en y portant la distinction budget/capacite sous forme
  de tableau.
- Mettre a jour `CHANGELOG.md`, inscrire le lot dans `Documentation/Lot/lots.md`, regenerer
  `Documentation/CahierTest.md`.

## Fichiers impactes
`Source/Elements/Levels/demo-plateforme.json`, `README.md`,
`Documentation/Specification/gameplay.md`, `niveaux.md`, `editeur-niveaux.md`,
`Documentation/Guide/guide-niveaux.md`, `Documentation/Lot/`, `CHANGELOG.md`.

## Tests (obligatoires)
Le lint d'exigences (`scripts/lint_exigences.py`) verifie que chaque exigence declaree est
referencee au moins une fois ; `scripts/generate_cahier_test.py --check` verifie que chaque test
porte son bloc de documentation de cas de test.

## Points d'attention
**Aucun nouveau tableau n'est concu ici.** La migration de format ne change aucun comportement, ce
qui garde le test de parcours complet valide. Dessiner un niveau exploitant reellement les circuits
fermes est un acte de *level design*, distinct de ce lot d'outillage — et il faudra alors eviter d'y
meler une pente, a cause du defaut connu.

## Definition de fait (DoD)
Lint d'exigences vert, cahier de test regenere, tous les niveaux livres se chargent, `ctest` a
100 %.

## Exigences
`EX-LVL-008`, `EX-LVL-005`, `EX-GP-054`, `EX-GP-055`, `EX-EDIT-032`, `EX-EDIT-033`.
