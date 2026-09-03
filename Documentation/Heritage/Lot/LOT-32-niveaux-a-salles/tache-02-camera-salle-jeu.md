# TACHE-02 — Caméra par salle en jeu (coupure nette) {#lot-32-tache-02-camera-salle-jeu}

**Lot :** [LOT-32](epic.md) · **Emplacement :** `HMI/Interface` · **Statut :** fait (mécanique de
salle/franchissement prouvée par le test système de TACHE-04, sans couche GPU ; confirmation
visuelle à l'écran non automatisée, laissée à un essai manuel par un humain lançant le jeu)

## Contexte
`GameScreen::render` (`LOT-16`) calcule aujourd'hui un zoom d'ajustement sur le **niveau entier**
(`Camera2D::fitZoom` avec `_levelWidth`/`_levelHeight`) et centre la caméra **une seule fois**, au
chargement (`loadLevel`, milieu du niveau). Cette tâche fait dépendre le rectangle cadré de la
**salle courante** du personnage (`RoomGrid`, TACHE-01) plutôt que du niveau entier, avec une
**coupure nette** au changement de salle plutôt qu'un recentrage continu.

## Travail à réaliser
- **Construire un `hmi::RoomGrid`** à partir des dimensions du niveau chargé (`_levelWidth`/
  `_levelHeight`), au même moment que le reste de l'état de scène (`loadLevel`).
- **Suivre la salle courante** : chaque pas fixe (ou à chaque frame de rendu, à trancher à
  l'implémentation — la position du personnage change au pas fixe, la caméra n'a besoin d'être
  réévaluée qu'à ce rythme), lire la position du personnage, la convertir en `GridPosition` (case
  contenant son centre), en déduire l'indice de salle via `RoomGrid::roomIndexAt`.
- **Coupure nette au changement d'indice** : si l'indice de salle a changé depuis la dernière
  évaluation, recalculer `roomBounds` de la nouvelle salle et appeler `_camera.setCenter(...)`
  **une fois**, sur le centre exact du rectangle de la nouvelle salle — pas d'interpolation, pas de
  recentrage image par image tant que le personnage reste dans la même salle (à la différence d'une
  caméra suiveuse, explicitement écartée en `LOT-16` et dans l'épic de ce lot).
- **Zoom** : dans `GameScreen::render`, remplacer l'appel `Camera2D::fitZoom(..., _levelWidth,
  _levelHeight, ...)` par un appel utilisant la **largeur/hauteur de la salle courante**
  (`roomBounds` de TACHE-01) — même fonction, même règle (entier tant que `≥ 1`, fractionnaire sinon,
  `EX-ARCH-022`), juste un rectangle plus petit en entrée.
- **Non-régression** : pour un niveau tenant dans une seule salle, `RoomGrid` produit une seule
  salle dont le rectangle couvre le niveau entier (TACHE-01) — le calcul de zoom et de centre
  retombe exactement sur le comportement `LOT-16` actuel, sans branche spéciale à écrire pour ce
  cas.
- **Chargement/redémarrage** : au (re)chargement d'un niveau (nouvelle partie, échec →
  redémarrage), le personnage repart à l'**entrée** — recalculer la salle et centrer la caméra dessus
  immédiatement (pas de salle « en retard » d'une frame après un redémarrage).

## Fichiers impactés
- `Source/HMI/Interface/GameScreen.h`/`.cpp` (état `RoomGrid`/salle courante, `render`,
  `loadLevel`, réinitialisation au redémarrage).

## Tests (obligatoires)
- Non testable automatiquement au niveau de `GameScreen` (dépendance rendu D3D11, non compilé dans
  `UnitTests`, même limite que `LOT-16` TACHE-03) : vérifié par relecture et **essai manuel** —
  charger le niveau de démonstration de TACHE-04 (plus grand qu'une salle) et constater que la
  caméra reste au zoom natif et bascule nettement d'une salle à l'autre en franchissant chaque
  frontière, dans les quatre directions.
- Non-régression manuelle : les niveaux existants (tenant dans une seule salle) s'affichent avec
  **exactement** le même zoom/cadrage qu'avant ce lot.
- La logique de **décision** (« l'indice de salle a-t-il changé ? ») doit rester séparable et
  testable sans GPU si l'implémentation le permet naturellement (ex. une petite fonction pure
  `hasRoomChanged(previous, current)` ou équivalent) — sans forcer une abstraction si `RoomGrid`
  seul (TACHE-01, déjà testé) suffit à couvrir la logique non triviale.

## Points d'attention
- **Ne pas recentrer à chaque frame** même dans la salle courante : `setCenter` n'est appelé qu'au
  **changement** d'indice de salle, sinon le moindre calcul flottant pourrait introduire un
  tremblement invisible mais superflu — la caméra doit rester **rigoureusement fixe** tant que le
  personnage reste dans la même salle (comportement `LOT-16` déjà de ce type : caméra fixe, pas
  suiveuse).
- **Écran d'erreur** (`_loadError`, cf. `GameScreen::render` actuel) : chemin inchangé, aucune
  notion de salle n'y intervient.
- `Camera2D` (`LOT-05`) n'a besoin d'aucune modification : `setCenter`/`setZoom` acceptent déjà
  n'importe quel centre/zoom positif.

## Définition de fait (DoD)
- Caméra en jeu cadrée sur la salle courante, coupure nette au changement, non-régression sur les
  niveaux existants vérifiée manuellement ; build `/W4 /WX` sans avertissement.

## Exigences
`EX-REN-015` (nouvelle), `EX-REN-013` (portée corrigée — scopée aux niveaux tenant dans une seule
salle), `EX-ARCH-022` (zoom pixel art de préférence entier).
