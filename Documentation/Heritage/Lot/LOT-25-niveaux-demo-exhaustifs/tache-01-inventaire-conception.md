# TACHE-01 — Inventaire des mécaniques et conception {#lot-25-tache-01-inventaire-conception}

**Lot :** [LOT-25](epic.md) · **Emplacement :** `Documentation` · **Statut :** fait

## Contexte
Avant d'écrire un seul niveau (`TACHE-02`), établir la liste complète de ce qui doit être exercé
et décider **combien** de niveaux couvrent cette liste — le nombre n'est volontairement pas fixé
avant cette tâche (voir décision de cadrage de l'épic).

## Travail à réaliser
- Dresser la liste des mécaniques jouables **par niveau** (exclut manette/menu d'options/
  localisation, transverses et déjà testées ailleurs) à partir des spécifications
  (`Documentation/Specification/gameplay.md`) et du CHANGELOG : mouvement, saut (simple, variable,
  coyote time, jump buffering), double saut, wall jump/wall slide, dash 8 directions, gravité
  asymétrique/apex/fast-fall, chute newtonienne, interrupteur↔porte, plaque de pression, bloc
  poussable, budgets de sauts/dashs, pentes (`LOT-22`), arrondi (`LOT-23`), blocs à taille
  fractionnaire (`LOT-24`).
- Pour chaque mécanique de la liste, prévoir un niveau **nouveau** qui l'exerce de façon
  **nécessaire** — un tableau mécanique → niveau(s), consigné dans ce fichier ou dans `epic.md`
  une fois complété. Les niveaux existants (`demo.json`…`demo5.json`) ne sont **pas repris** : la
  base repart vide (voir décision de cadrage de l'épic), ce tableau ne porte donc que sur des
  niveaux à créer.
- Trancher le **nombre final** de niveaux et leur **ordre** (difficulté croissante) : documenté ici
  avant que `TACHE-02` ne commence à produire des fichiers.
- Concevoir le contenu du **niveau final combiné** (dernier de la séquence, voir `epic.md`) : choisir
  quelles mécaniques il mélange et comment (ex. un dash au-dessus d'une pente, un bloc poussé sur
  une plaque de pression, un mur à double saut suivi d'un wall jump) — un vrai enchaînement, pas une
  simple juxtaposition d'obstacles isolés bout à bout.
- Définir la **convention de nommage** des nouveaux fichiers (ex. `demo-double-saut.json`,
  `demo-pente.json`, `demo-final.json` pour le niveau combiné — voir `TACHE-02`), pour que le
  tableau de cette tâche l'utilise directement.

## Fichiers impactés
- `Documentation/Lot/LOT-25-niveaux-demo-exhaustifs/tache-01-inventaire-conception.md` (ce
  fichier, complété avec le tableau final) ou `epic.md`.

## Tests (obligatoires)
- Aucun test automatisé propre à cette tâche (travail de conception) — la validation est la
  **revue** du tableau mécanique → niveau avant de passer à `TACHE-02`.

## Points d'attention
- **Une mécanique par niveau reste un objectif, pas une règle rigide** : deux mécaniques très liées
  (ex. wall jump et wall slide, ou coyote time et jump buffering, déjà intriqués dans le
  *game feel* du saut) peuvent raisonnablement partager un seul niveau si les séparer produirait un
  niveau trivial ou artificiel.
- **Ne pas oublier la plaque de pression** (mécanique de `demo5.json`, l'ancien niveau dont
  l'absence du test système est le symptôme déclencheur de ce lot) dans l'inventaire — la base
  repartant vide, rien ne garantit qu'elle soit couverte sans une vérification explicite du
  tableau final.

## Définition de fait (DoD)
- Tableau mécanique → niveau(x) complet, nombre de niveaux final tranché et documenté, contenu du
  niveau final combiné décrit, revu avant le début de `TACHE-02`.

## Tableau mécanique → niveau (final)

Convention de nommage : `demo-<mecanique>.json`, préfixe commun à tous les niveaux de cette
séquence (remplace la numérotation `demoN.json` qui ne disait rien du contenu). **13 niveaux** au
total (12 isolés + 1 final combiné), ordre = difficulté croissante.

| # | Fichier | Mécanique(s) exercée(s) | Nécessaire pour franchir |
|---|---------|--------------------------|---------------------------|
| 1 | `demo-deplacement.json` | Mouvement horizontal, chute, sol | Avancer (aucun saut) sur un terrain à paliers descendants, dangers en creux |
| 2 | `demo-saut.json` | Saut simple, hauteur variable | Un fossé franchissable seulement en sautant |
| 3 | `demo-double-saut.json` | Saut aérien (double saut, `EX-GP-015`) | Deux fossés successifs, le second hors de portée d'un saut simple |
| 4 | `demo-wall-jump.json` | Wall slide + wall jump (`EX-GP-016`) | Un puits vertical franchissable en enchaînant des wall jumps entre deux murs |
| 5 | `demo-dash.json` | Dash 8 directions (`EX-GP-017`) | Couloir bas (saut impossible) + fosse, franchi en avançant et dashant |
| 6 | `demo-interrupteur.json` | Interrupteur ↔ porte (`EX-GP-020`) | Une porte fermée bloque la sortie tant que l'interrupteur n'est pas touché |
| 7 | `demo-plaque-pression.json` | Plaque de pression (`EX-GP-025`) | Un mur d'un bloc barre le sol ; la plaque, juste avant, ouvre une porte au-dessus du mur — un saut pendant que la porte est ouverte permet de passer par-dessus |
| 8 | `demo-bloc.json` | Bloc poussable (`EX-GP-022`) | Un bloc à pousser comble une fosse pour servir de pont |
| 9 | `demo-budget.json` | Budget de sauts/dashs (`EX-GP-024`) | Budget **limité** (un seul saut) : un deuxième saut inutile ne doit pas être nécessaire, mais un niveau qui en exigerait deux échouerait — prouve le câblage JSON → spawn → décompte |
| 10 | `demo-pente.json` | Pente (`EX-GP-003`, `LOT-22`) | Palier surélevé atteignable seulement en suivant une pente (trop haut pour un saut direct) |
| 11 | `demo-arrondi.json` | Arrondi (`EX-GP-004`, `LOT-23`) | Variante courbe de la pente, même principe |
| 12 | `demo-bloc-reduit.json` | Bloc à taille réduite (`EX-GP-005`, `LOT-24`) | Un bloc `×0.5` sert de marchepied partiel ; l'espace autour reste franchissable (prouve la boîte réduite, pas la case entière) |
| 13 | `demo-final.json` | **Combinaison** : dash, pente, bloc poussable, interrupteur/porte, double saut | Niveau final : un unique parcours enchaînant ces mécaniques dans un ordre cohérent (pas une juxtaposition d'obstacles isolés) |

**Mécaniques couvertes implicitement, sans niveau dédié** : gravité asymétrique (chute plus rapide
que la montée), flottement à l'apex, fast-fall, chute newtonienne (vitesse terminale) — ce sont des
modulateurs **passifs** de tout saut/chute (`EX-GP-018`/`EX-GP-019`), déjà exercés par **chaque**
niveau de la séquence dès qu'un saut ou une chute a lieu ; ils n'ont pas de mode « échec » qu'un
script d'entrées déterministe pourrait démontrer (contrairement à « ce niveau échoue sans le
dash ») — déjà couverts en détail par les tests unitaires/intégration
(`test_physique_personnage.cpp`). De même, **coyote time** et **jump buffering** sont des
tolérances de *timing* qui compensent une imprécision **humaine** : un script d'entrées
déterministe n'a pas cette imprécision à compenser, donc aucun niveau dédié ne peut en démontrer la
nécessité de façon significative — restent couverts par leurs tests dédiés
(`CoyoteTimeAutoriseUnSautJusteApresLeBord`, `JumpBufferingHonoreUnSautPreAppuye`).

**Note de conception — `demo-plaque-pression.json`** : la géométrie initialement envisagée
(plaque, porte, sortie empilées verticalement l'une au-dessus de l'autre) s'est révélée
**infranchissable par construction**, indépendamment du script d'entrées : la plaque
(`EX-GP-025`) est une activation **continue**, réévaluée chaque pas à partir du seul recouvrement
de la boîte du personnage — dès que celle-ci quitte la case de la plaque (nécessaire pour
atteindre une sortie placée au-delà d'une porte pleine case), la porte se referme **avant** que le
personnage, plus petit qu'une case (0,4×0,8), n'ait fini de la traverser. Vérifié à la fois par le
calcul (le recouvrement plaque/sortie exigerait un déplacement supérieur à la case en un seul pas,
hors de portée de toutes les vitesses du jeu) et empiriquement (balayage exhaustif du délai de
saut et d'un double saut de secours, aucune combinaison ne passe). Le niveau livré contourne cette
limite d'architecture sans la travestir : la plaque ouvre une porte **au-dessus d'un mur** d'une
case (obstacle au sol), traversée par un **saut par-dessus** pendant la fenêtre où la porte est
ouverte, puis retour normal au sol. La porte est **fermée par défaut** (elle ne devient
franchissable qu'après recouvrement de la plaque, `MechanismController`) et le test
`NiveauPlaquePressionExigeUnSaut` confirme qu'un script qui ne fait que marcher (sans jamais
sauter) reste bloqué contre le mur, même une fois la plaque recouverte en chemin — la mécanique
reste donc bien nécessaire au franchissement, sans dépendre d'un franchissement au pas près.

## Niveau final combiné (`demo-final.json`)

Parcours unique enchaînant, dans l'ordre : (1) un couloir bas + fosse franchi au **dash** ; (2) une
**pente** montante menant à un palier plus haut ; (3) un **bloc poussable** à pousser dans une fosse
pour servir de pont ; (4) un **interrupteur** à toucher pour ouvrir une **porte** qui barre la
suite ; (5) un dernier fossé franchi au **double saut**, juste avant la sortie. Chaque étape reste
**nécessaire** (retirer mentalement une mécanique bloque le parcours à cet endroit précis), tout en
formant un seul niveau cohérent plutôt qu'une suite d'obstacles isolés bout à bout.

## Exigences
Aucune exigence propre — tâche de conception, préalable à l'implémentation.
