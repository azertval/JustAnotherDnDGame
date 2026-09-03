# LOT-25 — Refactoring complet des niveaux démo {#lot-25}

> Statut : **terminé**. Les niveaux `demo.json`…`demo5.json` ont grandi **un par un, au fil des
> lots**, chacun ajouté pour exercer la mécanique du moment — sans repasse d'ensemble depuis. Plutôt
> que de les compléter au cas par cas, ce lot **vide entièrement** le dossier de niveaux démo et
> repart d'une base neuve, conçue dès le départ pour être **modulaire** (un niveau par mécanique,
> nommé et organisé en conséquence) et couvrir **systématiquement** toutes les mécaniques livrées à
> ce jour, y compris celles des lots `LOT-22`/`LOT-23`/`LOT-24` (pentes, arrondi, blocs à taille
> fractionnaire). **Nombre de niveaux non fixé** : décidé en `TACHE-01`, à partir d'un inventaire
> réel des mécaniques à couvrir, pas d'un chiffre choisi à l'avance.

## Objectif
Constat déclencheur : le test système `test_parcours_complet.cpp` (`ParcoursCompletSysteme.
FranchitTouteLaSequence`) ne rejoue que `demo.json` à `demo4.json` — `demo5.json` (plaque de
pression, `LOT-19`) n'y figure **pas**, alors qu'il est bien chargé par le jeu (`Source/HMI/
main.cpp`). Plus largement, aucun niveau démo n'exerce le double saut, le wall jump/wall slide, ni
les budgets de sauts/dashs (`EX-GP-024`) de façon isolée et identifiable — ils sont vérifiés par
les tests **unitaires**/**intégration** (`test_physique_personnage.cpp`), jamais par un niveau
**jouable**. Ce lot répare cet écart et anticipe les mécaniques des lots `LOT-22` à `LOT-24`, en
repartant d'une base **vide** plutôt que de continuer à empiler sur l'existant.

## Périmètre

### Inclus
- **Inventaire exhaustif** des mécaniques livrées (mouvement, saut simple/variable, coyote time,
  jump buffering, double saut, wall jump/wall slide, dash 8 directions, gravité asymétrique/apex/
  fast-fall, chute newtonienne, interrupteur↔porte, plaque de pression, bloc poussable, budgets de
  sauts/dashs, **pentes**, **arrondi**, **blocs à taille fractionnaire**) et des mécaniques
  **transverses** non spécifiques à un niveau (manette, menu d'options, localisation — non
  couvertes par des niveaux, hors périmètre de ce lot).
- **Suppression de tous les niveaux démo existants** (`demo.json`…`demo5.json`) : base **vide**,
  reconstruite entièrement à partir de l'inventaire de `TACHE-01`, plutôt qu'un mélange de niveaux
  conservés/complétés/ajoutés.
- **Refonte du jeu de niveaux démo** : un ensemble de niveaux, tous nouveaux, où chaque mécanique
  (ou petit groupe de mécaniques apparentées) a **au moins un niveau dédié** qui la rend nécessaire
  pour franchir la sortie — pas seulement présente en décor.
- **Niveau final combiné** : la séquence se termine par un dernier niveau qui **mélange** un
  maximum des mécaniques précédentes (pas une simple répétition isolée) — un test d'intégration
  jouable, complémentaire aux niveaux isolés qui le précèdent.
- **Mise à jour de la séquence** jouée par le jeu (`Source/HMI/main.cpp`) et du test système
  (`test_parcours_complet.cpp`), pour que les deux restent des reflets fidèles l'un de l'autre.

### Exclus (hors périmètre de ce lot)
- **Nouvelles mécaniques** — ce lot **teste** l'existant (et ce que `LOT-22`/`LOT-23`/`LOT-24`
  livreront), n'en ajoute aucune.
- **Refonte visuelle/artistique** au-delà de l'atlas procédural déjà en place.
- **Mécaniques transverses non liées à un niveau** (manette, menu d'options) — déjà couvertes par
  leurs propres tests unitaires (`LOT-20`), sans lien avec la séquence de niveaux.

## Décisions de cadrage
- **Ce lot est séquencé en dernier**, après `LOT-22`/`LOT-23`/`LOT-24` : il doit pouvoir exercer
  leurs mécaniques, qui doivent donc déjà exister. Le numéro `LOT-25` reflète déjà cet ordre.
- **Le nombre de niveaux n'est pas fixé a priori.** `TACHE-01` produit un tableau
  mécanique → niveau(x) avant toute création de fichier ; le nombre qui en résulte est documenté
  dans cette tâche, pas deviné ici.
- **Base vide plutôt que complétion au cas par cas.** Les niveaux existants ont grandi de façon
  organique (un ajout par lot, jamais repensés ensemble) ; les vider entièrement et repartir d'un
  inventaire complet (`TACHE-01`) évite d'hériter de leurs compromis successifs et donne, dès le
  départ, une organisation **modulaire** (un fichier par mécanique, nommé en conséquence — voir
  `TACHE-02`) plutôt qu'une numérotation `demoN.json` qui ne dit rien du contenu.
- **Un niveau démo par mécanique (ou petit groupe cohérent), plus un niveau final combiné — pas un
  « niveau vitrine » unique en remplacement des niveaux isolés.** Les niveaux isolés restent la
  référence pour déboguer (un niveau qui échoue pointe directement vers la mécanique en cause) et
  maintenir (ajouter une mécanique = ajouter un niveau, jamais retoucher les autres) ; le niveau
  final combiné s'y **ajoute**, en dernier, pour vérifier que les mécaniques cohabitent
  correctement une fois combinées (ex. un dash au-dessus d'une pente, un bloc poussé sur une plaque
  de pression) — un risque que des niveaux strictement isolés ne peuvent pas révéler.
- **`main.cpp` et `test_parcours_complet.cpp` doivent lister exactement les mêmes fichiers, dans le
  même ordre.** Le décalage constaté avec `demo5.json` (chargé en jeu, absent du test système) ne
  doit plus pouvoir se reproduire silencieusement — envisager en `TACHE-03` un test ou un
  script qui compare les deux listes plutôt que de compter sur la vigilance humaine.

## Découpage

> État : ✅ fait · 🔄 en cours · ⬜ non commencé.

| Tâche | Intitulé | Emplacement | État |
|-------|----------|-------------|:----:|
| [TACHE-01](tache-01-inventaire-conception.md) | Inventaire des mécaniques et conception | `Documentation` | ✅ |
| [TACHE-02](tache-02-implementation-niveaux.md) | Implémentation des niveaux | `Source/Elements/Levels` | ✅ |
| [TACHE-03](tache-03-integration-sequence-tests.md) | Intégration séquence et tests système | `HMI/main.cpp`, `Test/Systeme` | ✅ |
| [TACHE-04](tache-04-documentation-verification.md) | Documentation et vérification | `Documentation` | ✅ |

## Critères d'acceptation du lot
1. Chaque mécanique de l'inventaire (`TACHE-01`) a au moins un niveau démo où elle est
   **nécessaire** pour atteindre la sortie (vérifiable en désactivant mentalement la mécanique :
   le niveau devient infranchissable).
2. Le **dernier** niveau de la séquence combine un maximum de mécaniques de l'inventaire (pas une
   simple répétition d'un niveau isolé) et reste franchissable.
3. `Source/HMI/main.cpp` et `test_parcours_complet.cpp` chargent **exactement** la même liste de
   niveaux, dans le même ordre — plus aucun niveau chargé en jeu mais absent du test système (ou
   l'inverse).
4. Le test système franchit tous les niveaux de la séquence dans l'ordre (`Won` partout), niveau
   final combiné compris.
5. **Vérification visuelle** de la séquence complète dans l'application compilée, en plus du test
   système automatisé — **substituée** par le test système `ParcoursCompletSysteme.
   FranchitTouteLaSequence` (voir `TACHE-04` pour la justification : session interactive hors
   périmètre de cette conversation).
6. Build `/W4 /WX` sans avertissement, Doxygen et lint des exigences verts.

## Dépendances
- **Dépend de `LOT-22`, `LOT-23` et `LOT-24`** (doit exercer leurs mécaniques) — à ne pas commencer
  avant que les trois soient terminés.
- Modifie `Source/HMI/main.cpp` (LOT-01 et suivants), `Source/Test/Systeme/test_parcours_complet.cpp`
  (`LOT-13`), et le contenu de `Source/Elements/Levels/` (tous les lots de gameplay).

## Navigation des tâches
- @subpage lot-25-tache-01-inventaire-conception
- @subpage lot-25-tache-02-implementation-niveaux
- @subpage lot-25-tache-03-integration-sequence-tests
- @subpage lot-25-tache-04-documentation-verification
