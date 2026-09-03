# TACHE-06 — Croisements de mécaniques et corrections moteur associées {#lot-65-tache-06-croisements-mecaniques}

**Lot :** [LOT-65](epic.md) · **Emplacement :** `Source/Core/Gameplay`, `Source/Test/Integration`, `Documentation/Specification` · **Statut :** en cours

## Contexte
La revue qui a motivé la `TACHE-05` a mis au jour un trou plus large que la couverture des
mécaniques **isolées** : la suite de tests couvre chaque mécanique **seule** de façon dense
(`test_physique_personnage.cpp` en compte 75) et presque **aucun croisement**.

Deux situations distinctes, à ne pas confondre :

**Testé unitairement, jamais joué** — le tableau serait la première mise en situation réelle :
bloc porté par une plateforme (`BlocPousseSurPlateformeEstPorte`), `dangerSwitched` piloté par une
**plaque** (`DangerCommuteActivationContinuePlaqueDePression`, donc mortel tant qu'on reste dessus),
`dashBudget` (`BudgetDeDashsRefuseAuDela`), plateforme **verticale**.

**Testé nulle part** — ni unitaire, ni intégration, ni tableau. Dix croisements, dont deux qui
appellent une décision de conception avant tout contenu :

1. **Écrasement par une porte qui se referme.** `core::Player::squished` n'est posé que par
   l'écrasement contre une **plateforme mobile** ; rien n'existe pour une porte. Or le
   `demo-plaque-pression` livré **crée cette situation à chaque partie** : le personnage quitte la
   plaque en sautant, et la porte se referme pendant qu'il la traverse. Le tableau repose donc sur
   un comportement que personne n'a spécifié ni testé, avec un risque de blocage définitif que
   `niveaux.md` § Conception interdit explicitement.
2. **Un bloc ne peut pas presser une plaque de pression.**
   `core::MechanismController::update(playerBox, playerMass, interactPressed)` ne reçoit que la
   boîte du **joueur**, alors que `EX-GP-025` parle d'un « poids suffisant » et que le contrôleur
   porte déjà un seuil `MIN_TRIGGER_MASS`. L'idiome de puzzle le plus classique du genre — poser un
   poids pour tenir une porte ouverte et partir — est hors d'atteinte, ce qui appauvrit sévèrement
   les tableaux de synthèse à venir.

Ces deux points sortent du cadrage « ce lot n'ajoute rien au moteur ». C'est un assouplissement
**délibéré et borné** : sans eux, ni l'énigme du tableau final ni la refonte de
`demo-plaque-pression` ne tiennent debout. Les deux corrections sont livrées **avant** tout contenu,
sinon les tableaux s'appuieraient de nouveau sur un comportement accidentel.

## Travail à réaliser

### Corrections moteur
- **Porte écrasante mortelle.** Quand une porte redevient solide sur la case occupée par le
  personnage, poser `core::Player::squished`. Toute l'infrastructure aval existe déjà :
  `hmi::GameSession` et `test_parcours_complet.cpp` traduisent tous deux `squished` en boîte de
  danger supplémentaire pour `core::evaluateOutcome`. Déclarer l'exigence côté `gameplay.md`.
- **Bloc activant une plaque.** `core::MechanismController::update` accepte les boîtes des blocs en
  plus de celle du joueur, chacune avec sa masse comparée au seuil `MIN_TRIGGER_MASS` existant. Les
  appelants disposent déjà des boîtes via `core::BlockController::boxAt`. Actualiser `EX-GP-025`,
  dont la formule « poids suffisant » cesse d'être sans objet.

### Batterie de tests de croisement
Un module d'intégration dédié, **indépendant du contenu** (géométrie construite en mémoire, pas de
fichier de niveau) :

| Croisement | Ce qu'il établit |
|---|---|
| Dash × bloc poussable, plein et réduit | `pushBlocks` exige un contact à `PUSH_TOUCH_TOLERANCE` (0,05) évalué sur la boîte du pas **précédent**, alors qu'un dash déplace 0,25 case/pas — cinq fois la tolérance. |
| Dash × pente, arrondi, concave, plafond incliné | `resolveSlopeFollow` et `resolveCeilingSlopeFollow` ne sont éprouvés qu'à vitesse de marche (0,05 case/pas). |
| Dash × plaque de pression | La plaque s'évalue par chevauchement au pas courant ; un dash la survole en deux pas. |
| Bloc poussé dans une porte qui se referme | Le `BlockController` reçoit `mechanisms.collisionMap()` : le cas est atteignable. |
| Bloc poussé sur une tuile de danger | Le bloc masque-t-il le danger, ou reste-t-il mortel dessous ? |
| Wall jump depuis une plateforme mobile | `wallDirection` contre une paroi pendant que le sol se déplace. |
| Plateforme × porte, plateforme × danger | Jamais combinés. |
| `DangerMover` × bloc, × plateforme | Aucune notion de collision entre eux — à constater et à figer. |
| Deux déclencheurs (interrupteur **et** plaque) sur la même porte | Non testé ; la validation du chargeur n'en dit rien. |

## Fichiers impactés
- `Source/Core/Gameplay/MechanismController.{h,cpp}` — écrasement par porte, boîtes des blocs.
- `Source/HMI/Game/GameSession.cpp` et `Source/Test/Systeme/test_parcours_complet.cpp` — les deux
  appellent `mechanisms.update` et doivent lui transmettre les boîtes des blocs.
- `Source/Test/Integration/test_croisements_mecaniques.cpp` (nouveau), `Source/Test/CMakeLists.txt`.
- `Source/Test/Unit/Core/Gameplay/test_mechanism_controller.cpp` — bloc sur plaque, porte écrasante.
- `Documentation/Specification/gameplay.md` — `EX-GP-021`, `EX-GP-025`.

## Tests (obligatoires)
- Chaque croisement du tableau ci-dessus a **son** test, avec son bloc `\castest{}` écrit en même
  temps que lui.
- Une porte qui se referme sur le personnage produit `Lost`, jamais un personnage figé dans un mur.
- Un bloc posé sur une plaque tient la porte ouverte **après** le départ du personnage, et la
  referme si le bloc repart.
- Un bloc de masse insuffisante ne déclenche pas la plaque (symétrique de
  `PlaqueDePressionPoidsInsuffisant`).
- `/W4 /WX` propre ; tests `Core` sans GPU.

## Points d'attention
- **Ne pas élargir la correction au-delà du besoin.** L'objectif n'est pas de généraliser les
  déclencheurs à toute entité, mais d'ajouter les blocs, dont la masse est déjà une donnée.
- **Le défaut plateforme + pente reste consigné, pas corrigé** : la seule présence d'un
  `movingPlatform` dans un fichier casse la résolution de collision pendant le suivi d'une pente
  ailleurs dans ce même niveau. Les tests de croisement plateforme évitent donc toute pente, et
  aucun tableau ne combinera les deux.
- **Un test de croisement qui passe du premier coup n'est pas une perte** : il fige un comportement
  jusque-là non spécifié. Un test qui échoue révèle un défaut à consigner — pas nécessairement à
  corriger dans ce lot.
- L'ordre de résolution dans le pas fixe (plateformes, puis blocs, puis physique, puis mécanismes)
  est déjà documenté et reproduit à l'identique par le test système : ne pas le réinventer.

## Résultat constaté
Douze croisements livrés ; **onze figent un comportement correct**, **un révèle un défaut**.

**Défaut trouvé — une pente franchissable à la marche ne l'est pas au dash.** Sur la silhouette
exacte des tableaux de pente livrés (pente à 45° suivie d'un palier plein), un personnage qui dashe
se fige au sommet : il bute contre la colonne pleine du palier avec les pieds encore 0,15 case trop
bas. Le test le **caractérise** plutôt que de le corriger — conformément au cadrage du lot — et
vérifie d'abord que la même géométrie se franchit à la marche, faute de quoi on ne saurait pas s'il
mesure un défaut ou une géométrie mal choisie. Consigné au `CHANGELOG`. **Conséquence pour
`TACHE-07`/`TACHE-08` : aucun tableau ne rend un dash obligatoire dans une montée de pente vers un
palier.**

**Comportements figés au passage**, jusque-là non spécifiés : un dash n'est jamais arrêté par une
plaque (elle s'active malgré la brièveté du contact) ; un bloc poussé sur un danger ne le neutralise
pas ; une plateforme mobile n'est jamais retenue par une porte fermée (sa position est une fonction
du numéro de pas, pas le résultat d'une collision) ; deux déclencheurs liés à une même porte
coexistent sans que le contrôleur ne se contredise.

**Les deux corrections moteur sont validées de bout en bout** : un bloc posé sur une plaque tient la
porte ouverte **après** le départ du personnage, un bloc réduit reste trop léger pour l'enfoncer, et
une porte qui se referme sur le personnage provoque l'échec au lieu de l'encastrer.

## Définition de fait (DoD)
- Les deux corrections moteur sont livrées, spécifiées et testées ; la batterie de croisements
  couvre les dix combinaisons listées ; chaque test porte son `\castest{}` ; tout défaut découvert
  et non corrigé est consigné au `CHANGELOG` avec sa reproduction ; `/W4 /WX` propre, `ctest` vert.

## Exigences
`EX-GP-021` (porte liée à un interrupteur — écrasement), `EX-GP-025` (plaque de pression, poids
suffisant) ; réutilise `EX-GP-022` (bloc poussable), `EX-GP-026` (plateforme mobile), `EX-GP-017`
(dash), `EX-GP-003`/`EX-GP-004`/`EX-GP-006`/`EX-GP-007` (pentes et arrondis), `EX-NFR-020` (tests),
`EX-NFR-010` (`Core` sans GPU).
