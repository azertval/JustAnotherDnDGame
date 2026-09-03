# TACHE-01 — Test de non-régression du volume de primitives {#lot-62-tache-01-test-budget-primitives}

**Lot :** [LOT-62](epic.md) · **Emplacement :** `Source/Test` · **Statut :** fait

## Contexte
`LOT-40` a rendu le rendu vérifiable sans GPU : `hmi::ComposedScene` produit la liste ordonnée des
primitives d'une image, `hmi::QuadRecorder` la capture, et `statistics()` en donne les compteurs.
Vingt tests utilisent déjà cette capacité pour vérifier l'**ordre** des calques, la **résolution**
des textures ou l'**isolement** d'un calque.

Aucun ne vérifie le **volume**. C'est pourtant la seule chose que `EX-NFR-005` demande de borner, et
la seule qui se dégrade silencieusement : un calque émis deux fois, un culling contourné par un
nouveau type de primitive, un fond redessiné par salle plutôt que par écran. Rien de tout cela ne
casse un test existant — l'ordre reste bon, les textures aussi.

## Travail à réaliser
- **Plafond par niveau livré**, en constante nommée, pour le nombre de primitives **composées** et
  **soumises** : chaque `demo-*.json` est chargé, une caméra de référence est posée (position
  d'entrée, cadrage standard), la scène est composée, les compteurs sont comparés au plafond.
- **Les deux modes de rendu** : Physique et Texture. Le second est structurellement plus lourd
  (fond, décors, ombres, premier plan) — c'est celui qui dérive.
- **Efficacité du culling assertée** : sur un grand niveau, la fraction de primitives écartées reste
  **au-dessus** d'un seuil. Une borne haute sur le total ne dit pas si le culling fonctionne ; une
  borne basse sur ce qu'il écarte, si.
- **Ventilation par calque** disponible dans le message d'échec : savoir que le total a doublé sans
  savoir quel calque a doublé rend le test pénible à exploiter. `hmi::LayerVisibility` (`LOT-51`)
  permet déjà d'isoler un calque — le réutiliser.
- **Test négatif** dans la même tâche : composer volontairement un calque deux fois et vérifier que
  le test échoue. C'est ce qui prouve que le plafond n'est pas simplement très large.

## Fichiers impactés
- `Source/Test/Unit/HMI/Graphics/test_render_budget.cpp` (nouveau).
- `Source/Test/CMakeLists.txt`.
- `Source/HMI/Graphics/ComposedScene.{h,cpp}` — ventilation par calque dans les statistiques, si
  elle n'est pas déjà accessible.

## Tests (obligatoires)
- Chaque `demo-*.json` livré reste sous son plafond, dans les deux modes de rendu.
- Le culling écarte au moins la fraction attendue sur un grand niveau (`demo-salles`).
- **Test négatif** : une double composition d'un calque fait échouer le contrôle.
- Les compteurs sont **déterministes** : deux compositions de la même scène avec la même caméra
  donnent exactement le même nombre.
- Tests **sans GPU**, sans Qt.

## Points d'attention
- **Choisir des plafonds larges.** Un plafond serré transforme chaque lot de contenu en mise à jour
  de constantes, et la constante finit par être ajustée sans réflexion — le test ne vérifie alors
  plus rien. Viser l'ordre de grandeur, pas la valeur exacte.
- **La caméra de référence doit être reproductible** : la dériver de la position d'entrée du niveau
  et d'une taille de cadrage constante, jamais d'un état par défaut susceptible de changer.
- Les tuiles animées (`LOT-46`) font varier le contenu selon le pas : composer à un pas **fixé**,
  sinon le compteur oscille et le test devient instable.
- Ce test s'ajoute à `UnitTests`, qui compile déjà `ComposedScene.cpp`, `ShadowRenderer.cpp`,
  `BackgroundRenderer.cpp` et les autres composeurs purs — vérifier qu'aucune source
  supplémentaire touchant le GPU n'est nécessaire.
- **Ne pas ajuster un plafond pour faire passer le test.** Un dépassement est un résultat : il se
  consigne, et se corrige hors de ce lot.

## Définition de fait (DoD)
- Chaque niveau livré est borné par un plafond nommé dans les deux modes, l'efficacité du culling
  est assertée, l'échec nomme le calque fautif, le test négatif démontre la sensibilité du contrôle,
  et tout s'exécute sans GPU de façon déterministe ; `/W4 /WX` propre.

## Exigences
Réutilise `EX-NFR-005` (primitives bornées et observables — honorée ici), `EX-NFR-004` (vérification
sans GPU), `EX-REN-015` (cadrage par salle), `EX-REN-043` (calques), `EX-NFR-020` (tests).
