# TACHE-03 — Personnage humanoïde (spawn 0,4×0,8, sprite) {#lot-11-tache-03-personnage-humanoide}

**Lot :** [LOT-11](epic.md) · **Emplacement :** `Source/HMI/Interface` · **Statut :** à faire

## Contexte
Le personnage n'est plus un carré 1×1 : il devient **humanoïde** (0,4 × 0,8). Le `GameScreen` doit
l'apparaître à cette taille — **collision** (Collider) **et** rendu (Transform.scale + sprite) — et
**centré** dans la tuile d'entrée (`Core/Physics/PlayerSpawn.h`, TACHE-01).

## Travail à réaliser
Dans `GameScreen::spawnPlayer` :
- Position : `core::playerSpawnPosition(entry.column, entry.row)` (centrée dans la tuile).
- `Collider` : `core::playerSize()` (0,4 × 0,8).
- `Transform.scale` : `core::playerSize()` pour que le **sprite** corresponde à la boîte (rendu
  humanoïde, `EX-REN-011`).
- Conserver le reste (Velocity, Player, région/couche du sprite).

## Fichiers impactés
- `Source/HMI/Interface/GameScreen.cpp` (spawn).

## Vérification (visuelle, pas de test unitaire — brique GPU)
- Le personnage apparaît **plus fin et plus haut** (silhouette), centré dans la tuile d'entrée.
- Il tombe, se pose, se déplace et interagit comme avant (les mécaniques sont indépendantes de la
  taille, gérées par le balayage).
- Capture de contrôle jointe à la revue.

## Points d'attention
- **Cohérence collision/rendu** : `Collider` et `Transform.scale` utilisent la **même** source
  (`playerSize()`), pas de valeurs dupliquées.
- **Spawn** : centré, sans coincer le personnage dans une tuile (marges symétriques).
- Aucune logique de simulation nouvelle ici — le `GameScreen` orchestre ; la taille est une donnée.

## Définition de fait (DoD)
- Personnage 0,4 × 0,8 en collision et au rendu, centré ; **vérifié visuellement** ; build `/W4 /WX`.

## Exigences
`EX-REN-011`, `EX-ARCH-011`, `EX-ARCH-012`.
