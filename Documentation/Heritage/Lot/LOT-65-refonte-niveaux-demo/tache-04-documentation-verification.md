# TACHE-04 — Documentation et vérification {#lot-65-tache-04-documentation-verification}

**Lot :** [LOT-65](epic.md) · **Emplacement :** `Source/Test`, `Documentation` ·
**Statut :** 🔄 en cours — tout fait sauf le parcours manuel

## Réalisé
- `EX-LVL-015` déclarée (`niveaux.md`, `TACHE-01`) ; `EX-LVL-012` actualisée (ne promet plus « 3
  niveaux »).
- `Source/Elements/Levels/README.md` : inventaire des vingt-deux tableaux, mécanique démontrée par
  chacun, cadrage retenu.
- `Documentation/Guide/guide-niveaux.md` : section garde-fou de couverture (ce qu'il vérifie
  exactement, comment justifier une exclusion) et avertissement sur le défaut plateforme
  mobile/pente.
- `Documentation/Manuel/jouer.md` : déjà complet pour les mécanismes du `LOT-63` (clé, porte
  verrouillée, plateforme mobile) — vérifié, aucune modification nécessaire.
- Registre des défauts (`CHANGELOG.md`, section *Non publié*) : le défaut plateforme mobile/pente,
  isolé par bissection, consigné avec sa reproduction.
- Cahier de test régénéré (1122 cas) ; `python scripts/generate_cahier_test.py --check` vert.
- CI locale complète vérifiée verte : `lint_exigences.py`, `check_demo_sequence.py`,
  `build_docs.py` (Doxygen), `ctest --preset ninja` à 100 % (1122/1122).

## Reste à faire
- **Parcours manuel complet** (binaire Release, manette branchée, son activé), du menu à l'écran de
  fin — hors de portée de cette session (pas d'automatisation d'interface en direct pour vérifier,
  convention du dépôt). C'est le moment où ce lot attend le plus souvent de trouver des défauts,
  d'après l'expérience des lots précédents ; à faire par un humain avant de considérer le lot
  réellement clos.

## Contexte
Ce lot est le dernier avant la clôture du programme, et le premier depuis longtemps à faire jouer
**tout** le moteur ensemble. Sa vérification n'est donc pas une formalité : c'est la répétition
générale de la `0.1.0`.

Deux livrables documentaires en découlent : le **registre des défauts découverts**, qui alimentera
la suite, et la mise à jour des documents décrivant le contenu — dont `EX-LVL-012`, qui promet
encore « **3 niveaux** » de démonstration.

## Travail à réaliser
- **Registre des défauts** découverts en jouant : symptôme, tableau et reproduction. Consigné dans
  le `CHANGELOG.md` (section *Non publié*) ou en tickets, jamais perdu dans une conversation. C'est
  le sous-produit le plus utile du lot.
- **Spécifications** : déclarer `EX-LVL-015` dans `niveaux.md` ; actualiser `EX-LVL-012`, dont le
  « 3 niveaux » date du MVP et ne décrit plus rien.
- **`Source/Elements/Levels/README.md`** : inventaire des tableaux, mécanique démontrée par chacun,
  cadrage retenu — la table de référence du level designer.
- **Guide du développeur** (`guide-niveaux.md`) : le garde-fou de couverture, ce qu'il vérifie
  exactement (présence, pas franchissabilité) et comment justifier une exclusion.
- **Manuel utilisateur** (`Documentation/Manuel/jouer.md`) : la section « Objectif d'un niveau »
  décrit les mécanismes rencontrés — la compléter de ceux du [LOT-63](@ref lot-63).
- **Cahier de test** régénéré ; chaque nouveau `TEST()` porte son bloc `\castest{}` écrit en même
  temps que lui.
- **Vérification manuelle complète** : jouer la séquence entière, du menu à l'écran de fin, avec le
  son, les effets, la pause et la progression — c'est le parcours que fera un joueur de la `0.1.0`.

## Fichiers impactés
- `Documentation/Specification/niveaux.md`.
- `Source/Elements/Levels/README.md`.
- `Documentation/Guide/guide-niveaux.md`, `Documentation/Manuel/jouer.md`, `CHANGELOG.md`.
- `Documentation/CahierTest.md` (régénéré).

## Tests (obligatoires)
- `python scripts/lint_exigences.py` — `EX-LVL-015` déclarée une fois et référencée.
- `python scripts/generate_cahier_test.py --check` et `python scripts/check_demo_sequence.py`.
- `python scripts/build_docs.py` avec la version Doxygen épinglée par `ci.yml`.
- `ctest --preset vs` à 100 %, **et** les jobs Release, ASan, analyse statique et format livrés par
  [LOT-58](@ref lot-58).
- Le garde-fou de couverture et le test système sont **verts et stables** sur trois exécutions
  consécutives — un test de contenu intermittent est un défaut, pas un aléa.
- **Parcours manuel complet** de la séquence sur le binaire, du menu à la fin.

## Points d'attention
- **Ne pas taire les défauts découverts.** Le registre est le livrable qui justifie une partie du
  coût du lot ; le vider pour « finir propre » reviendrait à jeter ce qu'on est venu chercher.
- **Ne documenter que le livré** : si des tableaux de synthèse ont été retirés faute de moteur
  stable, le `README.md` et le manuel décrivent la séquence réelle.
- Éviter `` `fichier.cpp::Nom` `` dans la documentation Doxygen : le `::` dans un span casse la
  génération sur la version épinglée de la CI sans rien dire en local.
- Le lint d'exigences produit des centaines de faux positifs si un worktree traîne sous `.claude/` :
  filtrer avant de conclure.
- Le parcours manuel est long : le faire **une fois** dans les conditions réelles (binaire Release,
  manette branchée, son activé) vaut mieux que dix essais partiels dans l'éditeur.

## Définition de fait (DoD)
- `EX-LVL-015` est déclarée et `EX-LVL-012` actualisée ; le `README.md` des niveaux inventorie
  tableaux, mécaniques et cadrages ; le guide décrit le garde-fou et ses limites ; le manuel couvre
  tous les mécanismes ; le registre des défauts est écrit ; le cahier est régénéré ; la CI complète
  est verte et stable ; le parcours manuel complet est fait.

## Exigences
`EX-LVL-015` (déclarée ici) ; réutilise `EX-LVL-012` (actualisée), `EX-NFR-021` (test système),
`EX-NFR-012` (conventions), `EX-NFR-020` (tests), `EX-NFR-022` (CI verte).
