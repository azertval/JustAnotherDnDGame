# TACHE-04 — Effacement écran, présentation & redimensionnement {#lot-01-tache-04-effacement-presentation}

**Lot :** [LOT-01](epic.md) · **Module :** `Source/HMI` · **Statut :** terminé

## Contexte
Avec la fenêtre (TACHE-01) et le device (TACHE-02), on produit la première image : un écran effacé à une couleur, présenté sans tearing, et robuste au redimensionnement.

## Travail à réaliser
- Ajouter à `GraphicsDevice` (ou une classe `Renderer` mince au-dessus) :
  - **Effacement** de la render target view à une couleur constante (`ClearRenderTargetView`).
  - **Présentation** via `IDXGISwapChain::Present` avec **V-Sync** (intervalle de synchronisation = 1).
- Implémenter le **redimensionnement** : sur événement de resize (TACHE-01), relâcher la render target view, appeler `ResizeBuffers`, recréer la render target view et le viewport.
- Ignorer le resize lorsque la fenêtre est **minimisée** (taille client nulle).

## Fichiers impactés
- `Source/HMI/GraphicsDevice.*` (ou `Source/HMI/Renderer.*` nouveau).
- `Source/HMI/CMakeLists.txt` si nouveaux fichiers.

## Points d'attention
- Ordre correct du redimensionnement : **libérer** les vues liées au back buffer **avant** `ResizeBuffers`, sinon échec.
- `Present(1, …)` pour la V-Sync (`EX-REN-022`).
- Aucune fuite lors des redimensionnements répétés (vérifiable sous AddressSanitizer et couche debug D3D).

## Définition de fait (DoD)
- La fenêtre affiche une couleur stable, présentée sans tearing.
- Redimensionnements répétés (dont minimisation/restauration) sans crash ni fuite.
- Compile `/W4 /WX`, formaté, API documentée en Doxygen.

## Exigences
`EX-REN-022`, `EX-REN-003`.
