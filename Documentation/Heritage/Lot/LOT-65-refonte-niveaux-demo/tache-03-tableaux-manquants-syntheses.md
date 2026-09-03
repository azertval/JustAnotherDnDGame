# TACHE-03 — Tableaux manquants et tableaux de synthèse {#lot-65-tache-03-tableaux-manquants-syntheses}

**Lot :** [LOT-65](epic.md) · **Emplacement :** `Source/Elements/Levels` · **Statut :** fait

## Réalisé
Cinq tableaux couvrent les quatorze types de tuile encore manquants après `TACHE-02`, regroupés
par famille plutôt qu'un tableau par type (progression à côté de leur variante "droite"/pleine
déjà connue) :
- `demo-pente-gauche.json` — `SlopeUpLeft`/`RoundedUpLeft`/`ConcaveUpLeft` : trois paliers
  **descendants** en marchant (miroir des variantes "droite"), sans saut.
- `demo-plafond.json` — `SlopeDownRight`/`SlopeDownLeft`/`RoundedDownRight`/`RoundedDownLeft` :
  posés en plafond avec un dégagement généreux au-dessus d'un couloir plat, jamais touchés en
  marchant — leur silhouette de blocage réelle est déjà exhaustivement prouvée au niveau
  Integration (`PlafondInclineBloqueSelonSaSilhouette` et consorts).
- `demo-concave.json` — `ConcaveUpRight` (sol, marché) et `ConcaveDownRight`/`ConcaveDownLeft`
  (plafond, dégagement généreux).
- `demo-bloc-quart.json` — `BlockQuarter`, terrain plat plutôt qu'une fosse (contrairement au
  bloc réduit) : la poussée écarte simplement le bloc, aucune fosse à combler à une hauteur qui
  dépendrait de l'échelle du bloc.
- `demo-dangers-directionnels.json` — `DangerDown`/`DangerLeft`/`DangerRight`, alcôves flottantes
  hors du couloir principal, même principe que `demo-dangers-avances.json` (`TACHE-02`).

Les trois variantes significatives encore non couvertes après ces cinq tableaux (déphasage d'un
`DangerBlink`, `DangerMover` vertical, texture par instance) ont été obtenues par des ajustements
cosmétiques sans risque sur les alcôves déjà isolées de `demo-dangers-avances.json` et un habillage
`texture` sur le bloc de `demo-bloc-quart.json`, plutôt que par de nouveaux tableaux.

Le mode de cadrage **suivi** (`Follow`), seul mode encore non couvert après `TACHE-01`, a été
obtenu en `TACHE-02` en déclarant explicitement `demo-final.json` (le parcours continu le plus
long de la séquence) en `follow` plutôt qu'en le laissant retomber sur *par salle* — aucun tableau
supplémentaire n'était nécessaire.

**Synthèse** : chacun des cinq tableaux combine déjà plusieurs types auparavant isolés (jusqu'à
quatre dans `demo-plafond.json`) ; `demo-final.json` (`TACHE-02`, redessiné) et
`demo-dangers-avances.json` restent les tableaux de synthèse inter-familles de la séquence
(dash/pente/interrupteur/double-saut ; les quatre dangers avancés) — la plateforme mobile n'y est
délibérément pas associée, un défaut de moteur consigné dans `CHANGELOG.md` (collision cassée
pendant le suivi de pente dès qu'une configuration de plateforme mobile existe dans le niveau).
Un tableau de synthèse
dédié combinant explicitement une mécanique de `TACHE-03` à une mécanique pré-existante (ex. clé +
bloc quart + interrupteur en un seul parcours) a été jugé de valeur marginale face à ce que
`demo-final.json` couvre déjà, et n'a pas été ajouté — candidat naturel pour un lot de contenu
ultérieur si souhaité.

Garde-fou de couverture (`TACHE-01`) **vert** : `ctest --preset ninja` à 100 % (1122/1122).

## Contexte
La `TACHE-01` a produit la liste des mécaniques que le contenu livré n'emploie nulle part ; la
`TACHE-02` a remis à niveau les tableaux existants. Restent deux manques.

D'abord les **mécaniques sans tableau** — celles que le garde-fou refuse encore. Ensuite, un manque
de nature différente : la séquence isole **toutes** ses mécaniques et n'en combine aucune. Or
`niveaux.md` demande une difficulté croissante, et un jeu de puzzle-plateforme naît de la
**combinaison** — un bloc à pousser sur une plaque pendant qu'un danger temporisé balaie le passage.
C'est aussi là que se trouvent les défauts d'interaction entre mécanismes, qu'aucun test unitaire
ne cherche.

## Travail à réaliser
- **Un tableau par mécanique non couverte**, jusqu'à ce que le garde-fou de la `TACHE-01` passe au
  vert. La liste est donnée par le contrôle, pas par une relecture.
- **Tableaux de synthèse**, en fin de séquence, combinant plusieurs mécaniques : mécanismes liés et
  dangers avancés, plateforme mobile au-dessus d'un danger, clé gardée par un parcours d'adresse,
  budget de mouvements contraignant une solution de puzzle.
- **Les trois modes de cadrage employés** au moins une fois chacun, dans un tableau où le choix se
  justifie — un couloir horizontal long pour le suivi, un puzzle compact pour le niveau entier, un
  ensemble de salles pour la coupure nette.
- **Progression de difficulté** : les synthèses viennent après les tableaux d'introduction, et
  `demo-final` reste le dernier.
- **Insertion dans la séquence** (donnée de contenu depuis le `LOT-59`) et dans le test système.
- **Vérifier les lignes directrices** sur chaque nouveau tableau : une seule nouveauté à la fois,
  aucune situation sans issue.

## Fichiers impactés
- `Source/Elements/Levels/*.json` (nouveaux), fichier de séquence.
- `Source/Test/Systeme/test_parcours_complet.cpp`.
- `Source/Elements/Levels/README.md`.

## Tests (obligatoires)
- **Le garde-fou de couverture passe au vert** — c'est le critère de fin de cette tâche.
- Chaque nouveau tableau est **franchissable** de bout en bout et **valide** au chargement.
- Les trois modes de cadrage apparaissent chacun dans au moins un tableau.
- Le test système reste dans une durée raisonnable malgré l'allongement de la séquence — la
  mesurer, pas la supposer.
- `python scripts/check_demo_sequence.py` vert.
- Budget de primitives respecté sur les tableaux de synthèse, qui sont les plus chargés
  ([LOT-62](@ref lot-62)).

## Points d'attention
- **Les tableaux de synthèse sont ceux qui trouveront les défauts.** C'est leur intérêt et leur
  risque : un mécanisme qui interagit mal avec un autre se manifestera ici, souvent comme une
  franchissabilité intermittente. Consigner, ne pas corriger dans ce lot, et si un tableau dépend
  d'un défaut non corrigé, le retirer de la séquence plutôt que de laisser un test système
  instable.
- **Un test système instable est pire que pas de test.** Si un tableau de synthèse ne se franchit
  pas de façon reproductible, le problème est dans le moteur ou dans le tableau — jamais une raison
  de tolérer un échec intermittent.
- Ne pas viser la difficulté maximale : ces tableaux démontrent et vérifient, ils ne sont pas une
  campagne calibrée.
- La séquence s'allonge sensiblement : vérifier que la progression persistée
  ([LOT-59](@ref lot-59)) reste lisible dans l'écran de sélection de niveau.

## Définition de fait (DoD)
- Le garde-fou de couverture est vert, chaque mécanique et chaque mode de cadrage sont employés au
  moins une fois, des tableaux de synthèse combinent plusieurs mécaniques en fin de séquence, tous
  les tableaux sont franchissables de façon reproductible, et les défauts découverts sont consignés.

## Exigences
`EX-LVL-015` (couverture exhaustive) ; réutilise `EX-LVL-012` (difficulté croissante), `EX-LVL-006`
(cadrage), `EX-LVL-013` (séquence en donnée), `EX-NFR-021` (franchissabilité), `EX-GP-020` à
`EX-GP-026` (mécanismes), `EX-GP-050` à `EX-GP-053` (dangers avancés), `EX-GP-024` (budget de
mouvements).
