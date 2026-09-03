# TACHE-03 — Orientation et non-régression du gameplay {#lot-48-tache-03-orientation-non-regression}

**Lot :** [LOT-48](epic.md) · **Emplacement :** `Source/HMI/Graphics`, `Source/Test` · **Statut :** fait

## Contexte
Le personnage regarde toujours dans la même direction : `core::Player::facing` existe et est
maintenu par la physique (il donne notamment la direction d'un dash sans saisie directionnelle,
`EX-CTRL-013`), mais le rendu ne l'utilise pas.

Par ailleurs, ce lot modifie le sprite le plus visible du jeu **sans devoir toucher au gameplay** :
la non-régression mérite une tâche dédiée plutôt qu'une ligne dans une définition de fait.

## Travail à réaliser
- **Retournement horizontal** selon `core::Player::facing`, par inversion des coordonnées de texture
  (`u0` et `u1` échangés) dans le quad du personnage — l'auteur ne dessine qu'un sens.
- **Ancrage préservé au retournement** : le point d'ancrage (TACHE-01) doit être symétrique, sinon le
  personnage se décalerait d'un demi-pixel à chaque changement de sens.
- **Suite de tests de non-régression du gameplay** : vérifier explicitement que la boîte de
  collision, les tests de franchissabilité des quinze niveaux de démonstration et les tests de
  physique passent **sans modification**.
- **Documentation** : mettre à jour `Documentation/Guide/guide-rendu.md` (section personnage) —
  elle décrit aujourd'hui en détail la silhouette procédurale et la contrainte de région carrée, qui
  ne sont plus le cas nominal.

## Fichiers impactés
- `Source/HMI/Graphics/PlayerSprite.{h,cpp}`.
- `Source/Test/Unit/HMI/Graphics/test_player_sprite.cpp`, `Source/Test/Systeme/` (franchissabilité).
- `Documentation/Guide/guide-rendu.md`, `Source/Elements/Assets/README.md`.

## Tests (obligatoires)
- Retournement : pour les deux orientations, les coordonnées de texture sont échangées et la position
  du quad est **symétrique** par rapport à l'ancrage (pas de décalage d'un demi-pixel).
- **Non-régression** : tests de physique, de collision et de franchissabilité inchangés et verts.
- La boîte de collision est identique avant et après le lot, quelle que soit la taille d'image.

## Points d'attention
- **Un test de franchissabilité qui échoue signale un défaut, pas un ajustement à faire.** C'est le
  contrôle qui garantit qu'un changement d'apparence n'a pas déplacé la hitbox.
- Le retournement ne doit pas s'appliquer aux **effets** attachés au personnage (LOT-53) sans
  décision explicite : une traînée de dash a sa propre orientation.
- Attention au filtrage : un retournement mal aligné sur la grille de pixels produit un décalage
  visible d'un pixel en pixel art — vérifier sur un motif asymétrique.

## Définition de fait (DoD)
- Le personnage regarde dans le sens de son déplacement, sans décalage au retournement ; tous les
  tests de gameplay et de franchissabilité passent inchangés ; le guide et le README d'assets sont à
  jour ; `/W4 /WX` propre.

## Exigences
`EX-REN-009` (personnage depuis une spritesheet, orientation) ; réutilise `EX-CTRL-013` (orientation
et dash), `EX-ARCH-022` (*nearest*), `EX-NFR-021` (test système de franchissabilité),
`EX-ARCH-012` (rendu sans effet sur la simulation).
