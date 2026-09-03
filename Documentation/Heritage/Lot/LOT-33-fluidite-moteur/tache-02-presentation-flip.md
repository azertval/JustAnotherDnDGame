# TACHE-02 — Présentation flip-model (latence réduite) {#lot-33-tache-02-presentation-flip}

**Lot :** [LOT-33](epic.md) · **Module :** `Source/HMI/Graphics` · **Statut :** terminé

## Contexte
La swap chain était créée avec l'ancien modèle de présentation *blt*
(`DXGI_SWAP_EFFECT_DISCARD`, un seul back buffer). Sous Windows 10/11, ce modèle impose au
compositeur (DWM) une **copie supplémentaire** du back buffer à chaque présentation, ce qui ajoute
de la latence entrée → image et nuit à la régularité de la cadence. Le modèle **flip**
(`DXGI_SWAP_EFFECT_FLIP_DISCARD`) présente le back buffer **directement**, sans cette copie —
c'est la présentation recommandée pour toute application Direct3D moderne, y compris V-Sync activée.

## Travail à réaliser
- `GraphicsDevice` : swap chain en `DXGI_SWAP_EFFECT_FLIP_DISCARD` avec **deux** back buffers
  (`BufferCount = 2`), format `R8G8B8A8_UNORM` (compatible flip), `SampleDesc.Count = 1`.
- Reliure de la cible de rendu **par frame** : le flip model dé-lie la cible de rendu du back buffer
  à `Present` ; `clear()` la relie donc (`OMSetRenderTargets`) en tête de frame avant tout dessin.
- `resize()` : dé-lier explicitement la cible de rendu (`OMSetRenderTargets(0, nullptr, nullptr)`)
  avant `ResizeBuffers`, en plus de libérer la render target view (une liaison résiduelle compterait
  comme une référence vivante et ferait échouer le redimensionnement).
- V-Sync inchangée : `present()` continue d'utiliser l'intervalle de synchronisation selon
  `vsyncEnabled()` (`EX-REN-022`).

## Fichiers impactés
- `Source/HMI/Graphics/GraphicsDevice.h` : documentation de la classe (flip model).
- `Source/HMI/Graphics/GraphicsDevice.cpp` : description de la swap chain, `clear()` (reliure),
  `resize()` (dé-liaison avant `ResizeBuffers`).

## Tests (obligatoires)
- Dépendance D3D11 : vérification **visuelle** (le jeu démarre, affiche, se redimensionne, reste
  net) — comme tout le rendu depuis `LOT-05`. Aucun test unitaire GPU.
- Non-régression de la suite (aucun code de simulation touché) : reste verte.

## Définition de fait (DoD)
- Fenêtre créée en flip model, rendu correct, redimensionnement sans artefact ni erreur DXGI,
  V-Sync toujours activable.
- Compile `/W4 /WX`, formaté, API documentée.

## Exigences
`EX-REN-004`, `EX-REN-002`, `EX-REN-022`, `EX-NFR-041`.
