# TACHE-03 — Section « Fond » et sélecteur de jeu de skins {#lot-44-tache-03-section-fond}

**Lot :** [LOT-44](epic.md) · **Emplacement :** `Source/HMI/Editor`, `Source/HMI/CMakeLists.txt` · **Statut :** fait

## Contexte
Les deux champs ajoutés au niveau en TACHE-01 doivent être éditables. C'est la **deuxième** section
du panneau « Textures » créé en LOT-42, et le premier test de son extensibilité : elle doit s'ajouter
sans toucher à la section Skins ni au câblage de `MainWindow`.

Différence importante avec la section Skins : celle-ci édite le **niveau courant**, pas une
configuration globale. Elle doit donc passer par `core::LevelDraft` et bénéficier de son historique.

## Travail à réaliser
- **Section « Fond »** : sélection d'un asset parmi `Assets/Backgrounds/` (grille de vignettes de
  LOT-43), plus une entrée « aucun fond » explicite — retirer un fond doit être aussi simple que
  d'en poser un.
- **Sélecteur de jeu de skins du niveau** : liste des jeux de `skins.json`, plus une entrée
  « jeu par défaut ». Placé avec la section Fond car c'est aussi une propriété **du niveau**, à ne
  pas confondre avec le sélecteur de jeu **courant d'édition** de LOT-42.
- **Écriture via `LevelDraft`** : les deux modifications passent par les mutateurs de TACHE-01,
  marquent le brouillon comme modifié, et sont annulables.
- **Rafraîchissement** : changer l'un ou l'autre met immédiatement à jour le canevas et la palette
  (LOT-42, TACHE-05).
- **Dossier `Assets/Backgrounds/`** + copie `POST_BUILD` dans `Source/HMI/CMakeLists.txt`.
- Traduction de toutes les chaînes, clés ajoutées aux deux catalogues.

## Fichiers impactés
- `Source/HMI/Editor/TexturePanel.{h,cpp}`, `Source/Elements/UI/TexturePanel.ui`.
- `Source/Elements/Assets/Backgrounds/` (nouveau dossier), `Source/HMI/CMakeLists.txt`.
- `Source/Elements/Localization/fr.lang`, `en.lang`.

## Tests (obligatoires)
- Effet d'une sélection sur le brouillon : assignation, retrait, annulation, rétablissement —
  logique pure testée sans Qt, sur `core::LevelDraft`.
- Le sélecteur de jeu du **niveau** et celui du **jeu courant d'édition** sont bien deux valeurs
  distinctes et ne se contaminent pas.

## Points d'attention
- **Ne pas confondre les deux sélecteurs de jeu de skins.** L'un est une propriété persistée du
  niveau, l'autre un réglage de session de l'éditeur. Les libellés doivent lever l'ambiguïté sans
  qu'on ait à consulter la documentation.
- Le fond fait partie du niveau : le modifier doit déclencher le garde-fou de perte de travail
  existant (confirmation avant de quitter sans enregistrer).
- Ne pas afficher le sélecteur de fond en mode Physique sans indication : l'utilisateur ne verrait
  aucun effet. Signaler que le rendu du fond requiert le mode Texture (`F8`).

## Définition de fait (DoD)
- Le fond et le jeu de skins du niveau sont éditables depuis le panneau, annulables, persistés à
  l'enregistrement, et leur effet est immédiat dans le canevas ; le dossier est déployé ; toutes les
  chaînes sont traduites ; `/W4 /WX` propre.

## Exigences
`EX-REN-044` (fond de niveau), `EX-EDIT-024` (jeu de skins par niveau) ; réutilise `EX-EDIT-026`
(bibliothèque d'assets), `EX-EDIT-005` (annuler/refaire), `EX-REN-033` (traduction), `EX-IHM-010`
(fenêtre à panneaux).
