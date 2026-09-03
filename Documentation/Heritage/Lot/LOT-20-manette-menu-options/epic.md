# LOT-20 — Manette et menu d'options {#lot-20}

> Statut : **terminé**. Deux évolutions de l'interface, indépendantes mais livrées ensemble :
> le jeu et l'éditeur deviennent jouables/navigables à la **manette** (`EX-CTRL-002`), et un
> nouveau **menu d'options** centralise les réglages jusqu'ici dispersés (langue) ou absents
> (V-Sync).

## Objectif
- **Manette** : `EX-CTRL-002` (souhaitée) n'est pas implémentée — seuls clavier et souris sont
  acquis. Le jeu doit rester jouable au clavier **et** devenir jouable à la manette, sans
  dupliquer la logique de gameplay/menu qui lit déjà `InputState`.
- **Menu d'options** : aucun écran ne permet de régler quoi que ce soit à part la langue (icône
  en coin, sur le menu principal seulement). Un écran dédié doit centraliser les réglages
  disponibles et rester accessible depuis le menu principal.

## Périmètre

### Inclus
- **Manette (XInput)** : D-pad et stick gauche (avec zone morte) pilotent les mêmes directions
  que les flèches/`Q`/`D` ; bouton **A** valide (menu) et saute (jeu) ; **B** et **Start**
  reviennent/échappent ; épaule droite (**RB**) dashe. Fonctionne **partout** où le clavier
  fonctionne déjà (menu, jeu, éditeur) sans toucher à leur code : la manette est fusionnée dans
  `InputState` au niveau des mêmes `Key` déjà utilisés.
- **Menu d'options** (nouvel écran, accessible depuis le menu principal) : bascule **V-Sync**
  (nouveau réglage, jusqu'ici fixé en dur), bouton de langue (réutilise le composant existant),
  état de connexion de la manette (information, non interactif), retour au menu.

### Exclus (hors périmètre de ce lot)
- **Remappage des touches** (`EX-CTRL-012`, souhaité) — un fichier de configuration éditable
  reste un lot à part ; ce lot fixe un mapping manette **par défaut**, non reconfigurable.
- **Écran de pause** (`EX-REN-031`) — n'existe toujours pas ; le bouton **Start** de la manette
  reproduit le comportement actuel d'Échap (retour au menu depuis le jeu), sans nouvel écran.
- **Action « Interagir »** (ligne du tableau `controles.md`, bouton **X**) — aucune mécanique de
  jeu ne l'utilise aujourd'hui (les mécanismes s'activent par contact, pas par un bouton dédié) ;
  mapper un bouton sans logique en aval serait du code mort. Laissé pour un futur lot qui
  introduirait une telle mécanique.
- **Vibration/retour haptique** — non demandé, non couvert par `EX-CTRL-002`.

## Décisions de cadrage
- **La manette est fusionnée dans `InputState`, pas exposée comme une source séparée.** Chaque
  bouton/direction manette synthétise le **même** `Key` que son équivalent clavier (`A` →
  `Key::Enter` **et** `Key::Space`, D-pad/stick → `Key::Left/Right/Up/Down`, etc.). Conséquence
  directe : **aucun** consommateur existant (`MenuModel`, `LevelPicker`, `PlayerInputMapper`,
  raccourcis de l'éditeur…) n'a besoin d'être modifié — la manette « fonctionne » partout dès la
  fusion faite une seule fois, satisfaisant `EX-CTRL-010` (action logique dissociée de la touche
  physique) sans introduire de couche d'indirection supplémentaire, puisque le `Key` de
  `InputState` **est** déjà cette couche.
- **Fusion au niveau lecture, pas au niveau écriture, pour ne jamais écraser le clavier.** Le
  clavier et la manette sont deux **sources indépendantes** avec leur propre paire courant/
  précédent ; `keyDown`/`keyPressed`/`keyReleased` combinent les deux (`OU` logique) au moment de
  la lecture. Écrire directement dans les tableaux du clavier depuis le sondage manette
  effacerait une touche clavier réellement maintenue dès que la manette relâche le bouton
  correspondant — bug réel, évité par construction avec deux sources séparées.
- **Le sondage XInput vit dans `Window` (`Platform`), pas dans `InputState`.** `InputState` reste
  **indépendant de toute fenêtre** (`EX-NFR-010`, testable en isolation) — seule `Window`, déjà
  dépendante de Win32, gagne une dépendance à `Xinput.h`. `InputState` expose seulement des
  méthodes d'écriture génériques (`onGamepadKeyDown`/`onGamepadKeyUp`), sans rien connaître de
  XInput.
- **Le menu d'options est un écran à part (`ScreenId::Options`), pas une modale.** Cohérent avec
  l'architecture existante (`ScreenManager`, un `IScreen` par écran) ; navigation clavier/souris/
  manette réutilisée du même modèle que `MenuModel` (`optionAtPoint`, mise en page à chasse
  fixe).
- **Le réglage V-Sync est appliqué immédiatement** (pas de bouton « Appliquer ») : cohérent avec
  la bascule de langue existante, un seul geste suffit.

## Exigences couvertes
- `EX-CTRL-002` (manette XInput) — implémentée, marqueur « souhaité » retiré.
- Réutilisées (inchangées) : `EX-CTRL-010` (action logique), `EX-CTRL-011` (pressée/maintenue/
  relâchée), `EX-CTRL-021` (échantillonnage une fois par frame), `EX-REN-022` (V-Sync activable
  — désormais réellement activable, pas seulement dans l'intention du texte de spec),
  `EX-REN-033` (catalogue de traduction, nouvelles clés `options.*`).

## Découpage

> État : ✅ fait · 🔄 en cours · ⬜ non commencé.

| Tâche | Intitulé | Emplacement | État |
|-------|----------|-------------|:----:|
| [TACHE-01](tache-01-integration-manette.md) | Intégration manette (XInput) | `HMI/Input`, `HMI/Platform` | ✅ |
| [TACHE-02](tache-02-menu-options.md) | Menu d'options | `HMI/Interface`, `HMI/Graphics` | ✅ |
| [TACHE-03](tache-03-documentation-verification.md) | Documentation et vérification | `Documentation` | ✅ |

## Critères d'acceptation du lot
1. Manette branchée : le menu principal, le sélecteur de niveau et le menu d'options se
   naviguent entièrement au D-pad/stick + A/B, sans toucher au clavier.
2. En jeu, la manette déplace, fait sauter (A) et dasher (RB) le personnage, à l'identique du
   clavier.
3. Sans manette branchée, rien ne change : aucune régression sur le clavier/souris (le sondage
   XInute échoue silencieusement, `ERROR_DEVICE_NOT_CONNECTED`, sans dégrader la boucle).
4. Le menu d'options bascule V-Sync et la langue, affiche l'état de connexion de la manette, et
   revient au menu principal — accessible comme 4ᵉ option du menu principal.
5. Logique nouvelle **couverte par des tests** (`ctest` vert), déterministe, sans GPU ni manette
   réelle (le sondage XInput lui-même n'est pas testé unitairement, comme le reste des accès
   Win32 de `HMI`). Build `/W4 /WX` sans avertissement, Doxygen et lint des exigences verts.

## Dépendances
- Étend `InputState`/`Window` (LOT-02/03), `MenuModel`/`MenuScreen` (LOT-04), `LanguageSelector`
  (LOT-13), `GraphicsDevice` (LOT-02), `PlayerInputMapper` (LOT-03) — sans les modifier pour la
  partie manette (fusion transparente).

## Navigation des tâches
- @subpage lot-20-tache-01-integration-manette
- @subpage lot-20-tache-02-menu-options
- @subpage lot-20-tache-03-documentation-verification
