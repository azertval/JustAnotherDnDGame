# TACHE-06 — Intégration dans le pas fixe et rendu {#lot-74-tache-06-integration-pas-fixe}

**Lot :** [LOT-74](epic.md) · **Emplacement :** `Source/HMI/Game/GameSession.cpp`,
`Source/HMI/Graphics` · **Statut :** fait

## Contexte
Les deux contrôleurs des TACHE-03 à TACHE-05 sont de la logique **pure** : ils ne savent ni où ils
tournent, ni comment on les dessine. `hmi::GameSession::update` est le point d'assemblage du pas
fixe, et son ordre est **documenté et testé** — l'y insérer correctement est le vrai contenu de
cette tâche.

Ordre existant, résumé : positions précédentes → particules → `_platforms->update()` →
`_blocks->update(...)` → composition de la grille de collision puis `_physics.update(...)` →
collision des blocs réduits → animations → caméra → `_mechanisms->update(...)` → détection
d'événements → `_dangers->update()` → `core::evaluateOutcome`.

## Travail à réaliser
- **Bloc descendant** : `_sinkingBlocks->update(...)` juste après `_platforms->update()`, puis
  **concaténer** ses `core::PlatformSample` à `platformSamples` avant que
  `_blocks->update(..., platformSamples, ...)` et `_physics.update(..., platformSamples)` ne les
  consomment. C'est cette concaténation, et elle seule, qui donne portage et collision continue au
  nouveau bloc — s'il faut écrire autre chose, la décision de cadrage a été perdue (voir TACHE-03).
- **Blocs volatils** : `_volatileBlocks->update(...)` à l'étape 4, avec la boîte du personnage
  **d'avant** le pas (même argument que `_blocks->update`, voir TACHE-04), puis composer son overlay
  dans la grille de collision : mécanismes → volatils → blocs poussables, dans cet ordre, avant
  `_physics.update(...)`.
- **Rechargement** : les deux contrôleurs sont reconstruits par `reload()` comme les autres, ce qui
  remet blocs détruits, blocs disparus et blocs descendus à leur état de fichier. Le vérifier
  explicitement plutôt que de le supposer.
- **Interpolation d'affichage** : le bloc descendant a une position continue, donc il doit passer par
  `snapshotPreviousPositions()` / `hmi::PreviousPosition` comme la plateforme mobile, sans quoi il
  saccadera au rythme du pas fixe au lieu de suivre la fréquence d'affichage.
- **Rendu du jeu et de l'éditeur** : dessiner le bloc descendant à sa position **courante** (et non à
  celle de sa tuile de départ) dans `LevelScene`, et ne plus dessiner un bloc volatil retiré. Côté
  éditeur (`DraftRenderer`), les trois se dessinent à leur position de fichier, comme
  `MovingPlatform` et `DangerMover` — l'éditeur montre l'état de **départ**.
- **Retours sensoriels** : le **clignotement** d'avertissement du bloc éphémère pendant son compte
  à rebours est livré (teinte alpha alternée, rythme dérivé du pas fixe). La secousse caméra d'un
  ground pound qui brise une dalle est déjà produite par le seuil d'impact existant, sans code
  dédié. Les **particules d'éclats**, elles, ne sont **pas** livrées : le contrôleur expose bien le
  front nécessaire (`blocksGoneThisStep()`), mais rien ne le consomme encore côté IHM. Le cadrage
  les classait explicitement en confort (« s'ils dérapent en périmètre, les livrer après le reste
  plutôt que de retarder l'intégration ») ; c'est le choix qui a été fait, et il est consigné ici
  plutôt que passé sous silence. Les trois blocs sont pleinement jouables et lisibles sans elles.

## Fichiers impactés
- `Source/HMI/Game/GameSession.{h,cpp}`.
- `Source/Test/Systeme/test_parcours_complet.cpp` et
  `Source/AiSolver/Env/HeadlessLevelEnvironment.{h,cpp}` — voir Points d'attention : l'ordre du pas
  n'est pas écrit à un seul endroit.

## Constat consigné : l'ordre du pas est écrit **quatre** fois
Le cadrage désignait `hmi::GameSession::update` comme « **le** point d'assemblage ». C'est inexact,
et le lot l'a découvert en voyant ses trois blocs ne rien faire du tout dans les tests système. Le
même ordre de résolution est réécrit dans **quatre** endroits indépendants :

1. `hmi::GameSession::update` — le jeu ;
2. `playLevelTraced()` (`test_parcours_complet.cpp`) — l'orchestration de référence du parcours ;
3. une **seconde** copie dans ce même fichier, pour la trace pas-à-pas ;
4. `aisolver::HeadlessLevelEnvironment::step` — l'environnement d'entraînement de l'IA.

Un garde-fou existe (`ParcoursCompletSystemeHeadlessEnvironment.FideliteParPas`, qui compare 2 et 4
pas à pas) et il a fait son travail : il a signalé la divergence dès que les blocs volatils n'ont été
branchés que d'un côté. Mais il ne compare que **deux** des quatre, et rien ne compare 1 à 2.
Toute mécanique nouvelle résolue dans le pas fixe doit donc être ajoutée aux quatre — c'est une
dette de conception réelle, consignée ici plutôt que corrigée : la factoriser touche l'IHM, les
tests système et l'IA, et relève d'un lot à elle seule.

## Tests (obligatoires)
- **Ordre du pas** : couvert de fait par `demo-bloc-fragile.json`, dont la traversée scriptée exige
  qu'un ground pound brise une dalle **et poursuive** sa chute. Le placer après `_physics.update`
  fait échouer `ParcoursCompletSysteme.FranchitTouteLaSequence`.
- **Fidélité entre orchestrations** :
  `ParcoursCompletSystemeHeadlessEnvironment.FideliteParPas` compare pas à pas le parcours de
  référence et l'environnement d'entraînement ; il a effectivement signalé la divergence tant que
  les deux contrôleurs n'étaient branchés que d'un côté.
- **Rechargement** : `LaCarteDuNiveauResteImmuable` (TACHE-05) montre qu'un contrôleur reconstruit
  depuis le même `Level` repart d'un tableau intact — c'est exactement ce que fait `reload()`.
- **Non-régression** : sur tous les tableaux existants, l'ajout des deux contrôleurs ne change
  strictement rien (`ctest` vert, `demo-final` et rejeu IA compris).

## Points d'attention
- Ne pas dupliquer `platformSamples` : une seule liste par pas, lue par plusieurs consommateurs.
  Attention à la durée de vie — `PlatformController::samples()` documente que sa référence n'est
  valable que jusqu'au prochain `update()`.
- Un bloc descendant sorti du tableau ne doit **plus rien** produire : ni échantillon, ni collision,
  ni dessin. Un échantillon fantôme porterait encore le personnage.
- Ne pas faire porter au rendu la décision « ce bloc existe-t-il encore » : la source de vérité est
  le contrôleur, le rendu la lit.
- `core::Sprite` n'a **aucun** drapeau de visibilité (ni le rendu de tuiles de moyen de sauter une
  entité) : masquer un bloc disparu se fait par sa **teinte alpha**. Ajouter un champ `visible` au
  composant aurait touché tout le rendu pour un besoin de trois tuiles.
- Les particules et le clignotement sont du confort : s'ils dérapent en périmètre, les livrer après
  le reste plutôt que de retarder l'intégration.

## Définition de fait (DoD)
- Les trois blocs se comportent en jeu comme spécifié, les tableaux existants sont inchangés, le
  bloc descendant s'affiche sans saccade ; build `/W4 /WX` ; `ctest` vert.

## Exigences
`EX-GP-027`, `EX-GP-028`, `EX-GP-029`, `EX-GP-026`, `EX-ARCH-011`, `EX-NFR-002`.
