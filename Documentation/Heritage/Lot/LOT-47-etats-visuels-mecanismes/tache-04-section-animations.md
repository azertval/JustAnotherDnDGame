# TACHE-04 — Section « Animations » du panneau « Textures » {#lot-47-tache-04-section-animations}

**Lot :** [LOT-47](epic.md) · **Emplacement :** `Source/HMI/Editor` · **Statut :** fait

## Contexte
Les clips sont décrits dans les fichiers d'assets (LOT-46) et demandés par la correspondance
état → clip (TACHE-01). Reste à choisir **quel asset animé** chaque famille de mécanisme utilise par
défaut, sans quoi il faudrait assigner une surcharge (LOT-45) sur chaque porte de chaque niveau.

C'est la quatrième section du panneau « Textures », et elle suit la même règle que les précédentes :
elle s'ajoute, elle ne remplace rien.

## Travail à réaliser
- **Section « Animations »** : pour chaque famille de mécanisme (porte, interrupteur, plaque,
  danger commuté, danger temporisé, danger mobile), sélection de l'asset animé par défaut parmi
  `Assets/Skins/` (même dossier et même vignette-sélecteur que la section « Skins », `LOT-43`) —
  un mécanisme est un type de tuile skinnable comme un autre, `Assets/Objects/` reste réservé aux
  surcharges **par case** de `LOT-45`.
- **Stockage** : dans le jeu de skins courant de `skins.json` (LOT-42) — un mécanisme est un type de
  tuile, son asset par défaut est donc une association type → asset, exactement comme un skin. Aucun
  nouveau fichier de configuration (`hmi::SkinCatalog` n'a reçu aucune extension de format).
- **Diagnostic des clips** : pour l'asset sélectionné, indiquer quels clips attendus par la
  correspondance sont **présents** et lesquels **manquent**. C'est l'information qui permet à
  l'auteur de savoir ce qu'il lui reste à dessiner, sans lancer le jeu.
- **Aperçu** : lecture de l'animation sélectionnée dans le panneau, pour vérifier le rythme sans
  entrer en mode essai.
- Traduction de toutes les chaînes.

## Fichiers impactés
- `Source/HMI/Editor/TexturePanel.{h,cpp}`, `Source/Elements/UI/TexturePanel.ui`.
- `Source/HMI/Editor/MechanismAnimationAssignments.{h,cpp}` (nouveau) : lignes de la section et
  diagnostic de clips manquants, logique pure testée sans Qt.
- `Source/Elements/Localization/fr.lang`, `en.lang`.
- `Source/Elements/Assets/skins.json`, `Source/Elements/Assets/Skins/{door,switch,plate,
  danger_switched,danger_blink,danger_mover}.png{,.anim.json}` (nouveau) : contenu de démonstration
  assigné par défaut, généré par `scripts/generate_mechanism_animations.py` (nouveau).
- `Source/Test/Unit/HMI/Graphics/test_skin_catalog.cpp` : le test de conformité du catalogue livré
  supposait un skin `single` toujours strictement 16×16 ; corrigé pour accepter la bande
  horizontale d'un skin `single` **animé** (16×(16×N), déjà documentée dans `Skins/README.md`
  depuis LOT-46, mais jamais exercée par le catalogue livré avant ce lot).

## Tests (obligatoires)
- **Diagnostic des clips** : pour un catalogue d'animation donné et une famille de mécanisme donnée,
  la liste des clips manquants est correcte. Fonction **pure**, testée sans Qt.
- L'assignation d'un asset animé à une famille est bien écrite dans `skins.json` et relue.

## Points d'attention
- **Ne pas créer un second format de configuration.** Réutiliser `skins.json` évite d'avoir deux
  fichiers décrivant l'apparence d'un même type de tuile, avec les incohérences que cela produit.
- L'aperçu doit utiliser le **même** catalogue d'animation que le jeu, pas une lecture parallèle :
  sinon l'aperçu pourrait montrer un rythme différent de celui joué.
- Le diagnostic de clips manquants doit rester **informatif**, pas bloquant : un asset partiel est
  un état de travail légitime.

## Définition de fait (DoD)
- Chaque famille de mécanisme peut recevoir un asset animé par défaut, stocké dans `skins.json` ;
  les clips manquants sont signalés dans le panneau ; l'aperçu joue l'animation ; chaînes
  traduites ; diagnostic testé sans Qt ; `/W4 /WX` propre.

## Exigences
`EX-REN-006` (apparence pilotée par l'état) ; réutilise `EX-EDIT-042` (association type → asset),
`EX-EDIT-024` (jeux de skins), `EX-EDIT-026` (bibliothèque d'assets), `EX-REN-005` (animations par
données), `EX-REN-033` (traduction).
