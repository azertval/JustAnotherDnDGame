# TACHE-09 — Tests croisés et déterminisme {#lot-74-tache-09-tests}

**Lot :** [LOT-74](epic.md) · **Emplacement :** `Source/Test/{Unit,Integration,Systeme}` ·
**Statut :** fait

## Contexte
Les TACHE-01 à TACHE-08 portent chacune ses tests propres. Cette tâche couvre ce qu'aucune ne peut
couvrir seule : les **croisements** entre les trois nouveaux blocs et les mécaniques déjà livrées, et
la démonstration explicite du déterminisme.

C'est là que se logent les défauts de ce genre de lot. Le `LOT-72` en a fait l'expérience : sa
première version du dash chargé passait ses tests propres et **cassait `demo-final`** ainsi que
l'entraînement de l'IA, parce que la mécanique nouvelle réinterprétait un geste déjà signifiant. Les
croisements ne sont donc pas un supplément de rigueur, ils sont l'endroit où le lot se valide.

## Travail réalisé
La couverture livrée est **unitaire et système**, plutôt qu'unitaire et d'intégration comme le
cadrage l'envisageait — et c'est un choix, pas un renoncement. Les trois blocs sont composés par
l'orchestration du pas fixe, or celle-ci est réécrite en quatre endroits (voir TACHE-06) : un test
d'intégration n'en exercerait qu'un seul, tandis que les tableaux de démonstration de TACHE-08 les
font tourner dans les **trois** orchestrations vérifiées automatiquement (jeu, parcours de référence,
environnement d'entraînement), avec le garde-fou de fidélité pas-à-pas qui les compare.

Livré :
- **14 tests unitaires** des deux contrôleurs — `test_sinking_block_controller.cpp` (6 cas :
  immobilité avant contact, armement par le dessous, armement irréversible, descente déterministe à
  vitesse constante, arrêt définitif contre la matière, sortie par le bas) et
  `test_volatile_block_controller.cpp` (8 cas : destruction au ground pound, survie à **tout** autre
  geste, pound décalé sans effet, bloc éphémère qui tient tant qu'on est dessus, délai exact en pas
  fixes, non-armement par le dessous, retour sans annulation, carte du niveau immuable).
- **Un test de sérialisation** verrouillant le refus d'un modèle d'IA à l'ancienne largeur
  d'observation (TACHE-07).
- **Trois tableaux joués de bout en bout** par `ParcoursCompletSysteme` (traversée complète,
  atteignabilité de chaque tuile, refus du couloir « droite maintenue »),
  `ParcoursCompletSystemeHeadlessEnvironment.FideliteParPas` et le garde-fou de couverture des
  mécaniques.

Ce que le cadrage envisageait et qui **n'a pas été écrit**, faute d'être exerçable sans nouvelle
infrastructure de test — à consigner plutôt qu'à prétendre couvert :

- **Bloc descendant × bloc poussable** (`EX-GP-022`) : un bloc poussable posé sur un bloc descendant
  est porté avec lui, sans glissement cumulé ; poussé pendant la descente, il se comporte
  normalement.
- **Bloc descendant × plateforme mobile** (`EX-GP-026`) : un bloc descendant qui rencontre une
  plateforme mobile s'arrête dessus ou est porté — décider la règle et la **tester**, plutôt que de
  la découvrir en jeu.
- **Bloc descendant × danger** : un bloc descendant qui amène le personnage sur un danger provoque
  l'échec ; un bloc qui sort du tableau ne provoque rien de particulier.
- **Bloc descendant × pente** : un bloc descendant s'arrête sur une pente comme un bloc poussable
  s'arrête dessus — les surfaces suivies ne sont pas solides pour la grille, c'est le piège classique
  du moteur.
- **Bloc fragile × tous les gestes** : le test central de TACHE-04, rejoué ici avec les mécaniques du
  `LOT-72` (dash chargé, dash boosté, poussée renforcée, jump-cancel, combo) — aucune ne brise le
  bloc, seul le ground pound le fait.
- **Bloc fragile × plaque de pression** (`EX-GP-025`) : un ground pound qui traverse un bloc fragile
  et atterrit sur une plaque l'active normalement.
- **Bloc éphémère × bloc descendant** : un bloc descendant qui se pose sur un bloc éphémère disparu
  poursuit sa descente.
- **Bloc éphémère × mécanismes** : un bloc éphémère au-dessus d'un danger, d'une porte, d'un
  interrupteur — la disparition ne perturbe aucune liaison.
- **Bloc éphémère × mécanismes** : un bloc éphémère au-dessus d'un danger, d'une porte, d'un
  interrupteur.

En revanche, **déterminisme** (`EX-NFR-002`) et **non-régression globale** sont bien couverts : le
premier par `DescenteDeterministeEtAVitesseConstante` (deux contrôleurs identiques, mêmes positions)
et par le délai du bloc éphémère exprimé en pas entiers ; la seconde par `demo-final`,
`RecompenseDemoNiveauxTest` et `RejeuIaSysteme`, tous inchangés et verts — c'est le critère
d'acceptation n° 1 du lot.

## Fichiers impactés
- `Source/Test/Unit/Core/Gameplay/test_sinking_block_controller.cpp` (nouveau),
  `Source/Test/Unit/Core/Gameplay/test_volatile_block_controller.cpp` (nouveau),
  `Source/Test/CMakeLists.txt`.
- `Source/Test/Unit/AiSolver/Nn/test_serialization.cpp`.
- `Documentation/CahierTest.md`, **régénéré** — jamais édité à la main.

## Tests (obligatoires)
Cette tâche **est** les tests. Sa vérification propre est double :
- `ctest` à 100 %, les trois cibles (`unitaire`, `integration`, `systeme`).
- `python scripts/generate_cahier_test.py` rejoué, et le cahier régénéré committé : chaque nouveau
  test porte son bloc `\castest{...}`, faute de quoi il n'apparaît nulle part dans la documentation
  de validation.

## Points d'attention
- **Chaque croisement doit pouvoir échouer.** Un test qui passerait même en retirant le code testé ne
  vaut rien : vérifier au moins pour les croisements les plus subtils (bloc descendant × pente, bloc
  fragile × dash boosté) qu'ils échouent bien quand on neutralise la mécanique.
- Les helpers existants suffisent (`spawnPlayer`, `STEP = 1/60`, niveaux construits en mémoire, aucun
  GPU) — ne pas introduire de fixture nouvelle sans nécessité.
- Toujours borner les boucles de simulation par un nombre d'itérations maximal : un croisement
  mal réglé peut ne jamais converger, et un test qui tourne indéfiniment bloque la CI plutôt que de
  la faire échouer.
- Ne pas déplacer les tests des TACHE-03 à TACHE-05 ici : chaque tâche reste vérifiable seule.

## Définition de fait (DoD)
- Tous les croisements listés sont testés et verts ; déterminisme démontré ; `demo-final` et le rejeu
  IA inchangés ; cahier de test régénéré ; build `/W4 /WX`.

## Exigences
`EX-GP-027`, `EX-GP-028`, `EX-GP-029`, `EX-GP-022`, `EX-GP-025`, `EX-GP-026`, `EX-GP-031`,
`EX-GP-058`, `EX-NFR-002`, `EX-NFR-021`.
