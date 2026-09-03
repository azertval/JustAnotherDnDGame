# LOT-37 — Refonte IHM (Qt) : liens de mécanismes visuels (traits/flèches) {#lot-37}

> Statut : **fait**. Prérequis : [LOT-35](@ref lot-35) (éditeur Qt) ; complémentaire de
> [LOT-36](@ref lot-36).
>
> **Note (cadrage vs implémentation)** : ce lot a été cadré avant le `LOT-38` (retrait de l'éditeur
> « maison » historique). Les chemins mentionnés ci-dessous (`Source/Editor`, `EditorController`,
> `LINK_TINTS`, prédicats `isTriggerTile`/`isLinkTargetTile` du legacy) appartenaient à ce code
> **supprimé** ; l'implémentation réelle vit dans l'arborescence Qt actuelle : géométrie/gestes
> purs dans `Source/HMI/Editor/{LinkGeometry,LinkGesture}.*`, rendu dans
> `Source/HMI/Graphics/DraftRenderer.cpp` (+ primitive `hmi::LineQuad` dans `SpriteBatch`), geste
> interactif dans `Source/HMI/Game/GameViewport.*` via un **outil dédié** `EditorTool::Link`
> (panneau Outils) plutôt qu'un raccourci `Maj+clic`, et panneau dans
> `Source/HMI/Editor/LinkPanel.*`. Voir @ref guide-editeur pour la description à jour.

## Objectif
Rendre **lisible et éditable** la liaison des interrupteurs, principal point de douleur cité par le
demandeur (« ce qui est actuel rend complexe le lien des interrupteurs »). Aujourd'hui, faute de
primitive de ligne, une paire déclencheur→cible est indiquée par une **simple teinte de case**
(`LINK_TINTS`) : au-delà de quelques liens, illisible. Ce lot introduit un **rendu explicite par
traits/flèches** reliant chaque déclencheur à sa/ses cible(s), plus un **panneau « Liens »** listant
et éditant les liaisons.

## Périmètre

### Inclus
- **Rendu des liens par traits/flèches** dans le viewport : chaque `Mechanism` (interrupteur/plaque →
  porte) et `DangerLink` (déclencheur → danger commuté) est dessiné comme une **flèche** du centre du
  déclencheur vers le centre de la cible, par-dessus la grille.
  - Nécessite une **primitive de ligne** : soit ajoutée au `SpriteBatch` (quads « fins » orientés,
    comme déjà fait pour la grille de repère), soit un overlay dédié. Décision au cadrage TACHE.
  - Code couleur / surbrillance au survol pour distinguer les paires ; la case de liaison en attente
    reste signalée.
- **Panneau « Liens »** (`QDockWidget`) : liste des liaisons du niveau (déclencheur → cible, avec
  coordonnées et type) ; sélectionner une entrée **met en surbrillance** le trait correspondant ;
  **supprimer** une liaison depuis le panneau.
- **Création/suppression de lien** on passe par un outil de sélection de déclencheur puis de cible ; la liaison est **créée immédiatement** et
  apparaît dans le viewport ; refaire la même paire la supprime (bascule conservée). Suppression de la mechanique de maj-clic
- Réutilise **intégralement** le modèle : `LevelDraft::linkMechanism` / `unlinkMechanism`, vecteurs
  `Mechanism` / `DangerLink`, distinction `isTriggerTile` / `isLinkTargetTile`.
- Documentation (guide éditeur — section liaisons) et tests de la logique nouvelle (géométrie des
  traits, appariement) découplée du GPU.

### Exclus (hors périmètre de ce lot)
- **Nouveau modèle de liaison** (identifiants persistés en mémoire, liaisons N↔N arbitraires) — le
  modèle actuel (une cible = un déclencheur, plusieurs cibles par déclencheur, résolu par position)
  est **conservé** ; seule sa **visualisation/édition** change.
- **Configuration des dangers avancés** (mover/blink : axe, période, phase) — reste éditée comme
  aujourd'hui ; ce lot cible la **liaison** déclencheur→cible, pas les paramètres de danger.
- **Textures, menus, gestion de fichiers** — LOT-39/38/36.

## Décisions de cadrage
- **Traits/flèches explicites, fin des teintes de case** : c'est la correction directe du point de
  douleur — une flèche montre sans ambiguïté « ce déclencheur ouvre cette porte », là où une teinte
  partagée obligeait à comparer des couleurs. Le rendu par teinte est retiré (ou conservé en appoint
  léger, à trancher).
- **Ajouter une primitive de ligne au pipeline plutôt que dépendre de Qt pour l'overlay** : garder le
  dessin des liens **dans le viewport D3D11** aligne au pixel sur la grille de jeu (un overlay Qt
  séparé se désynchroniserait du zoom/pan de la caméra). La primitive resservira (debug, sélection).
- **Le modèle de liaison ne change pas** : `Core` reste intact ; ce lot est purement présentation +
  interaction. Aucune migration de fichiers de niveaux.
- **Panneau « Liens » comme vue du modèle** : il **lit** `LevelDraft` et déclenche
  `link`/`unlinkMechanism` ; aucune duplication de l'état des liaisons.

## Exigences couvertes
- Nouvelles : `EX-IHM-030` (liens de mécanismes rendus par traits/flèches explicites),
  `EX-IHM-031` (panneau listant/éditant les liaisons du niveau).
- Reconduite (présentation, sémantique conservée) : `EX-EDIT-003` (liaison déclencheur → cible).
- Réutilisées : `EX-EDIT-010` (pas de duplication de la logique de niveau), `EX-NFR-010` (géométrie
  des traits testable sans GPU).

## Découpage

> État : ✅ fait · 🔄 en cours · ⬜ non commencé. Les tâches seront détaillées à l'ouverture du lot.

| Tâche | Intitulé | Emplacement | État |
|-------|----------|-------------|:----:|
| [TACHE-01](tache-01-primitive-ligne-geometrie.md) | Primitive de ligne/flèche (pipeline) + géométrie des traits (logique testable) | `Source/HMI/Graphics`, `Source/HMI/Editor` | ✅ |
| [TACHE-02](tache-02-rendu-liens-creation.md) | Rendu des liens + création avec retour visuel immédiat dans le viewport | `Source/HMI/Editor`, `Source/HMI/Game` | ✅ |
| [TACHE-03](tache-03-panneau-liens-doc.md) | Panneau « Liens » (liste, surbrillance, suppression) ; doc & vérification | `Source/HMI/Editor`, `Source/HMI/Interface`, `Documentation` | ✅ |

## Critères d'acceptation du lot
1. Chaque liaison déclencheur→cible du niveau est dessinée comme un **trait/flèche explicite** dans le
   viewport, aligné sur la grille au zoom/pan courant.
2. Créer une liaison montre un **trait provisoire** pendant le geste ; la liaison créée apparaît
   immédiatement ; refaire la même paire la supprime (bascule conservée).
3. Le panneau « Liens » **liste** toutes les liaisons, **met en surbrillance** celle sélectionnée et
   permet d'en **supprimer** une ; la vue et le viewport restent cohérents.
4. Le modèle de liaison (`Mechanism`/`DangerLink`) et la sérialisation sont **inchangés** ; un niveau
   enregistré est identique à ce qu'il aurait été avant ce lot pour les mêmes liaisons.
5. La géométrie des traits et l'appariement sont **couverts par des tests** sans GPU.
6. Build `/W4 /WX`, Doxygen et lint verts ; rendu **vérifié visuellement**.

## Dépendances
- Bâtit sur [LOT-35](@ref lot-35). Réutilise `LevelDraft::linkMechanism`/`unlinkMechanism` et les
  structures `Mechanism`/`DangerLink` (`LOT-14`/`19`/`31`) et le rendu de grille du viewport. Peut
  ajouter une primitive de ligne à `hmi::SpriteBatch` (`LOT-05`). Ne modifie pas `Core`.

## Navigation des tâches
- @subpage lot-37-tache-01-primitive-ligne-geometrie
- @subpage lot-37-tache-02-rendu-liens-creation
- @subpage lot-37-tache-03-panneau-liens-doc
