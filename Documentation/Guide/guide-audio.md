# Audio {#guide-audio}

Cette page explique comment le jeu produit du son : le socle de lecture (`hmi::AudioEngine`), le
catalogue qui associe un nom d'événement à un fichier (`hmi::SoundCatalog`), la détection des
transitions qui déclenchent un son (`hmi::GameEvents`) et la table qui les relie
(`hmi::SoundTriggers`). Ajouté en `LOT-60`, après le durcissement (`LOT-58`) et la boucle de jeu
complète (`LOT-59`, @ref guide-ecrans) dont les écrans de pause et de fin de niveau donnent enfin un
moment où un son de victoire ou de navigation peut exister.

## La règle d'or, une fois de plus

`Core` **expose des transitions d'état** ; c'est `HMI` qui décide qu'une transition fait du bruit.
Exactement la même séparation que pour le rendu (@ref guide-rendu, `hmi::MechanismVisuals`) — et
pour la même raison : la simulation reste pure, déterministe et testable **sans périphérique
audio** (`EX-NFR-010`, `EX-ARCH-012`). Aucun fichier de `Core/` n'inclut Qt, ni ne sait qu'un son
existe.

## Le socle : `hmi::AudioEngine`

`Source/HMI/Audio/AudioEngine.h` enveloppe `QSoundEffect` (Qt Multimedia), le composant Qt conçu
précisément pour des échantillons courts à faible latence — le cas d'usage exact d'un bruitage de
saut, par opposition à `QMediaPlayer` (pensé pour la musique, latence et coût plus élevés).

- **Détection du périphérique** à la construction (`QMediaDevices::defaultAudioOutput()`) : absent
  → le moteur passe en état **muet**, journalise un avertissement une seule fois, et **toute**
  demande de lecture ultérieure ne fait rien. Un constructeur dédié (`ForceMuted::Yes`) force cet
  état sans dépendre du matériel réel de la machine qui exécute les tests.
- **Préchargement obligatoire.** `QSoundEffect` charge son fichier de façon **asynchrone** : jouer
  immédiatement après construction ne produit rien. `AudioEngine::preload(id, fichier)` doit donc
  être appelé au démarrage pour chaque son du catalogue — jamais au premier déclenchement, sous
  peine d'un premier saut de la partie silencieux (défaut difficile à attribuer, puisque les sauts
  suivants fonctionnent).
- **Tourniquet d'instances.** `preload` prépare en réalité `MAX_INSTANCES_PER_EVENT` (3)
  `QSoundEffect` identiques par identifiant, et `play` les consomme en rotation : un même événement
  déclenché en rafale (dash contre un mur, sauts répétés) se recouvre sans s'interrompre lui-même
  de façon audible, sans empiler indéfiniment des lectures superposées.
- **Volume borné** à `[0, 1]`, appliqué à tous les échantillons préchargés.

## Le catalogue : `hmi::SoundCatalog`

`Source/HMI/Audio/SoundCatalog.h` résout un identifiant d'événement (`"saut"`, `"atterrissage"`…)
vers un nom de fichier relatif à `Source/Elements/Audio/`, lu depuis `sounds.json`. Le patron est
identique à `hmi::SkinCatalog` (@ref guide-rendu, `LOT-42`) et `hmi::AnimationCatalog` (`LOT-46`) :

- Un fichier **absent** produit un catalogue **vide** — cas légitime (poste de développement sans
  les assets), pas une anomalie.
- Une entrée **malformée** fait échouer le chargement **entier**, avec un code d'erreur
  programmatique (`SoundCatalogError`) — jamais un catalogue deviné ou partiellement chargé. C'est
  la même garantie que les deux catalogues cités, vérifiée dans leur code au moment d'écrire celui
  des sons (la documentation de cadrage affirmait initialement l'inverse — « ignore une entrée
  malformée » — corrigée après lecture du code réel).
- Logique **pure**, aucune dépendance Qt/périphérique : la résolution de chemin est séparée de la
  lecture, qui appartient à `AudioEngine`. Testable sans carte son.

Le repli d'un événement **sans entrée** ou d'un fichier **manquant** est le silence, jamais un son
de remplacement — contrairement au damier magenta d'une texture manquante (@ref guide-rendu), un
bip de repli serait insupportable en jeu.

## Détection des transitions : `hmi::GameEvents`

`Source/HMI/Game/GameEvents.h` détecte, une fois par **pas de simulation fixe** (jamais par image
de rendu, `EX-REN-021` — le piège que le `LOT-33` a déjà traité pour les entrées, @ref
guide-entrees), les transitions qui produisent un événement (`hmi::GameEvent`) :

- **`detectPlayerEvents`** compare un `PlayerEventState` (sous-ensemble pertinent de `core::Player`)
  au pas précédent et au pas courant : `Jumped`, `Landed`, `Dashed`, `WallContactEnter`. Aucune de
  ces transitions n'était triviale à détecter depuis les champs existants — en particulier, aucune
  combinaison de `jumpBufferTimer`/`coyoteTimer`/`airJumpsRemaining` ne distinguait fiablement « un
  saut vient de se déclencher » d'« un buffer a simplement expiré sans saut ». `core::Player` porte
  donc désormais un champ `justJumped` (front explicite, remis à faux en tout début de pas dans
  `CharacterPhysicsSystem::resolveVelocity`), sur le même principe que `dashTimer` : c'est `Core`,
  seul à savoir qu'un saut vient de se déclencher, qui l'expose — plutôt que de faire deviner la
  transition à `HMI` à partir de signaux ambigus.
- **`detectMechanismEvents`** compare l'état `isDoorOpen(index)` de chaque mécanisme entre deux
  pas, avec un vecteur `isContinuous` (nouvel accesseur de `core::MechanismController`, la
  distinction interrupteur/plaque de pression n'était jusqu'ici que privée) pour choisir entre
  `SwitchToggled` et `PressurePlatePressed`/`PressurePlateReleased`. **Un seul événement par
  mécanisme transitionne** : la porte partage le booléen de son déclencheur mais n'émet jamais un
  second son — deux sons superposés pour une seule action physique.
- **`detectOutcomeEvent`** traduit `core::LevelOutcome` (@ref guide-niveaux) en `Died`/
  `LevelCompleted`.

`hmi::GameSession::lastStepEvents()` (@ref guide-ecrans) expose les événements du **dernier**
`update()` — capturés **avant** `reload()` : sur un échec, le rechargement remet aussitôt le
personnage et les mécanismes à l'état d'entrée, et une implémentation naïve qui viderait la liste
d'événements à cette occasion effacerait `Died` avant que l'appelant ne l'ait jamais vu (mort et
rechargement se suivent dans le **même** pas).

Les événements d'**interface** (`MenuNavigate`, `MenuConfirm`, `PauseOpened`,
`SequenceCompleted`…) partagent la même énumération mais ne sont **pas** détectés par diffusion
d'état : les écrans du `LOT-59` les exposent déjà comme des signaux Qt discrets
(`GameViewport::levelSucceeded`, la navigation manette des menus) — `MainWindow` y joue le son
associé directement, sans détection supplémentaire.

## La table : `hmi::SoundTriggers`

`hmi::soundForEvent` (`Source/HMI/Audio/SoundTriggers.h`) associe chaque `hmi::GameEvent` à un identifiant
de `hmi::SoundCatalog`, en un `switch` **exhaustif sans `default`** : un événement ajouté sans
entrée casse la compilation. Certains événements (`WallContactEnter`, `BlockPushed`, `PauseOpened`)
résolvent délibérément vers `std::nullopt` — silence documenté, pas un oubli, faute de bruitage
dédié dans ce lot.

**Détection et association sont deux questions séparées** (« qu'est-ce qui est arrivé » vs. « qu'est-ce
que ça produit ») précisément pour qu'un futur système de particules (`LOT-53`, `EX-REN-008`)
puisse brancher un effet visuel sur la **même** détection sans la dupliquer ni la modifier.

## Provisionnement : Qt Multimedia

`Qt6::Multimedia` est un **composant additionnel** de Qt, pas une bibliothèque tierce : ajouté à
`find_package(Qt6 ... COMPONENTS Multimedia)` dans `Source/HMI/CMakeLists.txt`, avec la même garde
que `Widgets`/`Gui` (absent → cible `JustAnotherDnDGame` ignorée, configuration jamais cassée,
`EX-BUILD-010`). Provisionné en CI via `modules: qtmultimedia` sur les six points
`install-qt-action` (`ci.yml`, `release.yml`) ; en local, via le composant `Multimedia` de
l'installateur Qt officiel ou `aqtinstall -m qtmultimedia`.

`windeployqt` (`POST_BUILD`) déploie `Qt6Multimedia.dll` et ses greffons (`multimedia/`) à côté de
l'exécutable — vérifié explicitement en CI (job `build-test-coverage`), puisqu'un build local
réussi ne prouve rien sur le zip publié.

## Voir aussi
- `hmi::AudioEngine`, `hmi::SoundCatalog`, `hmi::GameEvents`, `hmi::SoundTriggers`.
- `core::Player::justJumped`, `core::MechanismController::isContinuous`.
- @ref guide-rendu — `hmi::SkinCatalog`/`hmi::MechanismVisuals`, les patrons repris ici.
- @ref guide-ecrans — `hmi::GameSession::update`/`lastStepEvents`, les écrans qui jouent les sons
  d'interface.
- @ref guide-entrees — la discipline « un événement par pas fixe » du `LOT-33`, reprise ici.
