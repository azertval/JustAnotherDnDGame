# TACHE-03 — Choix et prévisualisation du cadrage dans l'éditeur {#lot-64-tache-03-choix-previsualisation-editeur}

**Lot :** [LOT-64](epic.md) · **Emplacement :** `Source/HMI/Editor` · **Statut :** fait

## Contexte
Les deux tâches précédentes rendent le cadrage **exprimable** et **applicable**. Sans celle-ci, il
reste inaccessible : le designer devrait éditer le JSON à la main, ce que tout le programme
d'éditeur — quatorze lots — existe précisément pour éviter (`EX-VIS-006`).

Le point important n'est pas le sélecteur, qui est trivial, mais la **prévisualisation**. Choisir un
cadrage sans le voir revient à lancer l'essai à chaque essai de réglage. L'éditeur affiche déjà un
repère visuel de salles (`LOT-32`), qui est exactement l'amorce de ce qu'il faut.

## Travail à réaliser
- **Sélecteur de mode** dans les propriétés du niveau, avec les paramètres du mode retenu (taille de
  salle pour *par salle*). Les modes portent des noms traduits, pas des identifiants techniques.
- **Prévisualisation dans le canevas** :
  - *niveau entier* — le cadre du niveau ;
  - *par salle* — la grille de salles, en généralisant le repère existant à une taille variable ;
  - *suivi* — le rectangle visible autour d'une position donnée, avec la zone morte matérialisée.
- **Réutiliser `hmi::RoomGrid`** pour le repère de salles plutôt que d'en réécrire un, et
  `hmi::DraftRenderer` pour le tracé — comme le font déjà les liens de mécanismes et les poignées de
  décors.
- **Intégration à l'architecture d'information du `LOT-57`** : le mode courant apparaît dans la
  **barre d'état** (ce qui informe est permanent), le sélecteur est une commande **unique** (pas
  dupliquée entre menu et panneau).
- **Undo/redo** : changer de mode est une opération d'édition annulable, nommée, dans
  `core::LevelDraft` — pas un réglage hors historique.
- **Marquer le niveau comme modifié**, pour que les garde-fous de perte de données du `LOT-15`
  s'appliquent.

## Fichiers impactés
- `Source/HMI/Editor/` — panneau de propriétés du niveau (nouveau ou étendu), libellés dans
  `TaxonomyLabels` si la taxonomie s'y prête.
- `Source/HMI/Graphics/DraftRenderer.{h,cpp}` — tracé de la prévisualisation.
- `Source/HMI/Graphics/RoomGrid.{h,cpp}` — repère à taille variable.
- `Source/Core/Levels/LevelDraft.{h,cpp}` — opération annulable.
- `Source/HMI/Editor/EditorStatus.{h,cpp}` — ligne de barre d'état.
- `Source/Elements/Localization/{fr,en}.lang`.
- `Source/Test/Unit/Core/Levels/test_level_draft.cpp`, `Source/Test/Unit/HMI/Editor/test_editor_status.cpp`,
  `Source/Test/Unit/HMI/Graphics/test_room_grid.cpp` (étendus).

## Tests (obligatoires)
- Changer de mode est **annulable** et **rétablissable**, et marque le niveau comme modifié.
- Le mode choisi est enregistré et rechargé à l'identique (round-trip complet éditeur → fichier →
  éditeur).
- La barre d'état affiche le mode courant, dans les **deux** catalogues de traduction.
- Le repère de salles est correct pour une taille de salle **personnalisée**, y compris quand la
  dernière salle est tronquée en bord de niveau.
- La prévisualisation ne compose aucune primitive en dehors de l'éditeur — asserté via le
  *QuadRecorder*, comme le fait déjà le repère de salles.

## Points d'attention
- **Le round-trip est le point de rupture habituel** : si `core::LevelDraft` ne porte pas le champ,
  le premier enregistrement depuis l'éditeur efface silencieusement le mode. La `TACHE-01` doit
  l'avoir traité ; le vérifier ici de bout en bout.
- Ne pas dupliquer la commande : le `LOT-57` a fait de l'unicité des commandes un principe, et le
  dispatch contextuel d'annulation/rétablissement ne doit pas être modifié.
- Attention aux commentaires XML des fichiers `.ui` : un double tiret `--` casse `uic` avec une
  erreur « Expected '>' » qui ne mentionne pas le commentaire.
- La prévisualisation du mode *suivi* suppose une position de référence : utiliser l'**entrée** du
  niveau, seule position toujours définie et significative.
- Ne pas rendre les paramètres de la caméra de suivi éditables : hors périmètre, ce sont des
  constantes.

## Définition de fait (DoD)
- Le designer choisit le mode de cadrage depuis l'éditeur, le voit prévisualisé dans le canevas, le
  retrouve dans la barre d'état, peut annuler son changement, et l'enregistrement conserve le choix ;
  le repère de salles gère une taille variable ; traduit ; `/W4 /WX` propre.

## Exigences
`EX-EDIT-028` (choix et prévisualisation du cadrage) ; réutilise `EX-LVL-006` (mode porté par le
niveau), `EX-REN-016` (modes), `EX-EDIT-010` (réutilisation du modèle de `Core`), `EX-VIS-006`
(éditeur pour non-développeurs), `EX-REN-033` (traduction), `EX-NFR-004` (vérification sans GPU).
