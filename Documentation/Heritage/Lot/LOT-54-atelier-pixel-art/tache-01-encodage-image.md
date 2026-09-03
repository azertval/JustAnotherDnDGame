# TACHE-01 — Encodage et enregistrement d'image {#lot-54-tache-01-encodage-image}

**Lot :** [LOT-54](epic.md) · **Emplacement :** `Source/HMI/Graphics` · **Statut :** fait

## Contexte
Le projet sait **lire** une image depuis le LOT-39 : `hmi::decodeImageFile` décode un fichier en
pixels RGBA non prémultipliés via `QImage`. Il ne sait pas en **écrire** — sauf dans un cas
particulier, l'outil `--export-atlas` de `main.cpp`, qui utilise `QImage::save` directement.

L'atelier a besoin de l'opération symétrique, sous une forme réutilisable et testable.

C'est la seule tâche du lot qui ne dépend ni de [LOT-56](@ref lot-56) ni de [LOT-57](@ref lot-57) :
elle ne touche à aucune interface. Elle peut donc être réalisée en premier, indépendamment de
l'ordre d'exécution retenu pour le reste.

## Travail à réaliser
- **`encodeImageFile` / `saveImageFile`**, symétriques de `decodeImageFile` : depuis un tampon de
  pixels RGBA et des dimensions, écrire un PNG à un chemin donné. **Jamais d'exception** — résultat
  porteur d'un code exploitable (dossier inexistant, disque plein, fichier en lecture seule).
- **Aller-retour exact** : décoder puis réencoder une image doit restituer les mêmes pixels, canal
  alpha compris et **non prémultiplié** (le blend state du rendu suppose du non-prémultiplié — c'est
  une source classique de bords sombres autour des sprites).
- **Écriture atomique** : écrire dans un fichier temporaire puis remplacer, afin qu'une interruption
  ne laisse pas un asset tronqué — d'autant que le rechargement à chaud (LOT-43) peut lire pendant
  l'écriture.
- **Réutilisation par `--export-atlas`** : faire passer l'outil existant par cette fonction plutôt
  que par `QImage::save` directement, pour n'avoir qu'un seul chemin d'écriture.

## Fichiers impactés
- `Source/HMI/Graphics/TextureLoader.{h,cpp}` (ajout de l'écriture, symétrie avec le décodage).
- `Source/HMI/main.cpp` (`--export-atlas` passe par la nouvelle fonction).
- `Source/Test/Unit/HMI/Graphics/test_image_encode.cpp` (nouveau).

## Tests (obligatoires)
- **Aller-retour** : encoder puis décoder restitue exactement les mêmes pixels, y compris des pixels
  totalement et partiellement transparents.
- Cas d'erreur : dossier inexistant, chemin invalide — résultat exploitable, **sans exception**.
- L'écriture atomique ne laisse aucun fichier temporaire résiduel après succès comme après échec.
- Sans GPU (le décodage et l'encodage `QImage` ne demandent pas de device graphique).

## Points d'attention
- **Le piège du prémultiplié.** `QImage` propose plusieurs formats alpha ; le projet utilise
  `Format_RGBA8888` (non prémultiplié) au décodage. L'écriture doit rester dans le même format,
  sinon les allers-retours dégraderont progressivement les bords des sprites.
- Ne pas écrire directement sur le fichier cible pendant qu'un `QFileSystemWatcher` éventuel
  (LOT-43) surveille le dossier : l'écriture atomique règle aussi ce problème.
- Conserver la politique d'erreurs du module : aucune exception dans le chemin de rendu ni dans
  l'édition.

## Définition de fait (DoD)
- Une image peut être écrite depuis un tampon RGBA, de façon atomique et sans exception ;
  l'aller-retour est exact, alpha compris ; `--export-atlas` passe par le même chemin ; tests sans
  GPU verts ; `/W4 /WX` propre.

## Exigences
`EX-EDIT-045` (outil de dessin pixel art) ; réutilise `EX-REN-041` (décodage image),
`EX-REN-042` (assets externalisés), `EX-NFR-040` (erreur récupérable), `EX-NFR-010` (testable sans
GPU).
