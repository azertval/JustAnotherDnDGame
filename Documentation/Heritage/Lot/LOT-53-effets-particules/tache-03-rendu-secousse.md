# TACHE-03 — Rendu des particules et secousse d'écran {#lot-53-tache-03-rendu-secousse}

**Lot :** [LOT-53](epic.md) · **Emplacement :** `Source/HMI/Graphics` · **Statut :** ✅ fait

## Contexte
Les particules simulées (TACHE-01) et déclenchées (TACHE-02) doivent être dessinées, et la secousse
d'écran — le seul effet qui n'est pas une particule — appliquée.

Le rendu ne demande rien de nouveau : un quad par particule, sur des calques déjà existants. La
secousse, en revanche, touche la caméra, ce qui mérite une précaution explicite.

## Travail à réaliser
- **Rendu des particules** : un quad par particule vivante, teinté selon sa durée de vie restante
  (disparition en fondu), sur le calque *Object* ou *Foreground* selon l'effet — une traînée de dash
  passe derrière le personnage, une bouffée de poussière devant.
- **Texture** : soit une région de l'atlas, soit un asset dédié (`Assets/Effects/`). Trancher ; un
  simple carré teinté suffit pour un premier jet et évite un dossier d'assets de plus.
- **Mode Texture uniquement** : aucun effet en mode Physique, qui reste la lecture nue des
  collisions.
- **Secousse d'écran** : décalage temporaire appliqué à la **caméra de rendu** uniquement.
  - amplitude et durée en constantes nommées, volontairement **conservatrices** — un jeu de
    plate-forme précis devient injouable si l'écran bouge trop ;
  - décalage arrondi au pixel écran entier (netteté du pixel art, `EX-ARCH-022`) ;
  - jamais appliquée à la position simulée, ni à la logique de cadrage par salle (`EX-REN-015`).
- **Culling** : les particules passent par le culling comme le reste (LOT-40, TACHE-05).

## Fichiers impactés
- `Source/HMI/Graphics/ParticleRenderer.{h,cpp}` (nouveau) ou extension de `SpriteRenderer`.
- `Source/HMI/Graphics/Camera2D.{h,cpp}` (décalage de secousse).
- `Source/HMI/Game/GameSession.{h,cpp}`.
- `Source/Test/Unit/HMI/Graphics/test_screen_shake.cpp` (nouveau).

## Tests (obligatoires)
- Un quad par particule vivante, sur le calque attendu — asserté via le *QuadRecorder*.
- Aucun quad de particule en mode Physique.
- **Secousse** : le décalage décroît jusqu'à zéro dans la durée prévue, est arrondi au pixel entier,
  et la position simulée du personnage est **inchangée** pendant toute la secousse.
- La secousse ne modifie pas la salle cadrée (pas de bascule de salle provoquée par le décalage).
- Sans GPU.

## Points d'attention
- **La secousse ne doit jamais provoquer de bascule de salle.** La caméra bascule sur la position du
  personnage, pas sur son propre rectangle ; le vérifier explicitement, car un décalage appliqué au
  mauvais endroit produirait un clignotement de salle à chaque atterrissage.
- Un effet trop marqué nuit à la lisibilité. En cas de doute, réduire : il est plus facile
  d'augmenter une constante plus tard que de diagnostiquer un ressenti dégradé.
- Vérifier l'interaction avec l'interpolation de rendu (`hmi::PreviousPosition`) : la secousse
  s'applique après, sur la caméra, pas sur les positions interpolées.

## Définition de fait (DoD)
- Les particules sont dessinées sur le bon calque avec disparition en fondu, uniquement en mode
  Texture ; la secousse est brève, nette, sans effet sur la simulation ni sur le cadrage par salle ;
  assertions sans GPU vertes ; `/W4 /WX` propre.

## Exigences
`EX-REN-008` (effets visuels) ; réutilise `EX-REN-043` (calques), `EX-REN-046` (bascule),
`EX-REN-015` (cadrage par salle), `EX-ARCH-022` (netteté), `EX-ARCH-012` (rendu sans effet sur la
simulation), `EX-NFR-005` (culling et budget).
