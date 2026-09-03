# LOT-63 — Mécanismes manquants du référentiel {#lot-63}

> Statut : **fait**. Prérequis : [LOT-12](@ref lot-12) (mécanismes), [LOT-21](@ref lot-21)
> (blocs poussables), [LOT-31](@ref lot-31) (dangers avancés). Cinq tâches faites et vérifiées
> localement (build `/W4 /WX` Debug et Release, `ctest` à 100 % dans les deux configurations,
> Doxygen sur la version épinglée par la CI, `lint_exigences.py`/`check_demo_sequence.py`/
> `generate_cahier_test.py --check` verts). Essai manuel réel (jouer les trois niveaux à la
> manette/au clavier) réservé à l'utilisateur.

## Objectif
Livrer les trois mécanismes que les spécifications annoncent depuis le début et que le moteur n'a
jamais eus.

- **Clé et porte verrouillée** — `EX-GP-023`, marquée « ⚠️ optionnel MVP ». Aucun `core::TileType`
  ne les représente.
- **Action « Interagir »** — le tableau des contrôles de `controles.md` la liste avec ses touches
  par défaut (**E** au clavier, **X** à la manette) et la mention « ⚠️ souhaité, non implémenté ».
  Elle n'a même pas d'identifiant d'exigence. Son absence a une conséquence directe : un
  interrupteur ne s'actionne aujourd'hui que par **contact**, ce que `EX-GP-020` autorise
  (« contact **ou** action dédiée ») mais qui interdit tout mécanisme demandant une intention.
- **Plateforme mobile** — `vision.md` la range parmi la boîte à outils du genre, « (plus tard) ».
  C'est l'élément le plus attendu d'un jeu de plateforme après le saut lui-même.

L'objectif est le même dans les trois cas : réduire l'écart entre ce que le référentiel promet et ce
que le jeu contient, avant de publier une version qui se présente comme un jeu.

## Périmètre

### Inclus
- **Action logique « Interagir »**, remappable clavier et manette, avec ses valeurs par défaut
  documentées.
- **Clé** ramassable et **porte verrouillée** : la porte s'ouvre définitivement à la présentation
  de la clé correspondante ; plusieurs paires clé/porte possibles dans un même tableau.
- **Plateforme mobile** : parcours entre deux points, à vitesse constante, portant le personnage et
  les blocs poussables.
- **Intégration éditeur complète** pour les trois : palette, taxonomie, liaison, validation,
  rendu de brouillon.
- **Habillage** : skins et animations selon l'état, via les catalogues existants.
- **Niveaux de démonstration** couvrant chaque mécanisme, ajoutés à la séquence.

### Exclus (hors périmètre de ce lot)
- Plateformes à trajectoire libre, à chemin multi-points, à vitesse variable, déclenchées par un
  mécanisme : un aller-retour entre deux points suffit à couvrir le besoin.
- Inventaire visible, clés multiples d'un même type, portes à plusieurs clés.
- Interaction avec les décors ou les objets non mécanismes.
- Refonte de `core::MechanismController` : les nouveaux mécanismes s'y ajoutent selon le patron
  établi.

## Décisions de cadrage
- **Trois mécanismes, un lot, parce qu'ils partagent le même chemin.** Chacun suit exactement la
  route établie par tous les lots de mécanismes : `TileType` → contrôleur testé dans `Core` →
  intégration éditeur (taxonomie, palette, validation) → skin et animation → niveau de démonstration
  → cahier de test. Les répartir en trois lots répéterait trois fois la même cérémonie.
- **Découpable par tâche.** C'est le seul lot du programme dont on peut ne livrer qu'une partie sans
  incohérence : chaque mécanisme est indépendant des deux autres. Si le calendrier se tend, on
  rogne ici — d'où sa place tardive dans l'ordre d'exécution.
- **La plateforme mobile est le vrai sujet.** Clé et interaction sont des variations de mécanismes
  existants ; une plateforme qui **porte** le personnage touche à la collision continue
  (`EX-GP-014`), au suivi de surface et à l'interpolation d'affichage. C'est la tâche à risque, et
  elle doit être traitée comme telle.
- **L'interaction ne remplace pas le contact.** `EX-GP-020` autorise les deux ; changer
  l'interrupteur existant casserait des niveaux livrés et le test système. L'action nouvelle
  s'ajoute, elle ne se substitue pas.
- **Déterminisme préservé.** La plateforme se déplace au pas fixe, sa position est fonction du
  numéro de pas, jamais du temps réel (`EX-NFR-002`).

## Exigences couvertes
- **Levée** : `EX-GP-023` (⚠️ optionnel MVP → livré) — clé et porte verrouillée.
- Nouvelles : `EX-CTRL-022` (action logique « Interagir », remappable), `EX-GP-026` (plateforme
  mobile déterministe portant le personnage et les blocs).
- Réutilisées : `EX-GP-020`/`EX-GP-021` (interrupteur, porte), `EX-GP-022` (blocs poussables),
  `EX-GP-014` (collisions résolues sans traversée), `EX-CTRL-010` (actions logiques),
  `EX-CTRL-012` (remappage), `EX-NFR-002` (déterminisme), `EX-LVL-002`/`EX-LVL-004` (format et
  validation), `EX-EDIT-010` (réutilisation du modèle de `Core`), `EX-REN-005` (animation par
  données), `EX-REN-006` (apparence d'un mécanisme selon son état).

## Découpage

> État : ✅ fait · 🔄 en cours · ⬜ non commencé.

| Tâche | Intitulé | Emplacement | État |
|-------|----------|-------------|:----:|
| [TACHE-01](tache-01-action-interagir.md) | Action logique « Interagir », clavier et manette | `Source/Core/Physics`, `Source/HMI/Input` | ✅ |
| [TACHE-02](tache-02-cle-porte-verrouillee.md) | Clé ramassable et porte verrouillée | `Source/Core/Levels`, `Source/Core/Gameplay` | ✅ |
| [TACHE-03](tache-03-plateforme-mobile.md) | Plateforme mobile déterministe, portant le personnage | `Source/Core/Gameplay`, `Source/Core/Physics` | ✅ |
| [TACHE-04](tache-04-integration-editeur-habillage.md) | Intégration éditeur et habillage des trois mécanismes | `Source/HMI/Editor`, `Source/HMI/Graphics` | ✅ |
| [TACHE-05](tache-05-niveaux-documentation-verification.md) | Niveaux de démonstration, documentation et vérification | `Source/Elements/Levels`, `Documentation` | ✅ |

## Critères d'acceptation du lot
1. Une clé ramassée ouvre définitivement la porte verrouillée correspondante ; deux paires
   coexistent sans interférence dans un même tableau.
2. L'action « Interagir » est remappable au clavier et à la manette, et n'altère en rien le
   comportement par contact des interrupteurs existants.
3. Une plateforme mobile porte le personnage et un bloc poussable sans traversée, à toute vitesse de
   plateforme admise, et sans tremblement à l'affichage.
4. Les trois mécanismes se posent, se lient et se valident dans l'éditeur, et sont habillés selon
   leur état.
5. La simulation reste **déterministe** : deux exécutions de la même séquence d'entrées donnent le
   même résultat, plateformes comprises.
6. Les niveaux livrés existants restent franchissables — test système inchangé.
7. Build `/W4 /WX`, `ctest` à 100 %, Doxygen et lints verts.

## Dépendances
Bâtit sur [LOT-12](@ref lot-12) (mécanismes et liaisons), [LOT-21](@ref lot-21) (blocs, portés par
les plateformes), [LOT-29](@ref lot-29)/[LOT-30](@ref lot-30) (remappage clavier et manette),
[LOT-46](@ref lot-46)/[LOT-47](@ref lot-47) (animation par état). Les niveaux ajoutés rejoignent la
séquence livrée par [LOT-59](@ref lot-59).

## Navigation des tâches
- @subpage lot-63-tache-01-action-interagir
- @subpage lot-63-tache-02-cle-porte-verrouillee
- @subpage lot-63-tache-03-plateforme-mobile
- @subpage lot-63-tache-04-integration-editeur-habillage
- @subpage lot-63-tache-05-niveaux-documentation-verification
