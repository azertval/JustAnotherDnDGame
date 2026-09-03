# TACHE-03 — Non-régression du mode Physique et documentation {#lot-41-tache-03-non-regression-documentation}

**Lot :** [LOT-41](epic.md) · **Emplacement :** `Source/Test`, `Documentation` · **Statut :** fait

## Contexte
Le mode Physique est la **référence** de tout le programme : c'est lui qui donne la lecture directe
des collisions, et c'est à lui que le mode Texture sera comparé pendant treize lots. Il doit donc
être verrouillé par un test, pas par une impression visuelle.

Le cadrage initial exigeait un rendu « identique bit à bit », vérifié à l'œil — une formulation
invérifiable. Le *QuadRecorder* livré en LOT-40 permet de la remplacer par une assertion.

## Travail à réaliser
- **Test de référence** : pour une scène couvrant les cas représentatifs (tuiles solides, pentes,
  arrondis, dangers directionnels, mécanismes, blocs réduits, personnage), capturer les primitives
  composées en mode `Physique` et vérifier qu'elles correspondent à la référence établie avant le
  lot — mêmes régions d'atlas, mêmes positions, mêmes calques, même ordre.
- **Test de bascule** : le passage `Physique` → `Texture` → `Physique` restitue exactement la
  première liste (la bascule n'a aucun effet rémanent).
- **Documentation** :
  - `Documentation/Guide/guide-rendu.md` — décrire les deux modes, la touche `F8`, le défaut et la
    persistance ; mettre à jour la section d'orientation vers le programme.
  - `Documentation/Manuel/jouer.md` — mentionner `F8` côté joueur.
  - `Documentation/Manuel/partager-un-niveau.md` — mentionner `F8` côté éditeur, à côté de `F10`.

## Fichiers impactés
- `Source/Test/Unit/HMI/Graphics/test_render_mode.cpp` (nouveau).
- `Documentation/Guide/guide-rendu.md`, `Documentation/Manuel/jouer.md`,
  `Documentation/Manuel/partager-un-niveau.md`.

## Tests (obligatoires)
Ce sont l'objet même de la tâche (voir ci-dessus). Le test de référence doit rester en place pour
tous les lots suivants : c'est le filet qui garantit qu'aucun lot d'habillage ne dégrade la lecture
du physique.

## Points d'attention
- Le manuel utilisateur s'adresse à un non-développeur : décrire `F8` par ce qu'il fait
  (« voir le niveau tel qu'il est réellement construit, sans habillage »), pas par son
  implémentation.
- Ne pas documenter le mode Texture comme « le rendu final » tant que LOT-42 n'est pas livré : à ce
  stade, il affiche légitimement un damier partout.

## Définition de fait (DoD)
- Le mode Physique est verrouillé par un test de référence ; la bascule est sans effet rémanent ;
  guide et manuel à jour ; Doxygen et lint verts.

## Exigences
`EX-REN-046` (bascule) ; réutilise `EX-NFR-004` (rendu vérifiable sans GPU), `EX-NFR-030` (doc de
build et d'usage à jour).
