# TACHE-10 — Documentation et vérification finale {#lot-65-tache-10-documentation-verification-finale}

**Lot :** [LOT-65](epic.md) · **Emplacement :** `Documentation`, `Source/Elements/Levels` · **Statut :** non commencé

## Contexte
La `TACHE-04` avait documenté la séquence des vingt-deux tableaux ; les `TACHE-05` à `TACHE-09` en
ont refait le contenu et changé la composition. Cette tâche remet la documentation en accord avec ce
qui est livré, et exécute la vérification complète — dont le parcours **manuel**, seul juge du volet
tutoriel, qu'aucun test automatique ne peut rendre.

## Travail à réaliser
- **`Source/Elements/Levels/README.md`** : tableau des vingt-deux tableaux dans le nouvel ordre, avec
  la mécanique démontrée, le cadrage **et** les budgets — l'information qui manquait et qui est
  devenue structurante. Mentionner la disparition de `demo-arrondi.json` (fusionné) et de
  `demo-salles.json` (absorbé par le final).
- **`Documentation/Specification/niveaux.md`** : la doctrine écrite en `TACHE-05` y est déjà ;
  vérifier qu'elle décrit bien le contenu livré et non une intention.
- **`Documentation/Specification/gameplay.md`** : exigences actualisées par la `TACHE-06` (porte
  écrasante, poids d'une plaque).
- **`Documentation/Guide/guide-niveaux.md`** : décrire les garde-fous de profondeur au même endroit
  que le garde-fou de couverture, pour qu'un lecteur comprenne ce que chacun garantit — et surtout
  ce qu'il ne garantit pas.
- **`epic.md`** : découpage à jour, statut, et une section rendant compte de l'écart entre les
  critères d'acceptation et ce que les `TACHE-02`/`TACHE-03` avaient réellement livré.
- **`CHANGELOG.md`**, section *Non publié* : refonte, corrections moteur, garde-fous, et registre des
  défauts découverts en jouant les croisements.

## Vérification
1. Compilation `/W4 /WX` propre, `ctest` à 100 %.
2. `python scripts/check_demo_sequence.py` — séquence, test système et `MainWindow` alignés.
3. `python scripts/lint_exigences.py` — en filtrant `.claude`, dont les worktrees produisent des
   centaines de faux `DOUBLON`.
4. `python scripts/generate_cahier_test.py --check` — le contrôle le plus facile à oublier ; tout
   test ajouté par les `TACHE-05`/`TACHE-06`/`TACHE-07` doit porter son bloc `\castest{}`.
5. Documentation Doxygen sans avertissement.
6. **Parcours manuel complet** en Release, manette et son : le seul moyen de juger si la séquence
   enseigne réellement, et le moment où ce lot trouve historiquement le plus de défauts.

## Points d'attention
- **La documentation suit le code livré, jamais l'intention.** Ne décrire un tableau qu'une fois
  qu'il est franchi par le test système.
- **Ne pas écrire l'historique du débogage dans le code.** Les constats de cette refonte ont leur
  place ici et au `CHANGELOG`, pas en commentaire de source.
- Attention aux pièges Doxygen déjà rencontrés dans ce dépôt : jamais de `` `fichier.cpp::Nom` ``
  dans un span, et un `**gras**/**gras**` referme un bloc `/** */`.
- Le numéro de version ne s'écrit qu'à un seul endroit, `project(VERSION)`.

## Définition de fait (DoD)
- Documentation, spécification, guide, epic et changelog décrivent le contenu réellement livré ;
  les six vérifications automatiques sont vertes ; le parcours manuel est fait et ses constats
  consignés.

## Exigences
`EX-LVL-012`, `EX-LVL-015` ; réutilise `EX-NFR-020` (tests), `EX-NFR-021` (test système),
`EX-NFR-030` (documentation).
