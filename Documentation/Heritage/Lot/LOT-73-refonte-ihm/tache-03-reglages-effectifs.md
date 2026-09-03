# TACHE-03 — Tout réglage exposé atteint le moteur {#lot-73-tache-03-reglages-effectifs}

**Lot :** [LOT-73](epic.md) · **Emplacement :** `Source/HMI/Ai`, `Source/HMI/Interface` ·
**Statut :** fait

## Contexte
`AiModeScreen::configFromForm()` lisait consciencieusement neuf réglages de l'onglet Entraînement —
taux d'apprentissage du critique, taux de croisement, épisodes par lot, coefficient d'entropie,
exploration minimale, écrêtage du gradient, images par décision, budget de pas, seuil de blocage.

`onLaunchTraining()` ne les recopiait pas, et `struct TrainingRequest` n'avait **aucun champ** pour
les recevoir. Les régler ne changeait donc rien. Pire : le `config.json` écrit au lancement du run
décrivait une configuration que le run n'avait pas exécutée, de sorte que « Reprendre les réglages
de ce run » rechargeait des valeurs fausses — et le manuel les documentait comme actifs. Rien
n'échouait : ni la compilation, ni un test, ni l'exécution.

Second écart, de même nature : `applyConfigToForm` n'était appelée qu'au chargement d'un fichier et
au clic sur « Réinitialiser aux défauts », **jamais à la construction**. L'écran ouvrait donc sur
les littéraux du `.ui`, dont trois avaient divergé du code — taux d'apprentissage `0,01` au lieu de
`0,003`, gamma `0,99` au lieu de `0,995`, optimiseur `sgd` au lieu de `adam`. « Réinitialiser aux
défauts » **changeait** ainsi trois champs, exactement le symptôme que le commentaire du `.ui`
affirmait avoir éliminé.

Les deux écarts violent le corollaire `IHM ⊇ CLI` posé par le [LOT-ANNEXE-22](@ref lot-annexe-22),
désormais généralisé par [`EX-IHM-083`](@ref EX-IHM-083).

## Travail réalisé
- **`hmi::TrainingRequest` extraite** dans `Source/HMI/Ai/TrainingRequest.h`. `TrainingWorker` est un
  `QObject` à signaux, donc soumis à `moc` et impossible à compiler dans `UnitTests` ; la requête,
  elle, n'est que de la donnée. Cette séparation est ce qui rend la suite testable.

  **Et cette donnée ne porte aucun type Qt**, `QString` compris : le job `sanitize` de la CI
  construit `UnitTests` sous AddressSanitizer **sans installer Qt du tout**, si bien qu'une source
  compilée dans cette cible qui inclurait `<QString>` échoue — même règle que
  `HMI/Interface/DesignTokens.cpp` ou `PixelArtScale.cpp`. La requête est donc en `std::string`, et
  c'est l'écran, qui parle Qt, qui convertit à la frontière. Le défaut a bel et bien été commis
  pendant ce lot, et n'a été vu ni par le build local ni par les tests locaux — tous deux disposant
  de Qt : il se reproduit avec `cmake -DCMAKE_DISABLE_FIND_PACKAGE_Qt6=ON`.
- **Neuf champs ajoutés** à `TrainingRequest`, recopiés par `onLaunchTraining()`. Aucune extension
  du moteur n'a été nécessaire : `aisolver::cli::CommandLineOverrides` les exposait déjà tous.
- **`hmi::overridesFor`** (`Source/HMI/Ai/TrainingOverrides.h/.cpp`) : la traduction requête →
  surcharges, extraite du corps de `TrainingWorker::run()`. Logique **pure**, compilée aussi dans
  `UnitTests`. C'est ici, et nulle part ailleurs, qu'un réglage se perd ; l'extraire rend la
  traduction vérifiable champ par champ, sans lancer d'entraînement.
- **`applyConfigToForm(TrainingConfig{})` appelée à la construction** : l'écran ouvre sur les défauts
  du moteur, par le même appel que le bouton — une seule source de vérité.

## Vérification
`Source/Test/Unit/HMI/Ai/test_training_overrides.cpp`, trois cas :
- **Chaque réglage atteint les surcharges** — une requête dont chaque champ facultatif porte une
  valeur distincte et reconnaissable ; un champ oublié se voit comme un `std::nullopt`, jamais comme
  une coïncidence de valeurs. C'est le test qui aurait signalé les neuf réglages inertes.
- **Les réglages se retrouvent dans la configuration résolue** — la traduction seule ne prouverait
  rien si `loadTrainingConfig` n'en tenait pas compte ; ce cas relie les deux bouts.
- **Une requête vierge ne pose aucune surcharge** — l'écran qui ne touche à rien décrit le même run
  que `aisolver-cli train` sans option.
