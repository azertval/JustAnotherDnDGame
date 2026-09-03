# TACHE-02 — Clé ramassable et porte verrouillée {#lot-63-tache-02-cle-porte-verrouillee}

**Lot :** [LOT-63](epic.md) · **Emplacement :** `Source/Core/Levels`, `Source/Core/Gameplay` ·
**Statut :** fait

## Contexte
`EX-GP-023` — « Une **clé** collectée doit ouvrir une **porte verrouillée** correspondante » — porte
la mention « ⚠️ optionnel MVP » depuis la rédaction des spécifications de gameplay. Aucun
`core::TileType` ne représente ni la clé ni la porte verrouillée, et `core::MechanismController` ne
connaît que l'interrupteur à bascule et la plaque de pression.

C'est pourtant le mécanisme de puzzle le plus courant du genre, et celui qui donne le plus de marge
au level designer : contrairement à l'interrupteur, il crée une **dépendance d'ordre** entre deux
endroits du tableau.

## Travail à réaliser
- **Deux nouveaux `core::TileType`** : `Key` et `LockedDoor`, avec leurs noms sérialisés
  (`TileTypeName`) et leur place dans la validation (`EX-LVL-004`).
- **Liaison clé ↔ porte** par identifiant, exactement comme la liaison interrupteur ↔ porte
  existante — mêmes champs de format, même validation, aucune nouvelle notion de liaison.
- **Contrôleur** : la clé est **consommée** au ramassage, la porte correspondante s'ouvre
  **définitivement** (elle ne se referme pas ; une porte qui se referme, c'est l'interrupteur).
  Plusieurs paires indépendantes coexistent dans un même tableau.
- **Mode de ramassage** : au contact, ou par l'action « Interagir » de la `TACHE-01` — trancher au
  cadrage de l'implémentation et le documenter. Le contact est cohérent avec le reste du jeu ;
  l'action donne son premier usage à la `TACHE-01`.
  **Décidé : contact ET « Interagir »**, les deux à la fois (`core::MechanismController::update`,
  contrairement à l'interrupteur qui n'exige que le contact) — c'est ce qui donne à l'action son
  premier usage réel (`EX-CTRL-022`) plutôt qu'un doublon du contact déjà utilisé par les
  interrupteurs/plaques.
- **État réinitialisé** au (re)chargement du niveau, comme le budget de mouvements (`EX-GP-024`) :
  mourir après avoir pris la clé doit remettre la clé en place.
- **Grille de collision** : une porte verrouillée est **solide** tant qu'elle est fermée, et cesse
  de l'être une fois ouverte — via le même mécanisme que la porte existante
  (`MechanismController::collisionMap`), sans nouveau chemin.

## Fichiers impactés
- `Source/Core/Levels/TileType.h`, `TileTypeName.{h,cpp}`, `LevelLoader.cpp`, `LevelWriter.cpp`.
- `Source/Core/Gameplay/MechanismController.{h,cpp}`.
- `Source/Test/Unit/Core/Levels/test_tile_type_name.cpp`, `test_level_loader.cpp`,
  `test_level_writer.cpp` (étendus).
- `Source/Test/Unit/Core/Gameplay/test_mechanism_controller.cpp` (étendu).

## Tests (obligatoires)
- Ramasser la clé ouvre la porte liée, et **elle seule** : deux paires dans un tableau ne
  s'influencent pas.
- La porte ouverte **le reste** : aucun événement ne la referme.
- Une porte verrouillée fermée est **solide** ; ouverte, elle ne l'est plus — vérifié sur la grille
  de collision, pas seulement sur l'état logique.
- Le rechargement du niveau remet clé et porte dans leur état initial.
- Aller-retour de sérialisation : un niveau contenant clés et portes verrouillées s'écrit, se relit
  et donne exactement le même modèle.
- Validation : une clé sans porte liée, une porte sans clé, un identifiant en double — chacun est
  refusé avec une erreur exploitable.
- Tests `Core` purs, sans GPU.

## Points d'attention
- **Ne pas dupliquer la notion de liaison.** Le format en a déjà une, validée et testée ; en ajouter
  une seconde pour les clés créerait deux chemins de validation qui divergeront.
- Le **round-trip** de l'éditeur est le point de rupture habituel des nouveaux types de tuile :
  `LevelWriter` et `LevelLoader` doivent être étendus ensemble, et le test qui les confronte est
  celui qui attrape l'oubli.
- L'ombre du plan physique (`LOT-55`) traite déjà la porte comme un cas particulier : son type ne
  dit pas sa solidité, et `composeShadows` consulte la grille de collision courante. La porte
  verrouillée doit être traitée **de la même façon**, sinon elle projette une ombre après ouverture.
- Une porte verrouillée doit-elle apparaître dans le budget de mouvements ou les conditions de fin ?
  Non — mais le vérifier plutôt que le supposer.

## Définition de fait (DoD)
- Clé et porte verrouillée existent comme types de tuile sérialisables et validés, liés par le
  mécanisme de liaison existant, avec ouverture définitive, solidité cohérente, réinitialisation au
  rechargement et paires indépendantes ; couverts par des tests `Core` purs ; `/W4 /WX` propre.

## Exigences
`EX-GP-023` (clé et porte verrouillée — levée ici) ; réutilise `EX-GP-021` (porte),
`EX-GP-024` (réinitialisation au chargement), `EX-CTRL-022` (action « Interagir », si retenue),
`EX-LVL-002`/`EX-LVL-004` (format et validation), `EX-NFR-002` (déterminisme), `EX-NFR-020`
(tests unitaires `Core`).
