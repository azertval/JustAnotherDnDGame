# LOT-60 — Audio : socle et bruitages {#lot-60}

> Statut : **code, tests et documentation faits ; essai manuel humain final restant** (voir État
> de [TACHE-05](tache-05-documentation-verification.md)). Prérequis : [LOT-59](@ref lot-59)
> (écrans de pause et de fin de niveau — un bruitage de victoire a besoin d'un moment où exister),
> [LOT-58](@ref lot-58) (une nouvelle dépendance tierce arrive après le durcissement, pas avant).

## Objectif
Donner un son au jeu. Sauter, atterrir, dasher, actionner un interrupteur, mourir, gagner : chacun
de ces événements est aujourd'hui **totalement silencieux**.

`EX-REN-040` (« ⚠️ souhaité ») demande des bruitages depuis les premières spécifications, et
`vision.md` place explicitement les « bruitages simples » **dans** le MVP — seule la « bande-son
musicale élaborée » en est écartée. Les dossiers `Source/HMI/Audio/` et `Source/Elements/Audio/`
existent depuis le `LOT-04` et ne contiennent qu'un `README.md`.

L'absence de son est, avec l'absence d'effets (`LOT-53`), ce qui distingue le plus nettement le
rendu actuel de celui d'un jeu fini — d'autant que tout le travail de *game feel* des `LOT-09` à
`LOT-11` reste sans confirmation sensorielle.

## Périmètre

### Inclus
- **Socle audio dans `HMI`** : lecture d'échantillons courts à faible latence, plusieurs sons
  simultanés, volume global.
- **Bibliothèque : Qt Multimedia** (`Qt6::Multimedia`, `QSoundEffect`) — décision arrêtée, cf.
  décisions de cadrage.
- **Bruitages de `EX-REN-040`** : saut, atterrissage, dash, interrupteur, porte, plaque de pression,
  ramassage, mort, victoire de tableau, fin de séquence, et les sons d'interface (déplacement dans
  un menu, validation).
- **Déclenchement sur les transitions d'état** déjà exposées par `core::Player` et les contrôleurs
  de mécanismes — les **mêmes** que celles que `LOT-53` utilisera pour les particules.
- **Réglage de volume** dans `hmi::OptionsPage`, persisté comme le V-Sync et la langue.
- **Dégradation propre** : aucun périphérique audio, ou fichier de son manquant, laisse le jeu
  parfaitement jouable (`EX-NFR-040`).

### Exclus (hors périmètre de ce lot)
- Musique de fond, boucles musicales, transitions musicales — hors MVP par `vision.md`.
- Audio spatialisé, atténuation par la distance, réverbération, effets.
- Mixage par catégorie (un volume global suffit ; séparer effets et musique n'a de sens qu'avec de
  la musique).
- Édition de sons dans l'éditeur, assignation d'un son par tuile ou par mécanisme : les sons sont
  attachés aux **événements**, pas au contenu.
- Sons attachés aux décors ou à l'ambiance.

## Décisions de cadrage
- **Qt Multimedia, pour la cohérence du choix de bibliothèques.** L'application est déjà
  intégralement Qt depuis le `LOT-38` : fenêtre, widgets, thème, chargement d'images, persistance
  des réglages, et jusqu'au décodage PNG de l'atelier pixel art. Ajouter une bibliothèque audio
  tierce (miniaudio, ou XAudio2 directement) introduirait un second modèle de ressources, un second
  cycle de vie et un second style d'API pour un besoin que Qt couvre. `QSoundEffect` est
  précisément conçu pour des échantillons courts à faible latence — le cas d'usage exact d'un
  bruitage de saut — et `windeployqt` sait déjà déployer ses bibliothèques, ce qui laisse le
  processus de release inchangé.
- **Le coût assumé** est un module Qt de plus à provisionner : `qtmultimedia` doit être ajouté à
  l'installation Qt de la CI (`install-qt-action`) et le zip de release grossit. C'est un coût
  connu, encadré par `EX-BUILD-010`, qui traite déjà exactement ce sujet pour Qt.
- **Aucun son dans `Core`.** La simulation reste pure, déterministe et testable sans périphérique
  (`EX-NFR-010`, `EX-ARCH-012`). `Core` **expose des transitions d'état** ; c'est `HMI` qui décide
  qu'une transition fait du bruit. La règle est la même que pour le rendu, et pour la même raison.
- **Les déclencheurs sont partagés avec `LOT-53`.** Son et particules réagissent aux mêmes
  événements ; les écrire deux fois, c'est garantir qu'ils divergeront. Ce lot pose la lecture des
  transitions, `LOT-53` la réutilise — d'où son exécution **après** celui-ci.
- **Volume global unique.** Un mixage par catégorie sans musique reviendrait à régler une seule
  catégorie sous deux noms.
- **Le silence n'est jamais une erreur bloquante.** Machine sans carte son, périphérique occupé,
  fichier absent : le jeu se joue. C'est la politique d'erreurs du projet (`EX-NFR-040`), déjà
  appliquée aux textures manquantes par le repli procédural.

## Exigences couvertes
- **Levée** : `EX-REN-040` (⚠️ souhaité → livré) — bruitages de saut, interrupteur, victoire, échec.
- Nouvelles : `EX-REN-047` (socle audio dans `HMI`, déclenché par les transitions d'état de `Core`,
  sans effet sur la simulation), `EX-REN-048` (volume réglable et persisté ; absence de son ou de
  périphérique traitée en erreur récupérable).
- Réutilisées : `EX-BUILD-010` (provisionnement reproductible d'une dépendance non gérable par
  `FetchContent`), `EX-ARCH-012` (présentation sans effet sur la simulation), `EX-NFR-010`
  (`Core` testable sans périphérique), `EX-NFR-040` (erreur récupérable), `EX-NFR-002`
  (déterminisme), `EX-IHM-001` (interface hors-jeu en Qt), `EX-NFR-031` (version épinglée).

## Découpage

> État : ✅ fait · 🔄 en cours · ⬜ non commencé.

| Tâche | Intitulé | Emplacement | État |
|-------|----------|-------------|:----:|
| [TACHE-01](tache-01-socle-qt-multimedia.md) | Provisionnement de Qt Multimedia et socle de lecture | `Source/HMI/Audio`, `.github/workflows` | ✅ |
| [TACHE-02](tache-02-catalogue-sons.md) | Catalogue de sons piloté par données et repli silencieux | `Source/HMI/Audio`, `Source/Elements/Audio` | ✅ |
| [TACHE-03](tache-03-declencheurs.md) | Déclencheurs depuis les transitions d'état du jeu et de l'IHM | `Source/HMI/Game`, `Source/HMI/Interface` | ✅ |
| [TACHE-04](tache-04-volume-options.md) | Réglage de volume persisté dans les options | `Source/HMI/Interface` | ✅ |
| [TACHE-05](tache-05-documentation-verification.md) | Documentation et vérification | `Source/Test`, `Documentation` | ✅ |

## Critères d'acceptation du lot
1. Sauter, atterrir, dasher, actionner un mécanisme, mourir et terminer un tableau produisent chacun
   un son distinct et reconnaissable.
2. La latence entre l'événement simulé et le son est imperceptible à l'essai manuel.
3. Le volume se règle dans les options, prend effet immédiatement, et est retrouvé au lancement
   suivant.
4. Sur une machine **sans périphérique audio**, ou avec le dossier de sons **vide**, le jeu se lance
   et se joue normalement — seul un avertissement est journalisé.
5. **Aucune dépendance audio dans `Core`** : les tests unitaires, d'intégration et système se
   construisent et s'exécutent sans Qt Multimedia, inchangés.
6. Le zip de release démarre sur une machine sans Qt installé, sons compris.
7. Build `/W4 /WX`, `ctest` à 100 %, Doxygen et lints verts.

## Dépendances
Bâtit sur [LOT-59](@ref lot-59) (écrans de pause et de fin de niveau, où vivent les sons de
victoire et de navigation) et [LOT-38](@ref lot-38) (application Qt, options persistées). Suit
[LOT-58](@ref lot-58). Prérequis de [LOT-53](@ref lot-53), qui réutilise ses déclencheurs.

## Navigation des tâches
- @subpage lot-60-tache-01-socle-qt-multimedia
- @subpage lot-60-tache-02-catalogue-sons
- @subpage lot-60-tache-03-declencheurs
- @subpage lot-60-tache-04-volume-options
- @subpage lot-60-tache-05-documentation-verification
