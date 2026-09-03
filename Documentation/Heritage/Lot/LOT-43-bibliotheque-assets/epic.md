# LOT-43 — Bibliothèque d'assets : vignettes, import, rechargement à chaud {#lot-43}

> Statut : **fait**. Prérequis : [LOT-40](@ref lot-40) (*TextureCache* et son invalidation),
> [LOT-42](@ref lot-42) (panneau « Textures »).

## Objectif
Rendre le travail d'habillage **supportable** avant qu'il ne commence vraiment. Tant que ce lot
n'est pas livré, chaque retouche d'une texture impose de passer par l'explorateur de fichiers puis
de **relancer l'application** — c'est-à-dire à chaque itération de LOT-44, LOT-45, LOT-47, LOT-48,
LOT-49 et LOT-54.

C'est la raison pour laquelle ce lot est placé **avant** les couches visuelles restantes, et non
après comme le prévoyait le cadrage initial : c'est un multiplicateur de vitesse sur tout ce qui
suit, pas un confort de fin de programme.

## Périmètre

### Inclus
- **Widget de vignettes partagé**, utilisé par les sections du panneau « Textures » (Skins pour
  l'instant, puis Fond, Objets, Animations, Décors) **en place**, sans changer l'identité ni le
  câblage du panneau ni son modèle de données : `hmi::decodeImageFile` → `QPixmap`, indépendant du
  *TextureCache* D3D11 — deux consommateurs du même décodeur CPU, pas de nouveau décodeur.
- **Gestion des fichiers d'assets** (`EX-EDIT-026`), sans quitter l'application :
  - **importer** une image externe dans `Assets/{Skins,Backgrounds,Objects,Player,Decors}/`,
  - **renommer**, **dupliquer**, **supprimer**,
  - avec **avertissement explicite** quand l'asset visé est référencé par un niveau ou par un jeu de
    skins — la référence se fait par **nom de fichier**, renommer ou supprimer casse donc
    silencieusement les niveaux si rien ne prévient.
- **Rechargement à chaud** : action « Recharger les textures » (ou surveillance de dossier) qui
  invalide le *TextureCache* via `invalidate`/`invalidateAll` (API prévue en LOT-40) et régénère les
  vignettes, sans redémarrer l'application.

### Exclus (hors périmètre de ce lot)
- Aucun nouveau modèle de données, aucune nouvelle donnée `Level`/`skins.json` — pure consolidation
  d'UI et d'outillage sur les données introduites par LOT-42.
- Édition du contenu d'une image : c'est l'atelier pixel art (LOT-54), qui se branchera sur ce
  widget comme point d'entrée.
- Réécriture automatique des références lors d'un renommage : on **avertit**, on ne migre pas.
  Migrer supposerait de rouvrir et réécrire tous les niveaux, ce qui est une opération à part
  entière, non justifiée à ce stade.

## Décisions de cadrage
- **Placé tôt, délibérément** : le cadrage initial plaçait ce lot en avant-dernière position. Le
  rechargement à chaud conditionne la vitesse d'itération de six lots ; le livrer après eux
  reviendrait à payer l'intégralité du coût qu'il évite.
- **Évolution en place, pas un nouveau panneau** : réutilise le dock « Textures » créé en LOT-42 —
  cohérent avec la décision transverse du programme (« un seul panneau, pas trois jetables »).
- **Avertir plutôt que migrer** (cf. Exclus) : décision explicite, pour que le comportement ne soit
  pas pris pour un oubli.
- **Décodage CPU et cache GPU restent séparés** : les vignettes passent par `QPixmap`, jamais par
  une texture D3D11 — pas de dépendance de l'UI au device graphique.

## Exigences couvertes
- Nouvelle : `EX-EDIT-026` (gestion des fichiers d'assets et rechargement à chaud).
- Réutilisées : `EX-EDIT-042`/`EX-EDIT-024` (sections concernées), `EX-REN-041` (décodage image),
  `EX-REN-033` (traduction), `EX-NFR-040` (repli).

## Découpage

> État : ✅ fait · 🔄 en cours · ⬜ non commencé.

| Tâche | Intitulé | Emplacement | État |
|-------|----------|-------------|:----:|
| [TACHE-01](tache-01-widget-vignettes.md) | Widget de vignettes partagé (décodage CPU → `QPixmap`) | `Source/HMI/Editor` | ✅ |
| [TACHE-02](tache-02-gestion-fichiers.md) | Import, renommage, duplication, suppression + détection des références | `Source/HMI/Editor` | ✅ |
| [TACHE-03](tache-03-rechargement-chaud.md) | Rechargement à chaud (invalidation *TextureCache* + régénération des vignettes) | `Source/HMI/Graphics`, `Source/HMI/Editor` | ✅ |

## Critères d'acceptation du lot
1. Les sections du panneau « Textures » affichent des vignettes au lieu d'une liste texte, sans
   changement de comportement fonctionnel.
2. Importer une image externe la rend immédiatement assignable, sans manipulation dans l'explorateur
   de fichiers.
3. Renommer ou supprimer un asset référencé par un niveau ou un jeu de skins déclenche un
   avertissement nommant les références concernées.
4. Une action « Recharger » reflète un asset modifié sur disque sans redémarrer l'application.
5. La détection des références est testée sans GPU.
6. Build `/W4 /WX`, Doxygen, lint verts.

## Dépendances
Bâtit sur [LOT-40](@ref lot-40) (*TextureCache*, `invalidate`) et [LOT-42](@ref lot-42) (panneau
« Textures », `skins.json`). Réutilise *TextureLoader*/`hmi::AssetPaths` (LOT-39). Accélère
[LOT-44](@ref lot-44) à [LOT-49](@ref lot-49) ; prépare [LOT-54](@ref lot-54) (point d'entrée
« ouvrir dans l'atelier »).

## Navigation des tâches
- @subpage lot-43-tache-01-widget-vignettes
- @subpage lot-43-tache-02-gestion-fichiers
- @subpage lot-43-tache-03-rechargement-chaud
