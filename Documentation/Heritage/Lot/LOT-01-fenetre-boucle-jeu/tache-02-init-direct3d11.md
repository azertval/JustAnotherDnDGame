# TACHE-02 — Initialisation Direct3D 11 (RAII) {#lot-01-tache-02-init-direct3d11}

**Lot :** [LOT-01](epic.md) · **Module :** `Source/HMI` · **Statut :** terminé

## Contexte
Une fois la fenêtre disponible (TACHE-01), il faut initialiser le pipeline Direct3D 11 qui servira de base au rendu 2D des lots suivants.

## Travail à réaliser
- Créer une classe `GraphicsDevice` (RAII) encapsulant :
  - `D3D11CreateDeviceAndSwapChain` (device, contexte immédiat, swap chain lié au `HWND`).
  - Création de la **render target view** à partir du back buffer.
  - Configuration du **viewport** à la taille du client.
- Gérer les ressources via des **pointeurs intelligents COM** (`Microsoft::WRL::ComPtr`) pour une libération automatique.
- Activer le **niveau de debug** Direct3D en configuration Debug uniquement (`D3D11_CREATE_DEVICE_DEBUG`).
- Exposer les méthodes nécessaires à TACHE-04 : accès au contexte, à la render target view, à la swap chain, et une méthode de **redimensionnement** des buffers.

## Fichiers impactés
- `Source/HMI/GraphicsDevice.h`, `Source/HMI/GraphicsDevice.cpp` (nouveau).
- `Source/HMI/CMakeLists.txt` (liaison `d3d11`, `dxgi`, `d3dcompiler` selon besoin).

## Points d'attention
- **RAII strict** (`EX-NFR-041`) : aucune libération manuelle, tout via `ComPtr`/destructeur.
- Vérifier chaque `HRESULT` ; une erreur d'initialisation est **irrécupérable** → exception à la frontière de démarrage (cf. politique d'erreurs des conventions).
- En Debug, aucune fuite signalée par la couche de debug D3D à la fermeture.

## Définition de fait (DoD)
- Le device et la swap chain s'initialisent sur la fenêtre sans erreur `HRESULT`.
- La couche de debug D3D ne rapporte ni erreur ni fuite en Debug.
- Compile `/W4 /WX`, formaté, API documentée en Doxygen.

## Exigences
`EX-REN-002`, `EX-NFR-041`.
