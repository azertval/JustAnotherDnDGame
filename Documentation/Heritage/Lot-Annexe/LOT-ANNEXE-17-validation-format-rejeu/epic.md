# LOT-ANNEXE-17 — Validation et stabilisation du format de rejeu {#lot-annexe-17}

> Statut : **fait**. Prérequis : [LOT-ANNEXE-07](@ref lot-annexe-07) (format de rejeu v1).
> Dernier lot avant le branchement du rejeu sur le jeu réel ([LOT-ANNEXE-18](@ref lot-annexe-18)) :
> stabilise le format v1 à la lumière de l'usage réel par les générations 2 et 3, et ajoute la
> validation à la lecture qui manquait jusqu'ici — un rejeu ne doit jamais être silencieusement
> invalide.

## Objectif
Deux limites de l'état actuel :
- **Le format de rejeu n'est vérifié qu'à l'écriture.** `LOT-ANNEXE-07` a défini le format v1 tôt
  (génération 1) pour que la génération 2 puisse déjà exporter dedans — séquence de
  `core::PlayerInput` par pas fixe, métadonnées, empreinte du niveau d'origine. Mais rien ne
  vérifie, à la **lecture**, que le fichier de niveau référencé existe encore et correspond
  toujours à l'empreinte enregistrée à l'export. Si le niveau a été modifié depuis (tuile
  déplacée, rééquilibrage), rejouer l'ancien fichier de rejeu dessus produirait une trajectoire
  incohérente au mieux, un plantage au pire — silencieusement, sans qu'aucun signal n'alerte
  l'utilisateur ni la CI. Ce lot est le préalable obligé avant de brancher le rejeu sur
  `hmi::GameSession` (`LOT-ANNEXE-18`) : le jeu réel ne doit jamais charger un rejeu invalide sans
  le dire.
- **Le format v1 a été figé avant tout usage réel.** Défini en génération 1, avant qu'aucun
  algorithme (génération 2/3) n'ait exporté le moindre fichier réel, il lui manque des champs dont
  le besoin n'est apparu qu'à l'usage — une durée totale pratique à afficher sans recompter les
  pas, une empreinte de l'algorithme utilisé pour savoir d'où vient un rejeu donné (utile dès
  `LOT-ANNEXE-19`, l'outillage CLI, et pour le garde-fou CI de `LOT-ANNEXE-20`).

## Périmètre

### Inclus
- **Empreinte de niveau** (`aisolver::computeLevelFingerprint`) : fonction pure calculant un hash
  déterministe du contenu du fichier de niveau, comparée à celle enregistrée dans le rejeu.
- **Validation à la lecture** (`aisolver::validateReplay`) : vérifie l'existence du fichier de
  niveau référencé et la correspondance d'empreinte, avant que le rejeu ne soit exploitable par
  quiconque (jeu, CLI, script CI) — erreur **récupérable**, jamais une exception ni un plantage.
- **Stabilisation du format v1** : ajout de champs identifiés par l'usage réel des générations 2/3
  (durée totale, empreinte de l'algorithme d'origine) dans les métadonnées déjà prévues par
  `LOT-ANNEXE-07`, sans rupture de compatibilité avec les fichiers déjà écrits (le numéro de
  version, déjà prévu dans le format v1, distingue les deux générations de fichiers).

### Exclus (hors périmètre de ce lot)
- **Réparation ou migration automatique d'un rejeu invalide** (ex. recalculer l'empreinte,
  retrouver un niveau renommé) : un rejeu invalide est signalé, pas réparé — le seul recours est
  de le ré-exporter (`LOT-ANNEXE-19`, `export-replay`) depuis un modèle déjà entraîné.
- **Diagnostic détaillé de ce qui a changé dans le niveau** (diff sémantique tuile par tuile) :
  l'empreinte dit *si* le fichier a changé, jamais *quoi* — un diagnostic plus fin n'a pas de
  consommateur identifié (le seul geste possible en cas d'échec est de ré-exporter).
- **Refonte du format en v2 incompatible** : ce lot reste additif à l'intérieur du format v1 (champs
  optionnels, version déjà prévue) — une rupture de format n'a aucun besoin identifié ici.

## Décisions de cadrage
- **Empreinte par hachage FNV-1a 64 bits sur le contenu brut (octets UTF-8) du fichier de niveau**,
  calculée une fois à l'export et une fois à la lecture, jamais sur le modèle `core::Level` déjà
  chargé (le fichier peut différer textuellement — reformatage — sans que le modèle parsé change,
  mais le principe retenu ici est la simplicité et la reproductibilité : le même fichier produit
  toujours la même empreinte, immédiatement, sans dépendre du chargeur). Choix délibérément **non
  cryptographique** : rien à sécuriser, seulement détecter un changement accidentel. Ce choix d'un
  algorithme simple et non ambigu limite aussi le risque de divergence avec sa réimplémentation en
  Python pur par le garde-fou CI (`LOT-ANNEXE-20`), qui ne peut pas exécuter de code C++.
- **La validation ne s'exécute qu'au chargement du rejeu, jamais à chaque pas** : coût unique,
  négligeable devant la boucle de simulation qu'elle protège en amont.
- **Un rejeu invalide est une erreur récupérable** (`Documentation/Specification/conventions.md`,
  catégorie « erreur récupérable attendue »), au même titre qu'un niveau introuvable pour
  `core::LevelLoader` : `aisolver::validateReplay` renvoie un résultat portant une erreur
  optionnelle, jamais une exception — l'appelant (jeu, CLI, script) décide de la suite (message,
  code de sortie non nul).
- **Compatibilité ascendante stricte sur le format v1** : les fichiers déjà écrits avant ce lot
  (`version == 1`, sans les nouveaux champs) restent lisibles ; les nouveaux champs sont absents/à
  valeur sentinelle sur ces fichiers plutôt que de faire échouer leur lecture. Seuls les nouveaux
  exports (`version` incrémenté) les renseignent.

## Notions abordées
Aucune notion d'apprentissage automatique nouvelle : ce lot est de l'ingénierie logicielle pure
(hachage, validation de fichier) — voir @ref guide-annexe-apprentissage-renforcement pour le
rappel du déterminisme (`EX-NFR-002`) qui rend le rejeu fidèle possible en premier lieu.

## Exigences couvertes
- Nouvelle, déclarée dans [la spécification IA](@ref spec-ia) : [`EX-IA-018`](@ref EX-IA-018).
- Réutilisées (inchangées) : format de rejeu v1 et ses métadonnées (`LOT-ANNEXE-07`), politique
  d'erreur récupérable (`Documentation/Specification/conventions.md`), `EX-NFR-002` (déterminisme :
  la validation garantit justement que les conditions du déterminisme — même niveau — sont
  réunies).

## Découpage

> État : ✅ fait · 🔄 en cours · ⬜ non commencé.

| Tâche | Intitulé | Emplacement | État |
|-------|----------|-------------|:----:|
| [TACHE-01](tache-01-validation-lecture-empreinte-niveau.md) | Validation à la lecture (empreinte de niveau) | `Source/AiSolver/Replay` | ✅ |
| [TACHE-02](tache-02-stabilisation-format-v1.md) | Stabilisation du format v1 (nouveaux champs, compatibilité ascendante) | `Source/AiSolver/Replay` | ✅ |

## Critères d'acceptation du lot
1. Charger un rejeu dont le fichier de niveau référencé a été modifié après l'export échoue
   proprement (erreur récupérable, message explicite), sans exception ni plantage.
2. Charger un rejeu dont le fichier de niveau référencé n'existe plus échoue de la même façon.
3. Charger un rejeu valide, sur un niveau inchangé depuis l'export, réussit sans faux positif.
4. Un fichier de rejeu écrit par le format v1 d'origine (avant ce lot, sans les nouveaux champs)
   reste lisible sans erreur — seuls les nouveaux champs sont absents/à valeur sentinelle.
5. Logique nouvelle **couverte par des tests** (`ctest` vert), déterministe, sans GPU. Build
   `/W4 /WX` sans avertissement, Doxygen et `scripts/lint_exigences.py` verts.

## Dépendances
Étend le format de rejeu v1 de [LOT-ANNEXE-07](@ref lot-annexe-07). La stabilisation (TACHE-02)
s'appuie sur l'usage réel des générations 2 et 3 ([LOT-ANNEXE-10](@ref lot-annexe-10) à
[LOT-ANNEXE-16](@ref lot-annexe-16), qui produisent et évaluent les fichiers de rejeu réels).
Préalable obligatoire à [LOT-ANNEXE-18](@ref lot-annexe-18) (intégration jeu) et
[LOT-ANNEXE-20](@ref lot-annexe-20) (garde-fou CI), qui réutilisent tous deux cette validation.

## Navigation des tâches
- @subpage lot-annexe-17-tache-01-validation-lecture-empreinte-niveau
- @subpage lot-annexe-17-tache-02-stabilisation-format-v1
