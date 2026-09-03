# TACHE-01 — Correspondance état logique → clip {#lot-47-tache-01-correspondance-etat-clip}

**Lot :** [LOT-47](epic.md) · **Emplacement :** `Source/HMI/Graphics` · **Statut :** fait

## Contexte
`Core` expose déjà tous les états nécessaires, calculés au pas fixe et testés :

- `core::MechanismController::isDoorOpen(index)`, `isDangerActive(position)`, l'état des
  interrupteurs et des plaques de pression ;
- `core::DangerController::isBlinkActive(index)` et `moverBox(index)`.

Ce qui manque n'est donc pas l'information, mais sa **traduction en apparence**. Cette traduction est
une décision de présentation : elle appartient à `HMI`, et `core::Level` ne doit gagner aucun champ.

## Travail à réaliser
- **Fonction pure de correspondance** : à partir du type de tuile et de son état logique, renvoyer le
  **nom du clip** attendu. Table explicite, un cas par famille :
  - porte : fermée / ouverte ;
  - interrupteur : inactif / actif ;
  - plaque de pression : relâchée / enfoncée ;
  - danger commuté : inactif / actif ;
  - danger temporisé : inoffensif / mortel ;
  - danger mobile : un seul clip, l'état est porté par la position.
- **Noms de clips conventionnels**, documentés dans `Source/Elements/Assets/README.md` : c'est le
  contrat que l'auteur d'un asset doit respecter pour que son animation soit reconnue.
- **Aucune lecture supplémentaire de `Core`** : uniquement les accesseurs listés ci-dessus, déjà
  publics.

## Fichiers impactés
- `Source/HMI/Graphics/MechanismVisuals.{h,cpp}` (nouveau).
- `Source/Elements/Assets/README.md` (convention de noms de clips).
- `Source/Test/Unit/HMI/Graphics/test_mechanism_visuals.cpp` (nouveau).

## Tests (obligatoires)
- **Chaque famille de mécanisme, chaque état** : la correspondance renvoie le clip attendu — table
  petite et figée, donc testable exhaustivement.
- Un type de tuile sans état (solide, bloc) ne produit aucune demande de clip.
- Fonction **pure** : ni GPU, ni Qt, ni ECS.

## Points d'attention
- **Lecture seule stricte** (`EX-ARCH-012`) : la correspondance interroge les contrôleurs, elle ne
  les modifie jamais et ne conserve aucun état de simulation.
- Ne pas confondre l'état d'un **danger commuté** (lié à un déclencheur) et celui d'un **danger
  temporisé** (période fixe) : ce sont deux sources différentes, et un asset peut vouloir les
  distinguer visuellement.
- La correspondance doit rester indépendante de l'asset : elle dit **quel clip demander**, pas si
  l'asset le fournit. Le repli est traité en TACHE-02.

## Définition de fait (DoD)
- La correspondance est une fonction pure, exhaustivement testée, sans nouvelle surface `Core` ; la
  convention de noms de clips est documentée ; `/W4 /WX` propre.

## Exigences
`EX-REN-006` (apparence pilotée par l'état logique) ; réutilise `EX-GP-020`/`EX-GP-021`
(interrupteur, porte), `EX-GP-025` (plaque de pression), `EX-GP-052`/`EX-GP-053` (dangers commuté et
temporisé), `EX-ARCH-012` (rendu en lecture seule), `EX-NFR-010` (testable sans GPU).
