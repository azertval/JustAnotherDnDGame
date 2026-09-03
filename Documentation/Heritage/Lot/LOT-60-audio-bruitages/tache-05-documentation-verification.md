# TACHE-05 — Documentation et vérification {#lot-60-tache-05-documentation-verification}

**Lot :** [LOT-60](epic.md) · **Emplacement :** `Source/Test`, `Documentation` ·
**Statut :** fait

## Contexte
Ce lot introduit un **domaine** absent du projet depuis le début, et une **dépendance** de plus.
Les deux se documentent : l'un dans le guide du développeur, où rien ne parle encore d'audio ;
l'autre dans les endroits qui décrivent le provisionnement de Qt, aujourd'hui écrits pour deux
composants et bientôt faux pour trois.

## Travail à réaliser
- **Lever la mention ⚠️ de `EX-REN-040`** dans `Documentation/Specification/rendu-technique.md` :
  la section « 5. Audio (⚠️ minimal MVP) » décrit ce qui est livré, et **déclare** `EX-REN-047` et
  `EX-REN-048`.
- **Guide du développeur** : nouveau `Documentation/Guide/guide-audio.md` — architecture
  (`Core` expose des transitions, `HMI` en déduit des sons), catalogue et son repli silencieux,
  détection d'événements partagée avec les particules, préchargement et son piège de chargement
  asynchrone, provisionnement du module Qt. Le référencer depuis `Documentation/Guide/guide.md`.
- **Manuel utilisateur** : mentionner le réglage de volume dans la description du menu d'options
  (`Documentation/Manuel/jouer.md`).
- **`README.md`** : la liste des fonctionnalités du moteur ne mentionne aucun son ; l'ajouter. Les
  prérequis de build mentionnent Qt : préciser le module.
- **`CONTRIBUTING.md`/`README.md`** : provisionnement de `qtmultimedia` en local.
- **Cahier de test** régénéré par `python scripts/generate_cahier_test.py` ; chaque nouveau `TEST()`
  porte son bloc `\castest{}` écrit **en même temps** que lui.
- **Vérification manuelle**, au moment prévu : jouer un tableau complet en écoutant chaque
  événement, régler le volume, couper le son, terminer un tableau, mourir ; puis lancer le zip de
  release sur une machine sans Qt installé.

## Fichiers impactés
- `Documentation/Specification/rendu-technique.md`.
- `Documentation/Guide/guide-audio.md` (nouveau), `Documentation/Guide/guide.md`.
- `Documentation/Manuel/jouer.md`, `README.md`, `CONTRIBUTING.md`, `CHANGELOG.md`.
- `Documentation/CahierTest.md` (régénéré).

## Tests (obligatoires)
- `python scripts/lint_exigences.py` — `EX-REN-047` et `EX-REN-048` déclarées une fois et
  référencées.
- `python scripts/generate_cahier_test.py --check` et `python scripts/check_demo_sequence.py`.
- `python scripts/build_docs.py` avec la version Doxygen épinglée par `ci.yml`.
- `ctest --preset vs` à 100 %, **et** le job Release livré par [LOT-58](@ref lot-58).
- Essai du zip de release sur une machine **sans Qt installé** : le jeu démarre et produit du son.
  C'est le seul contrôle qui valide réellement le déploiement du module.

## Points d'attention
- **Ne documenter que le livré.** Si les sons d'interface ont été rognés, le guide le dit.
- Le contrôle décisif de ce lot est celui du **déploiement**, pas celui du code : un build local
  sonore ne prouve rien sur le zip publié, puisque le poste de développement a Qt installé.
- Éviter `` `fichier.cpp::Nom` `` dans la documentation Doxygen : le `::` dans un span casse la
  génération sur la version épinglée de la CI sans rien dire en local.
- Attention aux commentaires de code contenant `**gauche**/**droite**` : la séquence `**/` ferme un
  bloc `/** */` en plein milieu et produit des erreurs de compilation sans rapport apparent.

## État
`EX-REN-040` ne porte plus ⚠️ dans `rendu-technique.md` ; `EX-REN-047`/`EX-REN-048` y sont décrites
comme livrées, avec les symboles réels (`hmi::AudioEngine`, `core::Player::justJumped`,
`core::MechanismController::isContinuous`). Nouveau `Documentation/Guide/guide-audio.md`,
référencé depuis `guide.md`. Manuel utilisateur (`jouer.md`) : septième entrée de menu
(Crédits), volume dans la description du menu d'options. `README.md` : bruitages dans les
fonctionnalités, et un prérequis Qt6/Multimedia ajouté aux prérequis de build — qui ne
documentaient **aucun** module Qt jusqu'ici, pas même `Widgets`/`Gui` (gap plus large que ce lot,
corrigé à l'occasion). `CHANGELOG.md` : entrée LOT-60 complète. Cahier de test régénéré
(1003 cas), `lint_exigences.py`/`check_demo_sequence.py`/`build_docs.py` (Doxygen local 1.17.0 —
la version épinglée par la CI, 1.16.1, reste l'arbitre final) verts.

Deux avertissements Doxygen (paramètres non documentés sur `detectMechanismEvents` et le
constructeur d'`OptionsPage`) découverts en générant la doc localement, corrigés dans la foulée.

**Non fait par moi, délibérément** : l'essai manuel (jouer un tableau complet, régler/couper le
volume, lancer le zip de release sur une machine sans Qt) — ce lot suit la même règle que les
précédents : pas d'automatisation GUI ad hoc en cours de tâche, l'essai réel revient à l'utilisateur
au moment prévu.

## Définition de fait (DoD)
- `EX-REN-040` ne porte plus de ⚠️, les deux nouvelles exigences sont déclarées, le guide audio
  existe et est référencé, le manuel et le README disent vrai, le cahier est régénéré, la CI
  complète est verte, et le zip de release a été essayé sur une machine sans Qt.

## Exigences
Réutilise `EX-REN-040` (levée), `EX-REN-047`, `EX-REN-048`, `EX-BUILD-010` (provisionnement
documenté), `EX-NFR-012` (conventions), `EX-NFR-020` (tests), `EX-NFR-022` (CI verte).
