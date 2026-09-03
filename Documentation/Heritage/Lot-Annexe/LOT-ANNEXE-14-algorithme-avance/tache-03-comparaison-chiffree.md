# TACHE-03 — Comparaison chiffrée contre la génération 2 et le reste de la génération 3 {#lot-annexe-14-tache-03-comparaison-chiffree}

**Lot :** [LOT-ANNEXE-14](epic.md) · **Emplacement :** `Source/AiSolver/Training/Advanced` ·
**Statut :** fait

## Contexte
C'est la tâche de clôture de la génération 3, et du même coup de l'exigence ferme de l'utilisateur :
démontrer, chiffres à l'appui, qu'un apprentissage dirigé par gradient (REINFORCE, acteur-critique,
et maintenant l'algorithme avancé retenu en TACHE-01) dépasse la recherche aveugle par population de
la génération 2 (LOT-ANNEXE-10). Le comparateur de convergence introduit par LOT-ANNEXE-13
(TACHE-04) est directement réutilisable et étendu ici à quatre séries au lieu de deux.

## Travail à réaliser
- **Protocole de comparaison** : sur un **même jeu de niveaux de contrôle** (au minimum celui déjà
  utilisé par LOT-ANNEXE-12/13 ; idéalement 2-3 niveaux de difficulté croissante pour une conclusion
  plus robuste), exécuter des runs répétés (plusieurs graines) pour chacune des quatre approches :
  évolutionniste (LOT-ANNEXE-10), REINFORCE (LOT-ANNEXE-12), acteur-critique (LOT-ANNEXE-13),
  algorithme avancé (TACHE-01/TACHE-02 de ce lot).
- **Extension de `ConvergenceComparator`** (LOT-ANNEXE-13) : généraliser de deux à *N* séries de CSV
  en entrée, mêmes métriques (nombre d'épisodes moyen jusqu'à un plafond de récompense donné,
  écart-type de la récompense en fin de run), calculées uniformément pour les quatre approches. Une
  généralisation raisonnable en `Source/AiSolver/Training/Advanced/GenerationComparator.h/.cpp` (ou
  extension du comparateur de LOT-ANNEXE-13 déplacé à un niveau commun) est acceptable, à condition
  de ne pas dupliquer la logique de lecture/calcul déjà écrite en LOT-ANNEXE-13.
- **Budget d'évaluation équitable** : même budget total d'épisodes (ou, pour l'évolutionniste, une
  conversion cohérente entre nombre de générations/individus évalués et nombre d'épisodes de jeu
  joués, pour que la comparaison porte sur une quantité d'expérience de jeu comparable, pas sur un
  nombre d'itérations d'API différent d'un algorithme à l'autre).
- **Consignation du résultat** dans `epic.md` de ce lot (section Critères d'acceptation ou
  complément), avec le jeu de niveaux utilisé, le budget retenu, et un résumé chiffré des quatre
  séries — le point final et vérifiable de l'exigence ferme du programme.

## Fichiers impactés
- `Source/AiSolver/Training/Advanced/GenerationComparator.h` (nouveau, ou extension documentée de
  `ConvergenceComparator` de LOT-ANNEXE-13).
- `Source/AiSolver/Training/Advanced/GenerationComparator.cpp` (nouveau, ou extension).
- `Source/AiSolver/CMakeLists.txt` (nouveaux fichiers le cas échéant).
- `Documentation/Lot-Annexe/LOT-ANNEXE-14-algorithme-avance/epic.md` (résultat chiffré consigné une
  fois la comparaison exécutée).

## Tests (obligatoires)
- **Lecture uniforme de CSV hétérogènes** : le comparateur généralisé lit correctement des CSV aux
  colonnes spécifiques différentes (évolutionniste, REINFORCE, acteur-critique, algorithme avancé),
  sans supposer un schéma unique au-delà des colonnes communes déjà standardisées.
- **Non-régression du cas à deux séries** : le comparateur généralisé, appliqué aux deux mêmes séries
  que LOT-ANNEXE-13 (TACHE-04), produit des résultats identiques à ceux déjà obtenus par
  `ConvergenceComparator`.
- **Conversion de budget équitable** : sur des données synthétiques, la conversion
  générations/individus → épisodes de jeu pour l'évolutionniste produit un total d'épisodes cohérent
  avec le budget déclaré pour les autres approches.
- **Robustesse à une série manquante ou vide** : le comparateur signale explicitement une série
  absente ou inexploitable plutôt que de produire un résultat silencieusement faussé.

## Points d'attention
- **Comparaison honnête, pas orientée** : si l'algorithme avancé ne surpasse pas nettement
  l'acteur-critique sur le jeu de niveaux retenu, documenter ce résultat tel quel — la valeur du lot
  est la mesure elle-même, pas une conclusion prédéterminée. Ceci répond directement à l'exigence
  ferme de l'utilisateur par une démonstration mesurée, pas affirmée.
- **Même jeu de niveaux et même budget pour les quatre approches** : toute différence de protocole
  (niveaux différents, budget différent) invaliderait la comparaison et la conclusion du programme.
- Ce résultat clôt la génération 3 mais **n'est pas un jugement définitif** sur la supériorité
  générale d'une approche : il porte sur les niveaux de contrôle retenus, pas sur une généralisation
  (qui est explicitement le rôle mesuré, séparément, par la génération 4).

## Définition de fait (DoD)
- Comparateur généralisé disponible et testé (`ctest` vert) ; comparaison à quatre séries exécutée au
  moins une fois et son résultat consigné dans `epic.md` du lot ; build `/W4 /WX` sans avertissement ;
  Doxygen à jour ; `EX-IA-015` déclarée comme couverte, mesure comparative à l'appui — clôture de
  l'exigence ferme d'apprentissage par gradient de la génération 3.

## Notions abordées
@ref guide-annexe-ppo-dqn — PPO (limitation du pas par *clipping*) et DQN (valeur d'action, rejeu
d'expérience, réseau cible).

## Exigences
`EX-IA-015` (nouvelle, mesurée par ce lot de comparaison).
