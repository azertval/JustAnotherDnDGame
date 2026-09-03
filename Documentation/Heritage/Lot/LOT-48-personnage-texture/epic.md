# LOT-48 — Personnage : skin et animations depuis fichier {#lot-48}

> Statut : **fait**. Prérequis : [LOT-46](@ref lot-46) (moteur d'animation),
> [LOT-43](@ref lot-43) (bibliothèque d'assets).

## Objectif
Habiller le sprite **le plus regardé du jeu**, resté hors du cadrage initial du programme.

Le personnage est aujourd'hui dessiné par *hmi::playerPixel* : une silhouette humanoïde composée de
rectangles, générée en C++ dans `ProceduralAtlas.cpp`, avec sept images réparties sur trois clips.
Modifier son apparence suppose de recompiler. Et surtout, le gameplay a acquis depuis longtemps des
états que le visuel n'a jamais suivis : double saut, glissade murale, saut mural (`LOT-10`), chute
rapide et suspension à l'apex (`LOT-11`), dash directionnel — tous rendus par la même image de saut.

## Périmètre

### Inclus
- **Spritesheet externe** (`Assets/Player/`), chargée par le *TextureCache* et décrite par un
  `nom-asset.anim.json` (LOT-46), avec **repli sur l'atlas procédural** si le fichier est absent
  (`EX-REN-042`, patron déjà établi par LOT-39).
- **Clips couvrant les états réellement livrés** : repos, course, saut, **chute** distincte du saut,
  **atterrissage**, **glissade murale**, **dash**. Chaque clip est une projection de l'état exposé
  par `core::Player`/`core::Velocity` (`grounded`, `wallDirection`, `dashTimer`, signe de la vitesse
  verticale), dans le prolongement direct de l'actuel `targetClip()`.
- **Orientation gauche/droite** à partir de `core::Player::facing`, par retournement horizontal des
  coordonnées de texture — l'artiste ne dessine qu'un sens.
- **Découplage taille d'image / boîte de collision** : la contrainte historique « la région du
  personnage doit être carrée » (régression corrigée en `LOT-17`) tombe. La spritesheet déclare sa
  taille d'image ; `core::playerSize()` (0,4 × 0,8 unité, `Source/Core/Physics/PlayerSpawn.h`) reste
  **la seule** source de vérité de la hitbox, et un point d'ancrage explicite relie les deux. Une
  image plus grande que la hitbox (cape, cheveux, effet de dash) devient possible sans toucher au
  gameplay.

### Exclus (hors périmètre de ce lot)
- Toute modification de la physique ou de la boîte de collision : le changement d'apparence ne doit
  **rien** changer au ressenti ni aux niveaux existants.
- Effets accompagnant les mouvements (traînée de dash, poussière) : LOT-53.
- Plusieurs personnages ou skins de personnage sélectionnables : un seul, remplaçable par son
  fichier.
- Retrait de l'atlas procédural : il reste le repli et la référence de LOT-39.

## Décisions de cadrage
- **La hitbox ne bouge pas.** C'est l'invariant du lot. Le point d'ancrage image ↔ hitbox est une
  donnée de la spritesheet, jamais l'inverse : aucun fichier d'asset ne doit pouvoir modifier la
  physique (`EX-ARCH-012`), et les cent quatre-vingts tests de gameplay existants doivent passer
  sans modification.
- **Nouveaux clips maintenant, pas plus tard** : ajouter la chute, l'atterrissage, la glissade
  murale et le dash coûte quelques branches dans une fonction déjà existante et purement
  déterministe. Livrer une spritesheet externe sans ces états reviendrait à externaliser le problème
  au lieu de le résoudre.
- **Repli procédural conservé** : le jeu doit rester jouable et lisible sans aucun asset, exigence
  déjà tenue depuis LOT-39.
- **Un seul sens dessiné** : le retournement horizontal évite de doubler le travail de l'auteur.

## Exigences couvertes
- Nouvelle : `EX-REN-009` (personnage habillé depuis une spritesheet externe, clips couvrant les
  états de gameplay, taille d'image indépendante de la hitbox).
- Réutilisées : `EX-REN-005` (animations par données), `EX-REN-011`/`EX-REN-012` (sprites et
  animations), `EX-REN-042` (assets externalisés avec repli), `EX-ARCH-012` (rendu sans effet sur la
  simulation), `EX-GP-015`/`EX-GP-016`/`EX-GP-017` (états aériens à représenter), `EX-NFR-040`
  (repli).

## Découpage

> État : ✅ fait · 🔄 en cours · ⬜ non commencé.

| Tâche | Intitulé | Emplacement | État |
|-------|----------|-------------|:----:|
| [TACHE-01](tache-01-spritesheet-externe.md) | Spritesheet de personnage externe + repli procédural + point d'ancrage image/hitbox | `Source/HMI/Graphics` | ✅ |
| [TACHE-02](tache-02-nouveaux-clips.md) | Nouveaux clips (chute, atterrissage, glissade murale, dash) projetés depuis l'état de simulation | `Source/Core/Ecs/Systems`, `Source/HMI/Graphics` | ✅ |
| [TACHE-03](tache-03-orientation-non-regression.md) | Orientation par retournement horizontal + non-régression du gameplay | `Source/HMI/Graphics`, `Source/Test` | ✅ |

## Critères d'acceptation du lot
1. Remplacer le fichier de spritesheet change l'apparence du personnage sans recompiler.
2. En l'absence de spritesheet, le personnage s'affiche comme aujourd'hui (repli procédural).
3. Chute, atterrissage, glissade murale et dash sont visuellement distincts du saut.
4. Le personnage regarde dans le sens de son déplacement.
5. La boîte de collision est **inchangée** : tous les tests de gameplay et de franchissabilité
   passent sans modification.
6. La projection état → clip est testée sans GPU ; build `/W4 /WX`, Doxygen, lint verts.

## Dépendances
Bâtit sur [LOT-46](@ref lot-46) (clips par données) et [LOT-43](@ref lot-43) (import d'assets).
Remplace l'habillage procédural de [LOT-17](@ref lot-17) et [LOT-18](@ref lot-18). Lit l'état de
[LOT-10](@ref lot-10) et [LOT-11](@ref lot-11) sans le modifier. Prépare [LOT-53](@ref lot-53).

## Navigation des tâches
- @subpage lot-48-tache-01-spritesheet-externe
- @subpage lot-48-tache-02-nouveaux-clips
- @subpage lot-48-tache-03-orientation-non-regression
