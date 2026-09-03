# TACHE-06 — Documentation et vérification {#lot-58-tache-06-documentation-verification}

**Lot :** [LOT-58](epic.md) · **Emplacement :** `Documentation` · **Statut :** en cours

## Contexte
Ce lot ne change rien au produit : il change ce que le dépôt **garantit**. Sa documentation est donc
son livrable principal — et l'occasion de corriger des affirmations aujourd'hui fausses.

`conventions.md` et `exigences-non-fonctionnelles.md` décrivent clang-tidy, ASan et la couverture
comme de l'« outillage déjà en place ». C'était vrai au sens où les fichiers de configuration
existaient, et faux au sens où rien ne les exécutait. Après ce lot, la phrase devient exacte — à
condition de dire **où** chaque vérification s'exécute.

## Travail à réaliser
- **`Documentation/Specification/conventions.md`** : le tableau d'outillage indique, pour chaque
  outil, **où il s'exécute** (job CI nommé) et non plus seulement qu'il est configuré.
- **`Documentation/Specification/exigences-non-fonctionnelles.md`** : déclarer `EX-NFR-023` et
  `EX-NFR-024` en section 3 (Qualité & vérification) ; compléter `EX-NFR-003` et `EX-NFR-022` par
  leur moyen de vérification ; corriger la phrase de traçabilité finale.
- **`CONTRIBUTING.md`** : la liste « avant d'ouvrir une PR » reflète les contrôles réels, avec les
  commandes locales équivalentes (format, Release), pour qu'un échec de CI soit prévisible.
- **`README.md`** : la section CI énumère les jobs — elle sert de vue d'ensemble et sera fausse dès
  ce lot sinon.
- **`CHANGELOG.md`**, section *Non publié*.
- Consigner la **valeur de référence de couverture**, la **durée** de la CI avant et après, et
  l'**état du triage clang-tidy** (familles bloquantes, familles en avertissement et pourquoi).

## Fichiers impactés
- `Documentation/Specification/{conventions,exigences-non-fonctionnelles}.md`.
- `CONTRIBUTING.md`, `README.md`, `CHANGELOG.md`.

## Tests (obligatoires)
- `python scripts/lint_exigences.py` — `EX-NFR-023` et `EX-NFR-024` déclarées une fois et
  référencées ; `EX-NFR-003` n'est plus orpheline.
- `python scripts/generate_cahier_test.py --check` et `python scripts/check_demo_sequence.py`.
- `python scripts/build_docs.py` avec la version Doxygen épinglée par `ci.yml`.
- Les **cinq** jobs nouveaux ou modifiés (`build-test-release`, `sanitize`, `clang-tidy`, `format`,
  `build-test-coverage`) passent sur la PR du lot — vérifié par `gh pr checks`, pas supposé.

## Points d'attention
- **Ne pas annoncer plus que ce qui est livré.** Si le triage clang-tidy laisse des familles en
  avertissement, la documentation le dit, avec la liste. Une doc qui affirme une garantie non tenue
  est pire que l'absence de doc, puisqu'elle empêche de retrouver le trou.
- La liste des jobs figure à plusieurs endroits (`README.md`, `CONTRIBUTING.md`, commentaires de
  `ci.yml`) : les faire concorder, ou réduire à un seul endroit faisant autorité.
- Éviter `` `fichier.cpp::Nom` `` dans la documentation Doxygen : le `::` dans un span casse la
  génération sur la version épinglée de la CI sans rien signaler en local.

## État de la vérification (LOT-58) — mode local, sans PR ni push
Les six tâches ont été implémentées et vérifiées **localement**, sans ouvrir de PR (choix explicite
pour ce lot) :
- `lint_exigences.py`, `generate_cahier_test.py --check`, `check_demo_sequence.py`,
  `build_docs.py` (Doxygen `1.16.1` téléchargé pour matcher la CI) : **tous verts**.
- Presets Release : `ctest --preset ninja-release` → 943/943 tests (2 ignorés sous `NDEBUG`), test
  négatif démontré (variable POD lue seulement par `JADG_ASSERT`).
- ASan : 846+89+3 tests verts sur 3 exécutions consécutives, un vrai `heap-use-after-free` trouvé et
  corrigé, test négatif démontré (lecture heap hors bornes).
- clang-tidy : triage complet (129 fichiers), `bugprone-*` à zéro, test négatif démontré
  (`bugprone-integer-division`).
- clang-format : reformatage initial (192 fichiers) + `943/943` tests verts après, test négatif
  démontré.
- Couverture : `OpenCppCoverage` installé (élévation UAC accordée manuellement), mesure fusionnée
  prise le 2026-08-10 : **93.66 %** (8602/9184 lignes), seuil posé à 85 % (marge ~8.5 points), test
  négatif démontré par calcul direct (seuil à 95 % aurait fait échouer le job). Voir
  `tache-05-couverture.md`, section « État de la mesure », pour la réserve méthodologique (mesure
  prise sur un build **Ninja** local, pas le preset `vs` exact de la CI).

**Ce qui reste, faute de PR/CI réelle** : la démonstration `gh pr checks` des cinq jobs et la durée
réelle de la CI (mesurable seulement sur de vraies machines GitHub Actions), ainsi que la
confirmation que la couverture mesurée en CI (preset `vs`) reste cohérente avec la mesure Ninja
locale. À lever au premier passage réel en CI de la branche
`ci/lot-58-verification-release-analyse`, avant de considérer le lot totalement clos.

## Définition de fait (DoD)
- Les documents décrivent les vérifications **réellement exécutées**, les deux nouvelles exigences
  sont déclarées et référencées, `EX-NFR-003` est rattachée, les mesures de référence (couverture,
  durée, triage) sont consignées, et la CI complète est verte sur la PR du lot.
- **Presque atteinte** : voir « État de la vérification » ci-dessus. Documentation, câblage et
  vérifications locales faits, y compris la mesure de couverture. Seule la CI réelle (première
  exécution en ligne, `gh pr checks`, durée mesurée) reste à faire.

## Exigences
`EX-NFR-023`, `EX-NFR-024` (déclarées ici) ; réutilise `EX-NFR-003` (ASan), `EX-NFR-012`
(conventions), `EX-NFR-013` (sans avertissement), `EX-NFR-022` (CI), `EX-NFR-031` (outils épinglés).
