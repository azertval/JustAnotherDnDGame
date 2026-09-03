# LOT-33 — Fluidité du moteur (entrées nerveuses, présentation flip, interpolation) {#lot-33}

> Statut : **✅ terminé**.

## Objectif
Les lots précédents ont bâti le gameplay, l'éditeur et le contenu ; ce lot revient sur des **choix
techniques historiques de la boucle, du rendu et des entrées** qui dégradent le ressenti dès que la
machine tourne au-dessus de 60 Hz — l'écran de bureau courant (120/144/240 Hz). Aucun n'était un
bug « visible » dans les tests (tous déterministes, sans GPU ni horloge réelle), mais tous se
paient en jouabilité. Quatre corrections indépendantes, réunies parce qu'elles partagent la même
cause racine — **la boucle a toujours supposé un rendu à ~60 Hz calé sur la simulation** :

1. **Entrées perdues à haut framerate.** `Window::pumpMessages` avançait la ligne de base des
   fronts (`InputState::beginFrame`) à **chaque frame de rendu**, alors que seuls les pas de
   simulation lisent ces fronts. Quand le rendu dépasse 60 Hz, `FixedTimestep::advance` renvoie
   `0` pas sur une partie des frames (≈ 2 sur 3 à 144 Hz) : un appui capturé sur une de ces frames
   n'était **jamais lu**, puis effacé à la frame suivante. Résultat : des sauts/dashs « qui ne
   sortent pas », d'autant plus fréquents que l'écran est rapide — l'inverse d'un jeu nerveux
   (`EX-CTRL-020`, `EX-CTRL-021`, respectées seulement à 60 Hz jusqu'ici).
2. **Touches « collées » à la perte de focus.** Aucun traitement de `WM_KILLFOCUS` : une touche
   maintenue au moment d'un `Alt+Tab` ne reçoit jamais son `WM_KEYUP` et restait enfoncée — le
   personnage continuait d'avancer seul au retour dans la fenêtre.
3. **Micro-saccades du sondage manette.** `XInputGetState` est notablement coûteux quand le slot
   interrogé est **vide** (énumération de périphériques par le pilote) : le sonder à chaque frame
   alors qu'aucune manette n'est branchée provoquait des à-coups chez un joueur clavier.
4. **Présentation *blt* legacy + absence d'interpolation.** La swap chain utilisait l'ancien modèle
   `DXGI_SWAP_EFFECT_DISCARD` (copie supplémentaire, latence) au lieu du modèle **flip** moderne ;
   et surtout, la simulation à 60 Hz s'affichait **sans interpolation** sur des écrans plus rapides,
   produisant un *judder* (mouvement en marches d'escalier) même à framerate élevé. Un **facteur
   d'interpolation** était pourtant prévu **dès le départ** dans l'architecture (`EX-ARCH-031`) et
   exposé par `FixedTimestep::interpolationAlpha` — mais jamais exploité par le rendu.

Objectif du lot : rendre le jeu et les interfaces **fluides et nerveux quel que soit le framerate
d'affichage**, sans toucher au déterminisme de la simulation (`EX-NFR-002`) — toutes les
corrections vivent dans `HMI` (boucle, fenêtre, rendu) ou dans une couche de présentation qui
**lit** la simulation sans jamais la modifier.

## Périmètre

### Inclus
- **Découplage des fronts d'entrée et du rendu** (`HMI/Platform/Window`, `HMI/main.cpp`) :
  `pumpMessages` draine désormais la manette + les messages clavier/souris dans l'état **courant**
  sans avancer les fronts ; une nouvelle `Window::beginInputFrame` (recopie courant → précédent)
  est appelée par la boucle **après chaque pas fixe consommé**. Un appui capturé sur une frame sans
  pas de simulation **survit** jusqu'à ce qu'un pas le lise, au lieu d'être perdu. L'échantillonnage
  reste **une fois par frame, en amont de la logique** (`EX-CTRL-021` toujours respectée) ; la
  latence entrée → action reste **≤ un pas** (`EX-CTRL-020`, désormais à tout framerate).
- **Relâchement global à la perte de focus** (`InputState::releaseAll` sur `WM_KILLFOCUS`,
  `Source/HMI/Input`) :
  remet à zéro l'état courant **et** précédent de toutes les touches/boutons, sans produire de front
  « relâchée » parasite.
- **Sondage manette throttlé quand aucune manette n'est connectée** (`Window::pollGamepad`) : un
  slot resté déconnecté n'est re-sondé qu'une frame sur `GAMEPAD_DISCONNECTED_POLL_INTERVAL`
  (≈ toutes les 2 s), le sondage redevenant systématique dès qu'une manette est présente.
- **Modèle de présentation flip** (`HMI/Graphics/GraphicsDevice`, `EX-REN-004`) :
  `DXGI_SWAP_EFFECT_FLIP_DISCARD` + deux back buffers, à la place du modèle *blt* `DISCARD`. La
  cible de rendu est reliée au back buffer à chaque `clear()` (contrainte du flip model, qui la
  dé-lie à `Present`).
- **Interpolation de rendu** (`EX-ARCH-031`, concrétisée) : nouveau composant de présentation
  `hmi::PreviousPosition` (stocké dans le `core::World`, écrit par `HMI`, `Core` intact), rempli au
  début de chaque pas fixe par `GameScreen::snapshotPreviousPositions`. `SpriteRenderer::render`
  reçoit le facteur `FixedTimestep::interpolationAlpha` (via `RenderContext`) et dessine chaque
  entité mobile (personnage, dangers mobiles, blocs poussables) à `lerp(précédente, courante,
  alpha)` ; les entités fixes (tuiles) restent dessinées à leur position courante, sans surcoût.
- Documentation (guides boucle/rendu/entrées), mise à jour des spécifications concernées, tests.

### Exclus (hors périmètre de ce lot)
- **Interpolation de la caméra** — la caméra bascule déjà par **coupure nette** entre salles
  (`LOT-32`, choix assumé façon *Celeste*) : elle ne suit pas le personnage en continu, il n'y a
  donc rien à interpoler côté caméra. Seules les **entités** interpolent.
- **Interpolation de la rotation / de l'échelle** — seule la **position** est interpolée : c'est la
  seule grandeur qui varie en continu pour les entités concernées (l'échelle d'un bloc réduit est
  constante, aucune entité de jeu ne tourne). Étendre à rotation/échelle serait spéculatif.
- **`DXGI_PRESENT_ALLOW_TEARING` / présentation déchirée V-Sync désactivée** — le gain de fluidité
  visé porte sur la latence et la régularité **V-Sync activée** (défaut), qu'apporte déjà le flip
  model. Le *tearing* contrôlé pour un mode « framerate débridé » exige une détection de support
  DXGI dédiée : raffinement possible d'un lot ultérieur, hors périmètre.
- **Cadence de rendu plafonnée / mise en veille quand `steps == 0`** — la boucle continue de rendre
  à la fréquence de l'écran (V-Sync régule déjà). Un plafonnement d'économie d'énergie est un sujet
  distinct, non lié au ressenti.
- **Remappage, gameplay, contenu** — inchangés ; ce lot ne touche que la couche technique
  boucle/rendu/entrées.

## Décisions de cadrage
- **Avancer les fronts d'entrée par pas consommé, pas par frame de rendu** : c'est la correction
  minimale qui rétablit `EX-CTRL-020`/`EX-CTRL-021` à tout framerate sans changer le modèle
  d'échantillonnage (toujours une lecture OS par frame). Alternative écartée : caler le rendu sur
  60 Hz (reviendrait à annuler le découplage logique/rendu voulu par `EX-REN-021`).
- **`releaseAll` remet à zéro courant *et* précédent** : remettre seulement le courant produirait un
  front « relâchée » pour chaque touche maintenue au moment du basculement, susceptible de déclencher
  une action de relâchement fantôme (coupe de saut, etc.). Zéro sur les deux = aucun front.
- **Throttler le sondage d'un slot déconnecté plutôt que d'arrêter tout sondage** : il faut continuer
  à détecter un **branchement** à chaud ; un intervalle fixe (≈ 2 s) est un compromis simple entre
  réactivité au branchement et coût CPU nul en régime permanent.
- **Flip model sans `ALLOW_TEARING`** : le flip model apporte l'essentiel (latence réduite, cadence
  régulière) dès `FLIP_DISCARD` + 2 buffers, y compris V-Sync activée — le cas par défaut. Le
  *tearing* débridé, plus complexe (détection de support), est explicitement reporté.
- **`PreviousPosition` est un composant *de présentation*, côté `HMI`, mais rangé dans le
  `core::World`** : le `World` est générique sur le type de composant, donc `HMI` peut y stocker sa
  propre donnée d'interpolation **sans modifier `Core`** ni introduire de règle de simulation
  dépendante du rendu (`EX-NFR-010`, `EX-ARCH-011`, `EX-ARCH-012` intacts). Le rendu **lit** la
  simulation (`Transform`) et n'interpole que sa copie affichée, sans jamais muter l'état.
- **Interpoler uniquement les entités réellement mobiles** (personnage, dangers mobiles, blocs) via
  la présence du composant, plutôt que toutes les entités : les centaines de tuiles fixes n'ont pas
  le composant et sont dessinées à leur position courante (une simple recherche `hasComponent` O(1)
  par entité, négligeable). Interpoler entre deux positions **égales** serait de toute façon un
  no-op, mais l'éviter garde le rendu des tuiles strictement inchangé.

## Exigences couvertes
- Nouvelle : `EX-REN-004` (modèle de présentation flip, faible latence).
- Concrétisée (déclarée dès `LOT-01`, jamais implémentée) : `EX-ARCH-031` (facteur d'interpolation
  `[0,1]` fourni au rendu pour lisser le mouvement).
- Conformité restaurée à tout framerate (comportement, pas le sens) : `EX-CTRL-020` (latence
  entrée → action ≤ un pas), `EX-CTRL-021` (échantillonnage une fois par frame en amont de la
  logique).
- Réutilisées (inchangées) : `EX-REN-021`/`EX-ARCH-030` (simulation à pas fixe, rendu découplé),
  `EX-REN-022` (V-Sync activable), `EX-NFR-002` (déterminisme), `EX-NFR-010`/`EX-ARCH-011`/
  `EX-ARCH-012` (frontière `HMI → Core`, composants purs, rendu en lecture seule), `EX-NFR-040`
  (robustesse — manette absente/débranchée).

## Découpage

> État : ✅ fait · 🔄 en cours · ⬜ non commencé.

| Tâche | Intitulé | Emplacement | État |
|-------|----------|-------------|:----:|
| [TACHE-01](tache-01-entrees-nerveuses.md) | Entrées nerveuses : fronts non perdus, focus, sondage manette | `HMI/Platform`, `HMI/Input`, `HMI/main.cpp` | ✅ |
| [TACHE-02](tache-02-presentation-flip.md) | Présentation flip-model (latence réduite) | `HMI/Graphics` | ✅ |
| [TACHE-03](tache-03-interpolation-rendu.md) | Interpolation de rendu (`EX-ARCH-031`) | `HMI/Graphics`, `HMI/Interface` | ✅ |
| [TACHE-04](tache-04-documentation-verification.md) | Documentation, spécifications et vérification | `Documentation` | ✅  |

## Critères d'acceptation du lot
1. Sur un écran à 120/144 Hz, aucun appui (saut, dash, direction) n'est « avalé » : chaque appui
   consommé par un pas de simulation est pris en compte, quel que soit le framerate de rendu
   (`EX-CTRL-020`/`EX-CTRL-021`).
2. Un `Alt+Tab` en maintenant une direction ne laisse plus le personnage avancer seul au retour :
   les entrées sont relâchées à la perte de focus, sans action de relâchement parasite.
3. Sans manette branchée, aucune micro-saccade imputable au sondage XInput ; brancher une manette la
   rend opérante en moins de ~2 s.
4. Le mouvement du personnage (et des dangers mobiles / blocs poussés) est **lisse** à framerate
   élevé, sans *judder* en marches d'escalier — l'interpolation est visible et correcte.
5. La simulation reste **strictement déterministe** : la suite de tests (unitaires, intégration,
   système) passe à 100 % sans modification de résultat (`EX-NFR-002`), et le parcours système
   franchit toujours toute la séquence de niveaux.
6. Build `/W4 /WX` sans avertissement, Doxygen et lint des exigences verts. Logique nouvelle testable
   couverte (`InputState::releaseAll`, `FixedTimestep::interpolationAlpha`) ; l'intégration
   rendu/présentation (dépendance D3D11) est vérifiée **visuellement**, comme les autres lots de
   rendu (cf. `LOT-32`).

## Dépendances
- Étend `hmi::Window`/`hmi::main` (boucle, `LOT-01`), `hmi::InputState` (`LOT-06`/`20`/`30`),
  `hmi::GraphicsDevice` (`LOT-01`/`05`), `hmi::SpriteRenderer`/`RenderContext` (`LOT-05`) et
  `hmi::GameScreen` (`LOT-08`…`LOT-32`), sans modifier `core::FixedTimestep` (déjà prêt via
  `interpolationAlpha`, `LOT-01`) ni aucun composant/système de `Core`.

## Navigation des tâches
- @subpage lot-33-tache-01-entrees-nerveuses
- @subpage lot-33-tache-02-presentation-flip
- @subpage lot-33-tache-03-interpolation-rendu
- @subpage lot-33-tache-04-documentation-verification
