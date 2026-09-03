# TACHE-04 — Caméra : pan et zoom manuels {#lot-15-tache-04-camera-pan-zoom}

**Lot :** [LOT-15](epic.md) · **Emplacement :** `HMI/Interface` · **Statut :** à faire

## Contexte
`EditorScreen::renderGrid` recadre et rezoome automatiquement la caméra sur toute la grille à
**chaque frame** — pratique pour un petit niveau, mais impossible à contourner pour zoomer en
précision ou consulter une zone sans voir le reste dès que la grille dépasse la fenêtre.

## Travail à réaliser
- **Zoom** : la molette (`InputState::wheelDelta()`, TACHE-01) ajuste un facteur de zoom manuel
  (entier ou pas fixe, cohérence pixel art `EX-ARCH-022`), appliqué à `Camera2D::setZoom` à la
  place du calcul d'ajustement automatique dès qu'un zoom manuel est actif.
- **Pan** : glisser avec le bouton **droit** de la souris (`MouseButton::Right`, déjà suivi par
  `InputState`) déplace le centre de la caméra (`Camera2D::setCenter`) de la distance parcourue à
  l'écran, convertie en unités monde via l'échelle courante — aucun conflit avec le clic gauche
  (peinture/palette/liaison) ni avec les flèches (redimensionnement, inchangé).
- **Réinitialisation** : `Key::D0` (« 0 », ajoutée en TACHE-01) revient au cadrage automatique
  existant (comportement LOT-14, conservé comme état par défaut à l'ouverture de chaque niveau).
- L'ajustement automatique de `renderGrid` devient l'état **initial** (et l'état après
  réinitialisation), pas plus le calcul systématique de chaque frame.

## Fichiers impactés
- `Source/HMI/Interface/EditorScreen.h`/`.cpp` (état caméra manuelle : zoom courant, drapeau
  « cadrage manuel actif », gestion du glisser droit).

## Tests (obligatoires)
- Molette avec delta positif/négatif augmente/diminue le zoom appliqué à la caméra.
- Glisser bouton droit déplace le centre caméra dans le sens attendu, proportionnellement à
  l'échelle courante (un pan à fort zoom déplace moins d'unités monde qu'à faible zoom, pour une
  même distance écran).
- La touche de réinitialisation restaure exactement le cadrage automatique (mêmes centre/zoom que
  calculés par `renderGrid` en LOT-14).
- Peindre au clic gauche et redimensionner aux flèches restent inchangés pendant qu'un pan/zoom
  manuel est actif.

## Points d'attention
- `Camera2D` (LOT-05) n'a pas besoin d'évoluer : `setCenter`/`setZoom` suffisent, tout l'état
  « manuel vs automatique » reste côté `EditorScreen`, cohérent avec le rôle de présentation pure
  de la caméra (`EX-ARCH-030`, elle ne lit jamais l'ECS).
- Borner le zoom manuel à une valeur minimale raisonnable (> 0) pour éviter une division dégénérée
  dans `screenToWorld`/`worldToScreen`.

## Ajustement post-livraison (essai utilisateur)
Un premier passage ne bornait le zoom qu'à `≥ 1.0`, sans lien avec la taille du niveau. Un essai
interactif a montré que ça permettait de dézoomer bien au-delà de ce qu'il y a à voir, et de zoomer
sans limite jusqu'à perdre toute lisibilité. Les bornes ont été resserrées : **minimum** = le zoom
d'ajustement automatique du niveau **courant** (rien à voir de plus loin), **maximum** = la valeur
laissant encore **4 cases visibles** sur le plus petit axe de l'écran (précision jugée suffisante
pour poser un bloc). Les deux bornes se recalculent à chaque molette sur les dimensions courantes
du brouillon — aucune valeur figée, donc valable sans changement pour des niveaux plus grands
(prévu pour un lot ultérieur, LOT-16).

## Définition de fait (DoD)
- Pan/zoom manuels et réinitialisation opérationnels et testés (`ctest` vert) ; build `/W4 /WX` ;
  Doxygen à jour.

## Exigences
`EX-EDIT-013`, `EX-ARCH-022` (zoom entier, netteté pixel art).
