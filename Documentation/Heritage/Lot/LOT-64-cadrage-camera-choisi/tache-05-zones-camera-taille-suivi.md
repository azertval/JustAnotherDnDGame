# TACHE-05 — Zones de caméra dessinées à la main et taille de suivi réglable {#lot-64-tache-05-zones-camera-taille-suivi}

**Lot :** [LOT-64](epic.md) · **Emplacement :** `Source/Core/Levels`, `Source/HMI/Graphics`,
`Source/HMI/Editor`, `Source/HMI/Game` · **Statut :** fait

## Contexte
Les TACHE-01 à 04 livrent trois modes de cadrage, dont le mode *par salle* : une **grille
uniforme**, une seule taille de salle pour tout le niveau. Une fois ces trois modes en main, le
level designer a exprimé un besoin que la grille uniforme ne couvre pas : **mélanger plusieurs
tailles de caméra au sein d'un même niveau** — par exemple une salle large pour une pièce
d'ensemble et des salles étroites pour un couloir, dans le même tableau. Il a aussi demandé que le
mode *suivi* puisse, comme le mode *par salle*, avoir sa taille de vue réglée par niveau plutôt que
de retenir en dur la taille de salle par défaut.

Choix explicite (voir « Décisions de cadrage », `epic.md`) : intégrer ce besoin **dans ce lot**
plutôt que d'ouvrir un nouveau lot après fusion, pour livrer un cadrage cohérent en une seule
passe. La solution retenue est la plus simple qui couvre le besoin réel : laisser le designer
**dessiner lui-même** des rectangles de caméra sur le canevas, comme il dessine déjà un rectangle
de tuiles (`EditorTool::Rectangle`) — pas de transition animée, pas de déclencheur, pas de système
de caméra scripté.

## Travail à réaliser
- **Modèle** (`Core`) : `core::CameraZone` (rectangle en tuiles), liste `zones` dans
  `core::CameraFramingConfig`, valide uniquement en mode *par salle*, entièrement dans les bornes
  du niveau, de taille non nulle. Liste vide par défaut : comportement de grille automatique
  inchangé tant qu'aucune zone n'est dessinée.
- **Sérialisation** : `LevelLoader`/`LevelWriter` lisent/écrivent `cameraFraming.zones` ; round-trip
  exact, dans l'ordre du fichier.
- **Résolution de zone active** (`HMI`, pur, sans GPU) : `hmi::activeCameraZoneIndex` — la
  **première** zone de la liste qui contient la position du personnage l'emporte en cas de
  chevauchement (même convention que la superposition des décors) ; aucune zone ne la contient →
  repli sur le **niveau entier**, jamais un état indéfini.
- **Intégration `GameSession`** : au chargement et à chaque pas, la zone active est résolue comme
  la salle active de la grille automatique (`_currentRoomIndex`) l'est déjà ; le cadrage appliqué
  (centre, zoom) bascule entre zone dessinée, repli niveau entier, ou grille automatique selon que
  `zones` est vide ou non — un seul point d'application (`applyCameraFraming`), comme pour les
  trois modes de base.
- **Taille de suivi réglable** : le mode *suivi* réutilise `roomWidthTiles`/`roomHeightTiles` (même
  champs que le mode *par salle*) au lieu de la constante par défaut codée en dur, avec repli sur
  cette même constante si non déclarée.
- **Outil d'édition dédié** (`EditorTool::CameraZone`) : glisser sur le canevas (même geste que
  `Rectangle`) ajoute une zone en fin de liste au relâchement, annulable
  (`core::LevelDraft::addCameraZone`). Icône, entrée de catalogue d'actions, bijection avec
  `EditorTool` — même intégration que tout autre outil du projet, pas un cas particulier.
- **Panneau « Cadrage »** : tableau récapitulatif des zones (bornes en tuiles), bouton « Retirer »
  pour la ligne sélectionnée (`core::LevelDraft::removeCameraZone`, annulable) — les zones se
  **dessinent** sur le canevas mais se **retirent** depuis le panneau, comme la section « Objets ».
- **Prévisualisation** (`DraftRenderer`) : en mode *par salle* avec zones non vide, les rectangles
  dessinés remplacent entièrement l'affichage de la grille automatique.

## Fichiers impactés
- `Source/Core/Levels/CameraFraming.{h,cpp}` — `CameraZone`, champ `zones`, validation étendue.
- `Source/Core/Levels/LevelLoader.cpp`, `LevelWriter.cpp` — sérialisation des zones.
- `Source/Core/Levels/LevelDraft.{h,cpp}` — `addCameraZone`/`removeCameraZone`, annulables.
- `Source/HMI/Graphics/CameraZones.{h,cpp}` (nouveau) — `activeCameraZoneIndex`, pur, sans GPU.
- `Source/HMI/Game/GameSession.{h,cpp}` — résolution de zone active, taille de suivi réglable.
- `Source/HMI/Editor/EditorTool.h` — `EditorTool::CameraZone`.
- `Source/HMI/Interface/{IconGeometry,ActionCatalog}.{h,cpp}` — icône et entrée de catalogue.
- `Source/HMI/Editor/EditorStatus.cpp` — libellé et aide de la barre d'état.
- `Source/HMI/Game/GameViewport.{h,cpp}` — geste de dessin, `removeCameraZone`.
- `Source/HMI/Editor/TexturePanel.{h,cpp}`, `Source/Elements/UI/TexturePanel.ui` — tableau des
  zones dans la section « Cadrage ».
- `Source/HMI/Graphics/DraftRenderer.cpp` — prévisualisation des zones dessinées.
- `Source/Elements/Localization/{fr,en}.lang` — libellés de l'outil, de l'aide et du tableau.
- Tests unitaires étendus/nouveaux dans `Source/Test/Unit/Core/Levels/` et
  `Source/Test/Unit/HMI/{Graphics,Editor,Interface}/`, système dans
  `Source/Test/Systeme/test_parcours_edition.cpp`.

## Tests (obligatoires)
- Validation des zones : mauvais mode, largeur/hauteur nulle, position négative, dépassement du
  niveau, zones chevauchantes valides.
- Round-trip de sérialisation des zones, dans l'ordre exact du fichier.
- `addCameraZone`/`removeCameraZone` annulables ; retrait hors bornes sans effet.
- Résolution de zone active : zone unique, hors de toute zone, bornes inclusives/exclusives,
  chevauchement (la première de la liste gagne, dans les deux ordres), liste vide.
- Bijection des sept outils d'édition avec `EditorTool` (`ActionCatalog`).
- Traductions des nouvelles clés présentes dans les deux catalogues (`fr`/`en`).
- Parcours système complet : dessiner deux zones, retirer l'une par erreur puis annuler,
  enregistrer, recharger — les deux zones sont restituées dans le même ordre.
- Taille de suivi réglable : validée pour `Follow` comme pour `PerRoom` ; round-trip de
  sérialisation.
- Tests `Core`/`HMI` purs, sans GPU, comme le reste du lot.

## Points d'attention
- **Compatibilité inchangée.** `zones` est vide par défaut : tout niveau existant, y compris les
  quinze tableaux livrés, garde exactement son comportement de grille automatique.
- **Priorité déterministe.** Le chevauchement de zones est résolu par ordre d'ajout, jamais par
  taille ou position — même convention que la superposition des décors, pas une nouvelle règle à
  retenir.
- **Un seul point de résolution.** `hmi::activeCameraZoneIndex` est le seul endroit qui décide
  quelle zone est active ; `GameSession` et `DraftRenderer` l'utilisent (ou l'ignorent pour la
  simple prévisualisation) sans dupliquer la règle.
- **Dessiner sur le canevas, retirer dans le panneau.** Cohérent avec la section « Objets » —
  éviter d'inventer un second geste de suppression sur le canevas pour cet outil précis.
- **Tout nouvel outil suit l'intégration complète du catalogue** (icône, entrée, bijection) : le
  projet ne connaît pas de raccourci « outil sans icône ».

## Définition de fait (DoD)
- Les zones de caméra dessinées à la main permettent de mélanger plusieurs tailles de caméra dans
  un même niveau en mode *par salle* ; la taille de la caméra de suivi est réglable par niveau ;
  compatibilité totale avec les niveaux sans zones ; tests `ctest` à 100 %, `/W4 /WX` propre,
  lints et Doxygen verts.

## Exigences
`EX-LVL-007` (zones de caméra dessinées à la main), `EX-REN-017` (taille de suivi réglable),
`EX-EDIT-029` (outil de dessin et tableau des zones dans l'éditeur) — déclarées ici ; réutilise
`EX-LVL-006` (mode de cadrage porté par le niveau), `EX-REN-015`/`EX-REN-016` (cadrage par salle et
trois modes), `EX-EDIT-005` (annulation), `EX-EDIT-010` (réutilisation du modèle de `Core`),
`EX-NFR-004` (vérification sans GPU), `EX-NFR-040` (erreur récupérable).
