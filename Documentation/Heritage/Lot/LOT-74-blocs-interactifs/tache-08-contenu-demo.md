# TACHE-08 — Contenu : trois tableaux, séquence, parcours scripté {#lot-74-tache-08-contenu-demo}

**Lot :** [LOT-74](epic.md) · **Emplacement :** `Source/Elements/Levels`, `Source/Test/Systeme` ·
**Statut :** fait

## Contexte
Une mécanique livrée mais posée nulle part n'existe pas pour le joueur, et le dépôt le fait
respecter par deux garde-fous complémentaires, tous deux hérités du `LOT-65` :

- `Test/Systeme/test_couverture_mecaniques.cpp` dérive l'inventaire attendu de l'**énumération**
  (jamais d'une liste recopiée) et exige que chaque type soit posé **au moins
  `MIN_OCCURRENCES = 3` fois** dans la séquence livrée. Sa version initiale se contentait de la
  présence — insuffisant : la séquence la satisfaisait avec un exemplaire unique de presque chaque
  mécanique, dont plusieurs hors d'atteinte du personnage.
- `ParcoursCompletSysteme` (`test_parcours_complet.cpp`) rejoue la séquence entière et relève la
  trajectoire **réelle** : il refuse une mécanique posée mais inatteignable.

Autrement dit, les trois nouvelles tuiles doivent être posées trois fois **chacune** et réellement
traversées. La séquence compte 22 tableaux, bâtis sur le principe « une mécanique introduite à la
fois, puis combinée » (`EX-LVL-012`) — trois nouveaux tableaux, à la suite des trois tableaux de bloc
poussable, en sont le prolongement naturel.

C'est la tâche la plus lourde du lot : chaque tableau ajouté demande son **parcours scripté**.

## Travail à réaliser
- Trois tableaux dans `Source/Elements/Levels/` :
  - `demo-bloc-descendant.json` — enseigne « le toucher, c'est le lancer » : deux fosses enjambées
    par des blocs qui s'enfoncent dès le contact, et un cinquième bloc posé sur du sol plein, qui ne
    descend donc **pas** — la démonstration qu'un bloc descendant ne traverse jamais la matière.
  - `demo-bloc-fragile.json` — enseigne « le ground pound a enfin une cible » : cinq dalles fragiles
    dans le sol, sur lesquelles on peut aussi bien marcher sans les briser. `dashCharges: 0`, sans
    quoi « dash + bas » resterait un dash vertical et le geste serait inatteignable (`EX-GP-058`).
  - `demo-bloc-ephemere.json` — enseigne « on ne revient pas » : cinq dalles qui s'effacent une fois
    quittées, le chemin se refermant derrière le personnage.
- Les insérer dans `Source/Elements/Levels/sequence-demo.json`, après `demo-bloc-quart.json` pour les
  deux premiers types de la famille « bloc », en respectant la contrainte de progression du bloc
  fragile ci-dessus.
- Une **phase scriptée par tableau** dans `Source/Test/Systeme/ScriptedLevelSequence.h`, sur le modèle
  des phases existantes (lambda courte prenant `(pas, player, x, y)` et renvoyant un `PlayerInput`,
  précédée d'un commentaire numéroté qui explique le tracé et ce que le tableau démontre).
- `Source/Elements/Levels/README.md` : les trois tableaux dans la description de la séquence.

## Fichiers impactés
- `Source/Elements/Levels/demo-bloc-descendant.json`, `demo-bloc-fragile.json`,
  `demo-bloc-ephemere.json` (nouveaux), `sequence-demo.json`, `README.md`.
- `Source/Test/Systeme/ScriptedLevelSequence.h`.

## Tests (obligatoires)
- `test_couverture_mecaniques` : les trois types atteignent le seuil de trois occurrences.
- `ParcoursCompletSysteme` : la séquence entière se termine, les trois nouveaux tableaux compris, et
  chaque nouvelle mécanique est effectivement **touchée** par la trajectoire relevée.
- `scripts/check_demo_sequence.py` : le jeu et le module de test énumèrent toujours les mêmes
  fichiers.
- Chargement/validation des trois fichiers (`EX-LVL-004`).

## Points d'attention
- **Ne pas modifier les tableaux existants.** Leurs parcours scriptés sont calibrés à la tuile près
  — le `LOT-71` a dû réécrire 25 phases pour un seul tableau retracé. Toute la couverture nouvelle
  passe par les trois tableaux ajoutés.
- **Le plafond de pas** du garde-fou système est une borne de terminaison, pas une mesure de
  difficulté (`LOT-71`) : si la séquence allongée le dépasse, le relever est légitime — mais
  vérifier d'abord que ce n'est pas un tracé qui piétine.
- **Un bloc éphémère mal placé rend un tableau insoluble**, et c'est définitif (TACHE-05). Si
  `ParcoursCompletSysteme` échoue, corriger le tableau, jamais le test.
- Le bloc fragile demande un ground pound, donc **aucune charge de dash disponible** au moment du
  geste : `demo-bloc-fragile.json` déclare donc `dashCharges: 0`. C'est ce champ qui a mis au jour un
  **écart de fidélité préexistant** (voir plus bas).
- **Fosses d'une seule case, délibérément.** Les trois tableaux ne creusent jamais plus d'une case :
  le personnage peut toujours en ressortir d'un saut. Une disparition définitive (`EX-GP-029`) ou un
  bloc qui s'enfonce ne doivent jamais pouvoir **enfermer** le joueur dans un tableau
  d'apprentissage — la difficulté appartient à `demo-synthese` et `demo-final`.
- **Une marche à franchir avant la sortie**, dans chacun des trois. Sans elle, deux des tableaux
  étaient traversables en maintenant simplement « droite », ce que le garde-fou anti-couloir
  (`AucunTableauNEstFranchissableEnMaintenantDroite`, `LOT-65`) refuse à juste titre : un tableau
  livré doit exiger une mécanique de mouvement, pas seulement du temps.
- **Le tracé scripté du ground pound a dû être gardé en altitude.** Une première version rebouclait
  indéfiniment : le pound brisait la dalle, le personnage tombait dans le trou d'une case, en
  ressautait, se retrouvait de nouveau dans la fenêtre horizontale du pound, et n'avançait plus
  jamais. La garde `y <= 1.5` ne retient que les sauts partis de la passerelle (le plafond y borne
  l'apogée), pas ceux partis du trou : ce n'est pas un réglage cosmétique mais la **condition de
  terminaison** du parcours.

## Écart de fidélité mis au jour (hors périmètre initial)
`demo-bloc-fragile.json` est le premier tableau livré à déclarer une **capacité de tableau**
(`EX-GP-055`, ici `dashCharges: 0`). Il a révélé que l'orchestration de référence de
`test_parcours_complet.cpp` ne les appliquait **pas**, là où `hmi::GameSession::update` et
`aisolver::HeadlessLevelEnvironment` les appliquent toutes deux : le même tableau donnait donc deux
trajectoires selon l'orchestration, et « dash + bas » y était un dash vertical d'un côté, un ground
pound de l'autre. Corrigé dans les deux copies de l'orchestration de référence. Sans conséquence
jusqu'ici — aucun tableau livré ne déclarait ces champs — mais le défaut était bien réel.
- Trois tableaux d'apprentissage, pas trois tableaux de synthèse : une mécanique à la fois, les
  combinaisons appartiennent à `demo-synthese` et `demo-final`, hors périmètre ici.

## Définition de fait (DoD)
- Les trois tableaux sont jouables, dans la séquence, couverts et traversés par le parcours scripté ;
  les quatre linters et `ctest` sont verts.

## Exigences
`EX-LVL-004`, `EX-LVL-012`, `EX-LVL-015`, `EX-NFR-021`, `EX-GP-027`, `EX-GP-028`, `EX-GP-029`,
`EX-GP-058`.
