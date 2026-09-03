# TACHE-03 — Physique du personnage (gravité + déplacement + collisions) {#lot-08-tache-03-physique-personnage}

**Lot :** [LOT-08](epic.md) · **Emplacement :** `Source/Core/Ecs/Systems` · **Statut :** à faire

## Contexte
Cette tâche assemble l'intention d'entrée (TACHE-01) et le balayage (TACHE-02) en un **système de
simulation** à pas fixe, sur le modèle du `MovementSystem` existant : toute la logique dans le
système, les composants restent des données. C'est le cœur jouable du personnage — déplacement,
gravité, collisions — **sans saut** (hors périmètre).

## Travail à réaliser
- **`CharacterPhysicsSystem`** (ou nom proche) opérant sur les entités `Player` + `Transform` +
  `Velocity` + `Collider`, avec accès au `TileMap` (solides) et à l'intention `PlayerInput`.
- À chaque pas fixe :
  1. **Vitesse horizontale** = `moveX × vitesse constante` (⚠️ ~6 tuiles/s, `EX-GP-010`) — pas
     d'inertie au MVP.
  2. **Gravité** : `vy += g × dt`, appliquée en continu tant que le personnage n'est **pas au sol**
     (`EX-GP-012`) ; borne de vitesse de chute (terminal velocity) pour éviter le tunneling
     extrême et garder un ressenti stable.
  3. **Déplacement** `delta = velocity × dt`, résolu par le **balayage** (TACHE-02) : la position
     est corrigée, la **composante normale de la vitesse annulée** (choc mur/sol).
  4. **État « au sol »** : `Player::grounded` vrai si le contact bloquant du pas est **sous** le
     personnage (normale vers le haut) ; faux sinon.
- Constantes de physique regroupées et nommées (gravité, vitesse horizontale, vitesse de chute
  max), faciles à ajuster lors du futur *tuning*.

## Fichiers impactés
- `Source/Core/Ecs/Systems/CharacterPhysicsSystem.h`/`.cpp` (nouveau).
- `Source/Core/CMakeLists.txt`, `Source/Test/CMakeLists.txt`.

## Tests (obligatoires)
- **Chute** : sans sol dessous, `y` augmente et `vy` croît d'un pas à l'autre (gravité continue).
- **Atterrissage** : au-dessus d'un sol solide, le personnage s'**arrête** dessus, `vy` remis à 0,
  `grounded == true`, sans pénétration.
- **Blocage horizontal** : contre un mur, la position horizontale n'avance plus, `vx` annulé.
- **Vitesse constante** : `moveX = 1` sur terrain libre → avancée horizontale ≈ `vitesse × dt`.
- **Non-tunneling en chute rapide** : une grande vitesse verticale sur un pas ne traverse pas un
  sol fin (via le balayage) — vérifie l'intégration physique ↔ collision.
- **Déterminisme** : mêmes entrées (état + intention + dt) → même état final (`EX-NFR-002`).

## Points d'attention
- **Logique dans le système, données dans les composants** (`EX-ARCH-011`) — miroir du
  `MovementSystem`.
- **Pas de saut** ici : ne pas lire de champ « saut » ; l'état `grounded` est calculé mais sert
  surtout la caméra et le lot physique ultérieur.
- Pas fixe uniquement (`EX-NFR-002`) : aucune dépendance au temps réel de la frame.
- Réutiliser le balayage de TACHE-02 sans le réimplémenter ; le système **oriente**, la primitive
  **résout**.

## Définition de fait (DoD)
- `CharacterPhysicsSystem` fonctionnel, documenté et **testé** (cas ci-dessus, `ctest` vert) ;
  build `/W4 /WX`.

## Exigences
`EX-GP-010`, `EX-GP-012`, `EX-GP-002`, `EX-GP-014`, `EX-NFR-002`, `EX-ARCH-011`.
