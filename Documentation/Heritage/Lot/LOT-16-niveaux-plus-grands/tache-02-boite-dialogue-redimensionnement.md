# TACHE-02 — Boîte de dialogue de redimensionnement (Ctrl+R) {#lot-16-tache-02-boite-dialogue-redimensionnement}

**Lot :** [LOT-16](epic.md) · **Emplacement :** `HMI/Interface` · **Statut :** fait

## Contexte
`EditorScreen` porte déjà un champ de saisie de texte (`_nameInput`, LOT-15 TACHE-03) pour nommer
et renommer un niveau, avec un drapeau `_nameInputIsCreation` distinguant les deux usages. Ajouter
un troisième usage (redimensionner) par un booléen supplémentaire ne passerait pas à l'échelle ;
cette tâche généralise le mécanisme plutôt que d'en construire un second en parallèle.

## Travail à réaliser
- **Généraliser le slot de saisie** : remplacer `bool _nameInputIsCreation` par un `enum class
  TextPromptPurpose { CreateLevelName, RenameLevel, ResizeGrid };` (nom à ajuster librement),
  toujours porté par un seul `std::optional<TextInputField>` — un seul champ actif à la fois, la
  confirmation d'un usage n'affecte jamais les autres. Renommer au passage `_nameInput` en un nom
  neutre (ex. `_textPrompt`) et `renderNameInput` en `renderTextPrompt`, dont le titre affiché
  dépend du `TextPromptPurpose` courant (« Nom du nouveau niveau », « Renommer le niveau »,
  « Nouvelle taille (largeur x hauteur) »).
- **`Ctrl+R` ouvre la boîte de dialogue de redimensionnement**, hors confirmation/essai/sélecteur en
  cours (même emplacement que `F2` dans `update()`) : `TextPromptPurpose::ResizeGrid`, champ
  pré-rempli de la taille courante formatée `largeur x hauteur` (ex. `14x8`), validateur
  `hmi::isValidLevelSize` (TACHE-01).
- **Confirmation** (`TextPromptPurpose::ResizeGrid`) : `parseLevelSize(text)` (déjà garanti
  parsable, le champ n'aurait pas confirmé sinon) puis `requestResize(largeur, hauteur)` — **même**
  chemin que les flèches, donc même confirmation destructrice si la nouvelle taille perdrait
  l'entrée/la sortie/une liaison (`EX-EDIT-012`, aucune duplication).
- **Annulation** (`Échap`) : aucun effet, la grille reste inchangée — cohérent avec le renommage
  (LOT-15), à la différence de la création où annuler revient au sélecteur (aucun changement de ce
  comportement-là).

## Fichiers impactés
- `Source/HMI/Interface/EditorScreen.h`/`.cpp` (généralisation du slot de saisie, raccourci
  `Ctrl+R`, branchement de la confirmation vers `requestResize`).
- `Source/HMI/Input/InputState.h` (nouvel énumérateur `Key::R`, `0x52` — purement additif, même
  principe que les ajouts de LOT-15).

## Tests (obligatoires)
- Non testable automatiquement au-delà de `LevelSizeValidation` (TACHE-01) et
  `LevelDraft::resize`/`requestResize` (existants) : `EditorScreen` n'est pas compilé dans
  `UnitTests` (dépendance au rendu D3D11, comme le reste de la classe depuis LOT-14/15) — vérifié
  par relecture et par essai manuel (voir DoD).
- Non-régression : les tests existants de nommage/renommage (`TextInputField`,
  `LevelNameValidation`) restent valides après la généralisation du slot (le renommage du champ
  interne ne change aucun comportement observable).

## Points d'attention
- Le format pré-rempli (`largeur x hauteur`) doit être celui que `parseLevelSize` accepte en
  entrée sans modification — valider `Ctrl+R` puis `Entrée` immédiatement (sans rien taper) doit
  être un no-op valide, pas un refus.
- `Ctrl+R` ne doit rien faire si `_textPrompt`, `_pendingConfirmation`, `_playtest` ou `_picker`
  sont déjà actifs (même garde que `F2`, cf. l'ordre des blocs dans `update()`).

## Définition de fait (DoD)
- Boîte de dialogue opérationnelle ; build `/W4 /WX` ; Doxygen à jour ; vérifiée manuellement
  (ouverture pré-remplie, saisie valide redimensionne, saisie invalide refusée avec message,
  `Échap` sans effet, confirmation destructrice déclenchée si applicable).

## Exigences
`EX-EDIT-017` (partie « saisie directe »).
