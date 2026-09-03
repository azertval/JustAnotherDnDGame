# LOT-ANNEXE-11 — Entraînement niveau-par-niveau et export de rejeu {#lot-annexe-11}

> Statut : **fait**. Dépend de [LOT-ANNEXE-10](@ref lot-annexe-10) (algorithme
> évolutionniste) et [LOT-ANNEXE-07](@ref lot-annexe-07) (format de rejeu v1, décodage `argmax`).
> Dernier lot de la génération 2 : applique l'algorithme à un niveau réel du jeu et produit un
> livrable concret — une séquence d'actions rejouable en jeu, sans aucune inférence live.

## Objectif
LOT-ANNEXE-10 livre une **mécanique** générique — une génération à la fois, sans savoir ce qu'est
un niveau résolu ni quand s'arrêter. Ce lot en fait un **usage** concret : l'appliquer à **un**
fichier de niveau donné, avec un vrai critère d'arrêt, puis transformer le meilleur individu obtenu
en un livrable exploitable — une séquence d'actions déterministe, exportée au format de rejeu v1
(LOT-ANNEXE-07). C'est la première fois que le programme Lot-Annexe produit quelque chose qui
pourrait, une fois un lot d'intégration ultérieur écrit (génération 5), être rejoué **en jeu**.

## Périmètre

### Inclus
- **Boucle d'entraînement pour un fichier de niveau donné**
  (`aisolver::training::LevelTrainingSession`) : charge un niveau, construit son
  `HeadlessLevelEnvironment`, pilote `EvolutionaryTrainer::runGeneration()` (LOT-ANNEXE-10)
  génération après génération jusqu'à un critère d'arrêt.
- **Critère d'arrêt** à deux branches : le niveau est résolu par le meilleur individu **N fois
  d'affilée** (robustesse), ou un **plafond de générations** est atteint (borne le coût d'un
  entraînement qui ne convergerait jamais — cas normal pour cette ligne de base face à des niveaux
  que seule la génération 3 résoudra).
- **Rejeu déterministe** du meilleur individu final (décodage `argmax`, LOT-ANNEXE-07) pour produire
  la séquence d'actions gagnante — reproduit, hors ligne, exactement le comportement déjà observé
  pendant l'entraînement (même décodage déterministe qu'en LOT-ANNEXE-10).
- **Export** de cette séquence au format de rejeu v1 (LOT-ANNEXE-07, `Source/AiSolver/Replay`), avec
  refus explicite d'exporter un résultat non résolu.
- **Documentation d'un ordre de niveaux indicatif** pour l'usage manuel de ce lot — reprend l'ordre
  de la séquence `demo-*.json` existante (un mécanisme à la fois, difficulté croissante), sans
  aucun mécanisme logiciel d'enchaînement.

### Exclus (hors périmètre de ce lot)
- **Progression automatique d'un niveau à l'autre** : aucun enchaînement de runs, aucune liste de
  niveaux consommée par le code — décision ferme, transverse au programme Lot-Annexe, réaffirmée
  ici avec la plus grande insistance parce que c'est le lot le plus susceptible d'être tenté d'y
  déroger « pour automatiser le curriculum ». Chaque exécution d'entraînement porte sur **un seul**
  fichier de niveau, sans état partagé avec une exécution précédente ou suivante.
- **Interface graphique ou CLI ergonomique** de sélection de niveau : un point d'entrée minimal
  (chemin de niveau en argument) suffit à ce lot ; l'outillage complet est un sujet de génération 5
  (LOT-ANNEXE-17 et suivants).
- **Rejeu en jeu du fichier exporté** (lecture/exécution du format v1 dans `HMI`) : ce lot **écrit**
  le fichier de rejeu, il ne le **consomme** pas — la lecture en jeu est un lot d'intégration futur.
- **Auto-tuning de la configuration évolutionniste par niveau** (taille de population, taux de
  mutation ajustés automatiquement selon la difficulté perçue) : les valeurs par défaut de
  LOT-ANNEXE-10 sont réutilisées telles quelles, ajustables manuellement via `EvolutionaryConfig`,
  jamais choisies par une heuristique automatique.

## Décisions de cadrage
- **Un run = un niveau, strictement, sans exception.** `LevelTrainingSession` prend un unique
  chemin de fichier niveau en entrée ; il n'existe, nulle part dans ce lot, de type ou de fonction
  acceptant une liste de niveaux. C'est la mise en application concrète de la décision transverse du
  programme Lot-Annexe — ce lot est celui où elle est le plus visible et le plus facilement
  vérifiable par simple lecture du code (aucun `std::vector<Level>` en entrée d'une session).
- **Critère d'arrêt « résolu N fois d'affilée » interprété à la lumière du déterminisme.**
  L'évaluation de fitness (LOT-ANNEXE-10) et le rejeu (TACHE-02) sont tous deux déterministes : un
  individu aux poids fixes produit toujours la même trajectoire sur un même niveau. « Résolu N fois
  d'affilée » ne signifie donc pas répéter N fois un tirage aléatoire (l'issue serait identique à
  chaque répétition), mais exiger que le meilleur individu **reste invaincu comme champion** (aucun
  individu ne le dépasse) et **résolvant** pendant N générations consécutives — un filtre de
  stabilité contre un pic de fitness isolé, pas contre une variance d'évaluation qui n'existe pas
  ici.
- **Le rejeu final réutilise le même code de décodage `argmax`** que l'évaluation de fitness de
  LOT-ANNEXE-10 (pas une réimplémentation séparée) : élimine toute possibilité de divergence entre
  « ce que l'algorithme a mesuré comme réussite » et « ce qui est effectivement exporté ».
- **Un fichier de rejeu n'est exporté que pour un entraînement réellement résolu.** Si le critère
  d'arrêt est le plafond de générations (échec, dans le vocabulaire de ce lot), aucun fichier n'est
  écrit — un livrable « démonstration IA » qui échouerait en jeu serait pire qu'une absence de
  livrable, d'autant qu'aucune inférence live ne permet de le corriger a posteriori.
- **L'ordre de niveaux (TACHE-04) est une note d'usage, pas du code.** Aucune structure de données
  ne l'encode ; c'est un tableau documentaire destiné à qui lance manuellement des sessions
  d'entraînement, sur le modèle du curriculum pédagogique du jeu lui-même (`Source/Elements/Levels/
  README.md`, séquence `demo-*.json`, `LOT-25`) — un mécanisme introduit à la fois, sans qu'aucun
  run d'entraînement n'en dépende programmatiquement.

## Notions abordées
Applique directement @ref guide-annexe-algorithmes-evolutionnistes (LOT-ANNEXE-10) à un niveau
réel — aucune notion nouvelle par rapport à ce chapitre ; le curriculum indicatif (TACHE-04)
s'inspire du principe pédagogique « un mécanisme à la fois » déjà appliqué par le jeu lui-même
(`Source/Elements/Levels/README.md`), pas d'une source académique.

## Exigences couvertes
- Nouvelle, déclarée dans [la spécification IA](@ref spec-ia) : [`EX-IA-012`](@ref EX-IA-012).
- Réutilisées (inchangées) : `EX-IA-011` (algorithme évolutionniste, LOT-ANNEXE-10), format de
  rejeu v1 et décodage `argmax` de LOT-ANNEXE-07.

## Découpage

> État : ✅ fait · 🔄 en cours · ⬜ non commencé.

| Tâche | Intitulé | Emplacement | État |
|-------|----------|-------------|:----:|
| [TACHE-01](tache-01-boucle-entrainement.md) | Boucle d'entraînement pour un niveau et critère d'arrêt | `Source/AiSolver/Training` | ✅ |
| [TACHE-02](tache-02-rejeu-deterministe.md) | Rejeu déterministe du meilleur individu | `Source/AiSolver/Training` | ✅ |
| [TACHE-03](tache-03-export-rejeu.md) | Export au format de rejeu v1 | `Source/AiSolver/Training` | ✅ |
| [TACHE-04](tache-04-ordre-niveaux-curriculum.md) | Documentation de l'ordre de niveaux indicatif | `Documentation/Lot-Annexe` | ✅ |

## Critères d'acceptation du lot
1. Une exécution d'entraînement prend en entrée un unique chemin de fichier niveau et ne
   charge/consulte jamais aucun autre niveau — aucune progression automatique n'existe dans le code.
2. L'entraînement s'arrête dès que le meilleur individu résout le niveau et reste invaincu comme
   champion pendant N générations consécutives (constante configurable), ou au plafond de
   générations, selon ce qui survient en premier.
3. Le meilleur individu final est rejoué en mode déterministe (`argmax`) pour produire une séquence
   d'actions ; rejouée indépendamment sur `HeadlessLevelEnvironment`, cette séquence résout
   effectivement le niveau lorsque l'entraînement s'est arrêté par résolution (testé).
4. La séquence est exportée au format de rejeu v1 (LOT-ANNEXE-07) uniquement lorsque l'entraînement
   a abouti à une résolution ; aucun fichier n'est écrit pour un arrêt par plafond de générations.
5. L'ordre indicatif des niveaux (TACHE-04) est documenté et cohérent avec l'ordre effectif de la
   séquence `demo-*.json` (`Source/Elements/Levels/README.md`, `Source/Test/Systeme/
   test_parcours_complet.cpp`).
6. Build `/W4 /WX` sans avertissement, `ctest` vert, Doxygen à jour, `EX-IA-012` déclarée.

## Dépendances
Bâtit sur [LOT-ANNEXE-10](@ref lot-annexe-10) (algorithme évolutionniste) et [LOT-ANNEXE-07](@ref
lot-annexe-07) (format de rejeu v1, décodage `argmax`), transitivement sur toute la chaîne dont
LOT-ANNEXE-10 dépend (LOT-ANNEXE-01/03/05/06/08/09). Dernier lot de la génération 2 : aucun lot de
cette génération n'en dépend. La génération 3 (LOT-ANNEXE-12 et suivants) ne dépend pas non plus de
ce lot — elle réutilise `HeadlessLevelEnvironment`/la récompense/les statistiques en amont, pas cet
algorithme évolutionniste lui-même, qui reste sa **ligne de base de comparaison**.

## Navigation des tâches
- @subpage lot-annexe-11-tache-01-boucle-entrainement
- @subpage lot-annexe-11-tache-02-rejeu-deterministe
- @subpage lot-annexe-11-tache-03-export-rejeu
- @subpage lot-annexe-11-tache-04-ordre-niveaux-curriculum
