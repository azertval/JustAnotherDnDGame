# LOT-14 — Éditeur de niveaux intégré : édition de tuiles, mécanismes, essai immédiat {#lot-14}

> Statut : **terminé**. Ce lot remplace le placeholder « à venir » de `EditorScreen` par un
> **véritable mode éditeur**, intégré à l'application (`EX-EDIT-030`), permettant à un
> **non-développeur** de créer et modifier des niveaux **sans écrire de code** (`EX-EDIT-001`).

## Objectif
- Peindre la grille de tuiles (vide, solide, danger, entrée, sortie, interrupteur, porte) à la
  souris depuis une **palette**, en WYSIWYG (`EX-EDIT-002`).
- Placer l'**entrée**/la **sortie** et **relier visuellement** un interrupteur à sa porte
  (`EX-EDIT-003`/`004`).
- **Redimensionner** la grille et **annuler/refaire** (undo/redo, `EX-EDIT-005`).
- **Enregistrer/charger** au format réel du moteur, avec **validation** avant écriture et messages
  compréhensibles par un non-codeur (`EX-EDIT-006`/`007`, réutilisant le modèle et la validation de
  `Core` — `EX-EDIT-010`/`011`, aucune duplication).
- **Tester le niveau immédiatement** depuis l'éditeur (bascule vers le jeu, `EX-EDIT-008`).
- Documenter le **partage des niveaux** entre non-codeurs via une interface Git graphique
  (`EX-EDIT-022`).

Toute la logique de modèle/validation/sérialisation reste dans `Core`, pure et testable sans GPU
(`EX-NFR-010`) ; `HMI/Editor` n'orchestre que l'interaction souris et l'affichage, conformément à la
séparation `Core`/`HMI` de tout le moteur.

## Périmètre

### Inclus
- **Spec** : correction d'`EX-EDIT-006` dans `editeur-niveaux.md` — le format réellement défini par
  `EX-LVL-003` est du **JSON pur** (liste de tuiles-objets), pas le « format hybride ASCII + JSON »
  actuellement écrit dans la spec (texte obsolète, jamais implémenté ; `LevelLoader` ne charge que du
  JSON). Le brouillon est mis à jour pour refléter l'implémentation réelle.
- **Sérialisation (Core)** : un **écrivain** de niveau symétrique au `LevelLoader` (`Level` →
  JSON), aujourd'hui inexistant (seul le chargement existe) — brique indispensable à
  l'enregistrement.
- **Modèle d'édition (Core)** : une représentation **mutable** d'un niveau en cours d'édition
  (peindre une tuile, déplacer entrée/sortie, lier/délier un mécanisme, redimensionner), convertie
  en `core::Level` **validé** au moment de l'enregistrement (réutilise la validation `EX-LVL-004`).
- **Interaction (HMI/Editor)** : `EditorScreen` réel — grille cliquable (conversion souris → monde
  via `Camera2D`), palette de types de tuiles, peinture au clic/glisser, liaison interrupteur↔porte
  à la souris, redimensionnement, undo/redo (raccourcis clavier).
- **Essai immédiat** : bascule vers un écran de jeu chargeant le niveau **en cours d'édition** (pas
  nécessairement déjà enregistré sur disque), puis retour à l'éditeur.
- **Sélection du niveau à éditer** : nouveau niveau vierge ou fichier existant de
  `Source/Elements/Levels`.
- **Guide non-codeur** (`Documentation/Manuel/`) : publier/récupérer un niveau via une interface Git
  graphique (type GitHub Desktop), sans ligne de commande (`EX-EDIT-022`).

### Exclus (lots ultérieurs)
- **Décors & pipeline photo → pixel art** (`EX-DEC-*`) — explicitement post-MVP de l'éditeur dans la
  spec (§4bis de `editeur-niveaux.md`), livré après l'édition de tuiles de base.
- **Bloc poussable** (`EX-GP-022`) et **clé/porte verrouillée** (`EX-GP-023`) — non implémentés dans
  `Core` (exclus depuis LOT-12) : rien à éditer tant que le gameplay ne les supporte pas.
- **Édition collaborative temps réel** et **édition d'assets graphiques/sonores** — non-objectifs
  explicites de la spec (l'éditeur agence des tuiles existantes, il ne dessine pas les sprites).
- **Animation** d'ouverture de porte au-delà du retour visuel déjà existant (teinte, LOT-12).

## Décisions de cadrage
- **Où vit le modèle d'édition** : dans `Core` (nouveau, ex. `core::LevelEditor` ou extension de
  `Core/Levels`), **pas** en HMI — cohérent avec `EX-EDIT-010` (réutilisation, source unique de
  vérité) et le reste du moteur (logique dans `Core`, `HMI` orchestre). `core::Level` reste
  immuable une fois construit (design actuel, LOT-07) ; le modèle d'édition est un type **distinct**,
  qui se convertit vers un `Level` **validé** à l'enregistrement — pas de mutabilité ajoutée à
  `Level` lui-même.
- **Undo/redo** : pile de **snapshots complets** de l'état du brouillon (pas de delta par type de
  mutation), portée par `LevelDraft` (`Core`), donc testable sans GPU comme le reste de la logique
  du moteur — les niveaux du projet restent petits (≤ 14×8), la simplicité l'emporte sur l'économie
  mémoire d'un historique différentiel (arbitrage anticipé en TACHE-04, confirmé à l'implémentation).
- **Unicité entrée/sortie** : poser une nouvelle `Entry`/`Exit` **déplace** l'ancienne plutôt que
  de permettre plusieurs occurrences — cohérent avec la validation existante (`EX-LVL-004`,
  exactement une entrée et une sortie) et évite un état invalide en cours d'édition.
- **Portée écarté du gameplay non implémenté** : la palette de l'éditeur n'expose que les types de
  tuiles et mécanismes réellement gérés par `Core` aujourd'hui (`Empty`/`Solid`/`Danger`/`Entry`/
  `Exit`/`Switch`/`Door`) — pas de bloc poussable ni de clé, en cohérence avec le périmètre exclu.
- **`EX-EDIT-020`** (outil exécutable sans étape de build) est déjà satisfait par l'approche
  éditeur **intégré** actée dans la spec (`EX-EDIT-030`) : c'est le même exécutable du jeu, en mode
  éditeur — aucune tâche dédiée.
- **Essai immédiat, implémentation retenue** (précisée en TACHE-05) : une session de jeu
  **intégrée** — un `GameScreen` embarqué dans `EditorScreen`, piloté par délégation
  d'`update`/`render` — plutôt qu'une transition `ScreenManager` vers `ScreenId::Game`.
  `ScreenManager` détruit l'écran quitté à chaque transition (`_current = factory(target)`), ce qui
  aurait perdu le brouillon et l'historique undo/redo à chaque essai ; la délégation interne les
  préserve intégralement, sans changement d'architecture de `ScreenManager`/`ScreenTransition`.

## Exigences couvertes
- `EX-EDIT-001` à `EX-EDIT-008`, `EX-EDIT-010`, `EX-EDIT-011`, `EX-EDIT-020` à `EX-EDIT-022`,
  `EX-EDIT-030`, `EX-EDIT-031`.
- `EX-LVL-002`, `EX-LVL-003` (corrigée), `EX-LVL-004` (validation réutilisée), `EX-NFR-002`,
  `EX-NFR-010`, `EX-NFR-040`, `EX-ARCH-011`, `EX-ARCH-012`.

## Découpage

> État : ✅ fait · 🔄 en cours · ⬜ non commencé.

| Tâche | Intitulé | Emplacement | État |
|-------|----------|-------------|:----:|
| [TACHE-01](tache-01-serialisation-modele-edition.md) | Sérialisation JSON + modèle d'édition mutable | `Core/Levels` | ✅ |
| [TACHE-02](tache-02-ecran-editeur-palette.md) | Écran éditeur : grille cliquable + palette de tuiles | `HMI/Editor` | ✅ |
| [TACHE-03](tache-03-entree-sortie-mecanismes-redimension.md) | Entrée/sortie, liaison de mécanismes, redimensionnement | `HMI/Editor` | ✅ |
| [TACHE-04](tache-04-undo-redo.md) | Historique annuler/refaire | `Core/Levels`, `HMI/Editor` | ✅ |
| [TACHE-05](tache-05-enregistrement-validation-essai.md) | Enregistrement, validation, essai immédiat | `HMI/Editor`, `HMI/Interface` | ✅ |
| [TACHE-06](tache-06-integration-guide-non-codeur.md) | Intégration menu, tests système, guide non-codeur Git | `HMI`, `Documentation/Manuel` | ✅ |

## Critères d'acceptation du lot
1. Depuis le menu (« Mode Édition »), un non-codeur peut créer un **nouveau** niveau ou en **ouvrir**
   un existant, **peindre** la grille à la souris depuis une palette, placer entrée/sortie, **lier**
   un interrupteur à une porte, **redimensionner** la grille — sans écrire de code ni ligne de
   commande.
2. **Annuler/refaire** fonctionne sur les actions d'édition (tuiles, liaisons, redimensionnement).
3. **Enregistrer** un niveau invalide (pas d'entrée/de sortie, liaison non résolue) est **refusé**
   avec un message compréhensible ; un niveau **valide** s'enregistre au format réel du moteur et se
   **recharge à l'identique** (round-trip, `EX-EDIT-011`) — prouvé par test.
4. Un niveau en cours d'édition peut être **lancé dans le jeu** en un geste, puis l'éditeur
   **retrouvé** avec l'état d'édition intact.
5. Un guide non-codeur explique, sans ligne de commande, comment **publier**/**récupérer** un niveau
   via une interface Git graphique.
6. Logique de modèle/sérialisation/validation **couverte par des tests** (`ctest` vert),
   déterministe, sans GPU. Build `/W4 /WX` sans avertissement, Doxygen et `CHANGELOG.md` à jour,
   `lint` des exigences vert.

## Dépendances
- Réutilise le modèle et la validation de niveau (LOT-07), les mécanismes interrupteur↔porte
  (LOT-12), le rendu 2D et la caméra (LOT-05), les écrans/`ScreenManager` et le placeholder
  `EditorScreen` (LOT-06), le `GameScreen`/`LevelSequence` pour l'essai immédiat (LOT-09).

## Navigation des tâches
- @subpage lot-14-tache-01-serialisation-modele-edition
- @subpage lot-14-tache-02-ecran-editeur-palette
- @subpage lot-14-tache-03-entree-sortie-mecanismes-redimension
- @subpage lot-14-tache-04-undo-redo
- @subpage lot-14-tache-05-enregistrement-validation-essai
- @subpage lot-14-tache-06-integration-guide-non-codeur
