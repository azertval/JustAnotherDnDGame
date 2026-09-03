# TACHE-04 — Nettoyage des tests, documentation (menu/options/contrôles) & vérification {#lot-38-tache-04-nettoyage-tests-doc}

**Lot :** [LOT-38](epic.md) · **Emplacement :** `Source/Test`, `Documentation` · **Statut :** non commencé

## Contexte
Clôt le lot (et l'essentiel du programme) : **stabiliser la suite de tests** après le retrait du
legacy (TACHE-03) et **mettre à jour toute la documentation** décrivant l'UI, qui parle encore des
écrans maison. C'est aussi le moment de vérifier de bout en bout l'application Qt unifiée.

## Travail à réaliser
- **Tests** :
  - Retirer les tests d'UI legacy sans objet ; conserver/rattacher les tests de logique migrée
    (validations, taxonomie, bindings, opérations de fichiers, géométrie de liens, navigation).
  - Vérifier la couverture (OpenCppCoverage en CI) : la logique nouvelle (contrôleurs, navigation,
    opérations) est couverte ; `UnitTests` ne linke pas Qt.
- **Documentation** (réécrire pour Qt) — **guides à mettre à jour** :
  - `Documentation/Guide/guide-ecrans.md` : **réécriture majeure** — ce guide documente la pile
    `IScreen`/`ScreenManager`/`ScreenTransition` **supprimée** ; le remplacer par la navigation Qt
    (menu/vues) ou le retirer de `guide.md` s'il devient sans objet.
  - `Documentation/Guide/guide-editeur.md` : point d'entrée depuis le menu Qt (lancer l'éditeur).
  - `Documentation/Guide/guide-entrees.md` : finaliser le passage du remappage aux écrans Qt (la
    capture de touches/boutons passe par Qt), les modèles `…KeyBindings` restant inchangés.
  - Créer (ou intégrer à `guide-ecrans`) une section « menu / options » pour l'UI Qt ; référencer via
    `Documentation/Guide/guide.md` (`@subpage`).
  - `Documentation/Specification/controles.md` et toute référence aux écrans maison (`MenuScreen`,
    `OptionsScreen`, `…KeybindingsScreen`).
  - `Documentation/Specification/architecture.md` : entériner « UI Qt unique + rendu D3D11 embarqué,
    `Core` intact » ; retirer les mentions d'`IScreen`/`ScreenManager`.
  - Déclarer/mettre à jour `EX-IHM-040`/`EX-IHM-041`.
  - Regénérer le cahier de test (`generate_cahier_test.py --check` vert).
- **Vérification de bout en bout** : parcours complet menu → jeu → éditeur (peinture, niveaux, liens,
  options, remappage) → quitter.

## Fichiers impactés
- `Source/Test/**` (retraits/adaptations).
- `Documentation/Guide/*`, `Documentation/Specification/architecture.md`,
  `Documentation/Specification/controles.md`, cahier de test généré.

## Tests (obligatoires)
- **Suite complète verte** (`ctest --preset vs`), déterministe ; parcours système inchangé.
- **`--check` du cahier de test** et **lint des exigences** verts.
- **Couverture** générée sans régression notable.
- **Vérification manuelle** de bout en bout (ci-dessus).

## Points d'attention
- **Aucune référence documentaire orpheline** vers les écrans supprimés (le lint des exigences et
  Doxygen `WARN_AS_ERROR` aident à les détecter).
- **Doxygen 1.9.8** : générer localement avant push (piège connu).
- **Cohérence guides** : les captures/descriptions doivent refléter l'UI Qt réelle.

## Définition de fait (DoD)
- Suite de tests stabilisée et verte ; documentation UI entièrement mise à jour pour Qt ; cahier de
  test et lint verts ; Doxygen vert ; vérification de bout en bout OK ; critères du [LOT-38](epic.md)
  satisfaits.

## Exigences
`EX-IHM-040`/`EX-IHM-041` ; `EX-NFR-002` (déterminisme), `EX-NFR-010`.
