# TACHE-05 — Doctrine de conception et garde-fous de profondeur {#lot-65-tache-05-doctrine-gardes-fous-profondeur}

**Lot :** [LOT-65](epic.md) · **Emplacement :** `Documentation/Specification`, `Source/Test` · **Statut :** en cours

## Contexte
La `TACHE-01` avait écrit, en toutes lettres, la limite de son propre garde-fou :

> **Couvert ≠ franchi.** Une mécanique posée dans un coin inaccessible du tableau serait « couverte »
> sans jamais être jouée.

C'est exactement ce qui s'est produit. Une revue des vingt-deux tableaux livrés par les
`TACHE-02`/`TACHE-03` établit que le garde-fou est **vert** alors que :

- **10 tableaux sur 22** se franchissent sans jouer — 9 avec le script `rightOnly()` (maintenir
  « droite », rien d'autre) et `demo-plateforme` avec **aucune entrée** ;
- chaque mécanique n'existe qu'en **une seule instance** dans toute la séquence (941 tuiles `solid`
  contre 1 exemplaire de presque chaque type) ;
- quatre tableaux posent leur sujet **hors d'atteinte** : les quatre variantes de plafond incliné de
  `demo-plafond` sont en ligne 2 au-dessus d'un couloir en ligne 7, alors qu'un saut simple monte de
  ≈ 2,4 tuiles (`jumpSpeed 15`, `gravity 50`, flottement d'apex ×0,5) — la ligne 3,0 n'est jamais
  atteinte. Idem pour les concaves de plafond de `demo-concave` et pour les quatre dangers avancés,
  dont l'interrupteur : `dangerSwitched` ne peut donc **jamais** être commuté ;
- `dashBudget` n'est employé par **aucun** tableau, le garde-fou acceptant `jumpBudget` **ou**
  `dashBudget` ;
- les `zones` de caméra (`EX-LVL-007`) et `roomWidthTiles`/`roomHeightTiles` (`EX-REN-017`), livrées
  par le `LOT-64`, n'apparaissent dans aucun tableau — le garde-fou ne relève que le `mode`.

Le critère d'acceptation n°4 du lot (« introduit une mécanique à la fois, puis les combine dans des
tableaux de synthèse ») et l'exigence `EX-LVL-012` ne sont donc pas tenus. Cette tâche livre la
**doctrine** qui manquait et les **garde-fous** qui la rendent vérifiable — de nouveau **rouges**,
et de nouveau pilotes des tâches suivantes.

## Travail à réaliser
- **Doctrine de conception** dans `Documentation/Specification/niveaux.md` § Conception, en
  complément des trois lignes existantes :
  1. **Chemin critique** — la mécanique du tableau est la *seule* façon d'atteindre la sortie.
     Référence : le `demo-dash` hérité, dont le couloir d'une case de haut rend le saut impossible.
  2. **Répétition** — au moins **trois** instances de la mécanique par tableau : montrer sans
     risque, pratiquer, varier.
  3. **Contrainte de capacité** — `jumpBudget`/`dashBudget` (`EX-GP-024`, « contrainte de puzzle »)
     fixés pour interdire le contournement en force. Sans cela, le double saut et le dash, acquis
     définitivement dès le cinquième tableau, permettent de sauter par-dessus toute énigme.
  4. **Introduction avant emploi** — aucune mécanique mortelle ou bloquante n'apparaît sur le chemin
     critique avant le tableau qui l'a présentée sans risque.
- **Garde-fou anti-couloir** (`Source/Test/Systeme/test_parcours_complet.cpp`) : pour chaque tableau
  de la séquence, affirmer que `playLevel({file, rightOnly()})` ne renvoie **pas** `Won`. Exclusions
  nommées et justifiées, comme `excludedTileTypes()` — `demo-deplacement` seul, dont c'est le sujet.
- **Garde-fou de profondeur** (`test_couverture_mecaniques.cpp`) : remplacer « présent au moins une
  fois » par un **compte minimal d'occurrences** par type de tuile.
- **Séparer les deux budgets** : `jumpBudget` et `dashBudget` deviennent deux critères distincts.
- **Variantes de cadrage** : étendre `CoverageState` aux `zones` de caméra et à une taille de
  salle/suivi explicitement déclarée.
- **Garde-fou de proximité au trajet** : pendant le rejeu scripté, enregistrer la trajectoire du
  personnage et affirmer que chaque tuile de mécanique du tableau passe à portée d'un saut d'une
  position réellement occupée — le contrôle qui manquait à `TACHE-01`.

## Fichiers impactés
- `Documentation/Specification/niveaux.md` — § Conception.
- `Source/Test/Systeme/test_couverture_mecaniques.cpp` — profondeur, budgets séparés, cadrage.
- `Source/Test/Systeme/test_parcours_complet.cpp` — anti-couloir, proximité au trajet.

## Tests (obligatoires)
- Les quatre contrôles **échouent** au départ en nommant précisément les tableaux et mécaniques en
  cause — résultat attendu de cette tâche.
- **Test négatif** pour chacun, sur des données en mémoire, sans dépendre des niveaux sur disque :
  un tableau franchi par `rightOnly()` est signalé ; un type présent deux fois seulement est
  signalé ; une tuile hors de portée du trajet est signalée.
- Les exclusions sont **toutes** commentées et justifiées.
- Test `Core` pur, sans GPU ; `/W4 /WX` propre.

## Points d'attention
- **Le seuil de répétition est un choix, pas une vérité.** Trois instances suffisent à distinguer
  « posé » de « pratiqué » sans imposer de remplissage ; le fixer dans une constante nommée, pas en
  littéral dispersé.
- **La proximité au trajet doit tolérer le hors-chemin volontaire.** Un secret facultatif reste
  légitime ; le contrôle vise ce qui est *inatteignable*, pas ce qui est *optionnel*. Calibrer sur
  la hauteur d'un saut simple et non d'un double saut, et le documenter.
- **Ne pas transformer l'anti-couloir en interdiction de marcher.** Un tableau peut comporter de
  longues portions plates ; ce qui est refusé, c'est qu'il se *termine* sans autre entrée que
  « droite ».
- Réutiliser `playLevel` tel quel plutôt que d'en écrire une variante : la composition des
  contrôleurs (plateformes, blocs, mécanismes) y est déjà exactement celle de `hmi::GameSession`.

## État initial constaté
Les quatre contrôles livrés, exécutés sur la séquence héritée des `TACHE-02`/`TACHE-03` :

**Anti-couloir — 10 tableaux franchis en maintenant « droite »** : `demo-interrupteur`,
`demo-pente`, `demo-pente-gauche`, `demo-arrondi`, `demo-concave`, `demo-plafond`,
`demo-bloc-quart`, `demo-dangers-avances`, `demo-dangers-directionnels`, `demo-salles`. En y
ajoutant `demo-plateforme`, franchi sans **aucune** entrée, **onze tableaux sur vingt-deux** ne
demandent rien au joueur. `demo-bloc-quart` n'avait pas été repéré par la revue manuelle : son
script de rejeu comportait un saut, mais ce saut n'est pas nécessaire — le contrôle automatique est
plus fiable que la relecture, ce qui est précisément sa raison d'être.

**Profondeur — 27 types de tuile posés moins de trois fois**, dont 26 une seule fois dans toute la
séquence : `danger`, `pressurePlate`, `block`, `slopeUpLeft`, `roundedUpRight`, `roundedUpLeft`,
`blockHalf`, `blockQuarter`, les quatre variantes de plafond, les quatre concaves, les quatre
dangers directionnels, `dangerMover`, `dangerSwitched`, `dangerBlink`, `key`, `lockedDoor`,
`movingPlatform` ; `slopeUpRight` deux fois. Seuls `switch`, `door`, `solid`, `entry` et `exit`
passent le seuil.

**Variantes non employées** : aucun `dashBudget`, aucune zone de caméra (`cameraFraming.zones`),
aucune taille de salle ou de suivi choisie par un niveau (`roomWidthTiles`/`roomHeightTiles`).

**Proximité — 13 tuiles de mécanique hors d'atteinte du personnage** : 4 dans `demo-plafond`
(toutes ses variantes de plafond incliné), 5 dans `demo-dangers-avances` (les quatre dangers **et**
l'interrupteur du danger commuté), 2 dans `demo-concave` (ses deux concaves de plafond), 2 dans
`demo-dangers-directionnels`.

## Définition de fait (DoD)
- La doctrine est écrite dans la spécification ; quatre contrôles automatiques échouent tant qu'un
  tableau est franchissable sans jouer, qu'une mécanique n'est posée qu'une fois, qu'un budget de
  dash n'est employé nulle part ou qu'une variante de cadrage manque ; chacun a son test négatif ;
  `/W4 /WX` propre.

## Exigences
`EX-LVL-012` (difficulté croissante, une mécanique à la fois puis synthèses), `EX-LVL-015`
(couverture vérifiée automatiquement) ; réutilise `EX-GP-024` (budget de mouvements), `EX-LVL-006`
et `EX-LVL-007` (cadrage), `EX-REN-017` (taille de suivi), `EX-NFR-021` (test système).
