# TACHE-04 — Assertions JADG_ASSERT {#lot-02-tache-04-assertions}

**Lot :** [LOT-02](epic.md) · **Emplacement :** `Source/Core/Diagnostics` · **Statut :** fait

## Contexte
Les assertions vérifient les **préconditions et invariants** (bugs de programmation). Elles doivent être actives en Debug, nulles en Release, et **testables** sans interrompre la suite de tests.

## Travail à réaliser
- Macro `JADG_ASSERT(condition, message)` :
  - En **Debug** : si `condition` est fausse, appelle le **handler d'assertion** courant (par défaut : journalise via le logger puis rompt/abandonne), avec fichier/ligne/message.
  - En **Release** : se compile en **aucune instruction** (pas d'évaluation de la condition).
- **Handler surchargeable** : une fonction (par ex. `std::function` ou pointeur) invoquée en cas d'échec, remplaçable en test pour enregistrer l'échec au lieu d'abandonner.

## Fichiers impactés
- `Source/Core/Diagnostics/Assert.h`, `Assert.cpp` (nouveau).
- `Source/Test/Unit/test_assert.cpp` (nouveau).

## Tests (obligatoires)
- Condition vraie → le handler n'est **pas** appelé.
- Condition fausse (Debug) → le handler est appelé **une fois** avec message et position ; via un handler de test, l'échec est capturé sans abandon.
- Compilation en Release : la macro ne produit aucun effet (vérification par revue ; pas d'effet de bord évaluable).

## Points d'attention
- L'argument `condition` ne doit être évalué qu'une fois et **uniquement en Debug** (pas d'effet de bord attendu en Release).
- Une assertion signale un **bug**, jamais une erreur d'exécution normale (cf. §9 politique d'erreurs).
- Conventions : macro en `UPPER_SNAKE_CASE`, documentation `.h` + `.cpp`.

## Définition de fait (DoD)
- Comportement Debug/Release correct, handler surchargeable testé (`ctest` vert).
- Compile `/W4 /WX`, formaté, documenté Doxygen.

## Exigences
Conventions §9 et §10 ; `EX-NFR-020`.
