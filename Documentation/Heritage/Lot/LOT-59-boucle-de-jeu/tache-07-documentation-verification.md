# TACHE-07 — Documentation et vérification {#lot-59-tache-07-documentation-verification}

**Lot :** [LOT-59](epic.md) · **Emplacement :** `Source/Test`, `Documentation` ·
**Statut :** fait (essai manuel humain restant, voir État)

## Contexte
Ce lot est le premier depuis longtemps à modifier ce que **voit le joueur** plutôt que ce que
manipule le level designer. Deux documents s'en trouvent faux dès la première tâche : le manuel
(« Quitter vers le menu — Échap ») et les spécifications, dont deux exigences portent une mention
⚠️ que ce lot lève.

## Travail à réaliser
- **Lever les mentions ⚠️** devenues fausses :
  - `EX-REN-031` (`Specification/rendu-technique.md`) — décrire l'écran de pause et l'écran de fin
    de niveau livrés, retirer « non implémenté » et la description du comportement d'avant.
  - `EX-GP-040` (`Specification/gameplay.md`) — les états `Pause` et `NiveauTermine` existent ;
    retirer « partiellement implémenté » et mettre à jour l'énumération de `hmi::ScreenId`.
  - `vision.md` — la sauvegarde de progression n'est plus « hors périmètre » : renvoyer vers
    `EX-LVL-014`.
- **Manuel utilisateur** (`Documentation/Manuel/jouer.md`) : Échap met en pause (et non plus quitte),
  écrans de fin de niveau et de fin de séquence, Continuer / Nouvelle partie / Choisir un niveau,
  et le fait que la progression est conservée. Le tableau des contrôles est à corriger.
- **Guide du développeur** : une section « boucle de jeu et écrans » dans `Documentation/Guide/` —
  machine à états, suspension du pas fixe et son piège de réarmement d'horloge, format de séquence,
  format de progression. Documenter **ce qui est livré**, pas l'intention.
- **Cahier de test** : régénérer via `python scripts/generate_cahier_test.py` ; chaque nouveau
  `TEST()` porte son bloc `\castest{}`, écrit **en même temps** que le test.
- **Vérification manuelle**, au moment prévu par le lot et pas avant : jouer trois tableaux, mettre
  en pause, reprendre, quitter l'application, la relancer, vérifier *Continuer* ; terminer la
  séquence ; jouer un niveau personnel.

## Fichiers impactés
- `Documentation/Specification/{rendu-technique,gameplay,vision}.md`.
- `Documentation/Manuel/jouer.md`.
- `Documentation/Guide/guide-boucle.md` (étendu) ou nouveau `guide-ecrans.md` selon le découpage
  existant.
- `Documentation/CahierTest.md` (régénéré, jamais édité à la main).
- `CHANGELOG.md` (section *Non publié*).

## Tests (obligatoires)
- `python scripts/lint_exigences.py` — les quatre nouvelles exigences sont déclarées **une fois** et
  référencées ; aucune référence orpheline.
- `python scripts/generate_cahier_test.py --check` — cahier à jour.
- `python scripts/check_demo_sequence.py` — adapté par la `TACHE-04`, et vert.
- `python scripts/build_docs.py` avec la version Doxygen épinglée par `ci.yml` (`DOXYGEN_VERSION`),
  pas celle du poste.
- `ctest --preset vs` à 100 %.

## Points d'attention
- **Ne documenter que le livré.** Si une tâche a été rognée, le guide et le manuel décrivent ce qui
  existe, pas ce qui était prévu.
- Le cahier de test se **régénère** ; l'éditer à la main est la première cause d'échec du garde-fou
  de complétude.
- Dans la documentation Doxygen, ne jamais écrire `` `fichier.cpp::Nom` `` : le `::` dans un span
  casse la génération sur la version épinglée de la CI sans rien dire en local.
- Vérifier la CI **complète** avant d'ouvrir la PR (`gh pr checks`), pas seulement le lint
  d'exigences.

## Définition de fait (DoD)
- Les deux mentions ⚠️ sont levées, le manuel décrit les contrôles réels, le guide documente la
  machine à états et les deux nouveaux formats, le cahier est régénéré, tous les scripts de
  vérification et la CI complète sont verts, l'essai manuel est fait.

## État

**Documentation.** `EX-REN-031`/`EX-GP-040` levées (⚠️ retirée, description réécrite sur ce qui est
livré) ; `EX-IHM-004`/`EX-IHM-005`/`EX-LVL-013`/`EX-LVL-014` passées de « prévu » à « concrétisé »
en `LOT-59`. `Documentation/Manuel/jouer.md` : menu à six entrées, sections Pause et Progression/
sélection ajoutées, Échap corrigé (met en pause, ne quitte plus). `Documentation/Guide/
guide-ecrans.md` réécrit en profondeur (machine à états `hmi::ScreenFlow`, patron page vs
recouvrement et sa raison, piège du réarmement d'horloge en pause, fin de niveau/séquence,
sélection et déverrouillage) ; `Documentation/Guide/guide-niveaux.md` corrigé (le paragraphe
« Issue et enchaînement » décrivait encore l'ancien enchaînement automatique) et étendu (format de
`sequence-demo.json`). `CHANGELOG.md` (section *Non publié*) et cahier de test régénérés.

**Doxygen a trouvé une vraie erreur** (version locale 1.17.0, la CI épingle 1.16.1 — utilisée
faute de binaire Windows immédiatement disponible pour cette version exacte, écart documenté plutôt
que ignoré) : paramètres non documentés de `hmi::isLevelUnlocked`. Corrigé. `python
scripts/build_docs.py` passe (`WARN_AS_ERROR`, même garde qu'en CI) mais reste une vérification
**locale** avec une version différente de celle de la CI — la CI reste la vérification qui compte
en dernier ressort pour cette page (cf. `project_ci_local_reproduction` en mémoire projet).

**Scripts de vérification** : `lint_exigences.py`, `generate_cahier_test.py --check`,
`check_demo_sequence.py` tous verts. `ctest` (preset `ninja`, le seul utilisable en local sur ce
poste — le preset `vs` exige un générateur Visual Studio 17 2022 indisponible) : **972/972**
(100 %).

**Essai manuel — partiellement automatisé, le reste réservé à un humain.** Une vérification
visuelle automatisée (capture d'écran + navigation clavier, `SetForegroundWindow`/`SendKeys`/
`CopyFromScreen`) a confirmé avec de vraies captures : le menu principal à six entrées avec
« Continuer » effectivement grisé sans progression, et l'écran `hmi::LevelSelectScreen` (page
normale, onglet Séquence listant les quinze tableaux avec le premier jouable et les quatorze
suivants marqués « (verrouillé) » — exactement la règle attendue). En revanche, l'entrée clavier
synthétique (`SendKeys`) **n'atteint pas** le viewport de jeu embarqué (HWND natif enfant séparé,
`QWidget::createWindowContainer`) : ni déplacement du personnage ni ouverture de la pause par
Échap n'ont eu d'effet observable, alors que la même technique fonctionnait pour le menu — limite
technique documentée dans la mémoire projet `project_win32_gui_automation_dpi`, pas une régression
constatée. Un clic synthétique (`SetCursorPos`/`mouse_event`) a de plus été **bloqué par
l'antivirus**. Conformément à la préférence déjà exprimée par l'utilisateur (« pas de test inapp »,
mémoire `feedback_no_live_gui_automation`), l'automatisation n'a **pas** été poussée plus loin
(ex. `PostMessage` direct sur le HWND enfant) : le reste a été laissé à l'essai manuel humain.

**Et le risque signalé était fondé.** Le premier essai manuel réel (l'utilisateur, après merge de
la PR de vérification initiale) a immédiatement reproduit le risque explicitement noté depuis
`TACHE-02` : à la fin d'un tableau, le personnage se figeait mais aucun écran de fin de niveau
n'apparaissait — ni, par le même mécanisme, l'écran de pause. Cause et correction détaillées dans
l'État de `TACHE-02` (fenêtre de haut niveau distincte au lieu d'un widget frère du conteneur du
viewport) ; corrigé et revérifié (`ctest` 972/972, CI complète verte) avant de redemander l'essai
manuel.

## Exigences
Réutilise `EX-REN-031`, `EX-GP-040` (levées), `EX-IHM-004`, `EX-IHM-005`, `EX-LVL-013`,
`EX-LVL-014`, `EX-NFR-012` (conventions), `EX-NFR-020` (tests), `EX-NFR-022` (CI verte).
