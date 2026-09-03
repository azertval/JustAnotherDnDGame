# TACHE-05 — Système de rendu des sprites (ECS → écran) {#lot-05-tache-05-systeme-rendu-sprites}

**Lot :** [LOT-05](epic.md) · **Emplacement :** `Source/HMI` · **Statut :** fait

## Contexte
Le pont entre la simulation et l'image : un composant de rendu qui **lit** l'ECS et dessine
chaque entité affichable. Il vit dans `HMI` (présentation) et ne modifie jamais les
composants (`EX-ARCH-012`).

## Travail à réaliser
- `SpriteRenderer` (dans `HMI`) : à chaque frame, itère `world.view<Transform, Sprite>()`,
  résout la région d'atlas (TACHE-03), calcule le quad monde (position/échelle depuis
  `Transform`), applique la caméra (TACHE-04) et l'empile dans le `SpriteBatch` (TACHE-02).
- **Tri par couche** (`EX-REN-014`) : dessiner fond → décor → entités → interface (ordre
  stable et déterministe à couche égale).
- Rendu **en lecture seule** de l'ECS : accès par référence const, aucune mutation.

## Fichiers impactés
- `Source/HMI/Graphics/SpriteRenderer.h`, `SpriteRenderer.cpp` (nouveau).
- `Source/HMI/CMakeLists.txt`.

## Vérifications (obligatoires)
- Toutes les entités `Transform + Sprite` sont dessinées ; celles sans `Sprite` ne le sont pas.
- L'ordre de dessin respecte les couches (une entité de couche supérieure passe au-dessus).
- Vérification que l'ECS n'est pas muté par le rendu (accès const).

## Points d'attention
- Ne pas allouer par frame de façon incontrôlée (réutiliser les tampons du `SpriteBatch`).
- Le renderer **n'est pas** un `core::ISystem` (il vit dans `HMI`) : il lit le `World` mais
  n'est pas orchestré par `World::update` (le rendu est découplé de la simulation,
  `EX-REN-021`).

## Définition de fait (DoD)
- Rendu ECS → écran correct (couches respectées, lecture seule), documenté ;
  build `/W4 /WX` sans avertissement.

## Exigences
`EX-ARCH-012`, `EX-REN-010`, `EX-REN-011`, `EX-REN-014`, `EX-REN-021`.
