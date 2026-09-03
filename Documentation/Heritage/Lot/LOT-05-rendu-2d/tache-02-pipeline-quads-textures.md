# TACHE-02 — Pipeline de quads texturés (HLSL, blend, nearest) {#lot-05-tache-02-pipeline-quads-textures}

**Lot :** [LOT-05](epic.md) · **Emplacement :** `Source/HMI/Graphics` · **Statut :** fait

## Contexte
`GraphicsDevice` sait effacer et présenter, mais ne dessine rien. Cette tâche pose le
**pipeline de rendu 2D** : dessiner des rectangles texturés (quads) avec transparence et
un échantillonnage adapté au pixel art.

## Travail à réaliser
- **Shaders HLSL** (vertex + pixel) compilés à l'exécution via `D3DCompile` : le vertex
  shader applique une matrice (projection écran + caméra) ; le pixel shader échantillonne
  la texture d'atlas et applique la teinte.
- **Buffers** : vertex/index buffers pour des quads (position, UV, couleur), en RAII.
- **États** : *blend state* alpha (transparence, `EX-REN-011`), *sampler* **point/nearest**
  (`EX-ARCH-022`), rasterizer 2D (pas de depth test, ou depth désactivé).
- Une classe `SpriteBatch` (ou `QuadRenderer`) : `begin()/draw(quad)/end()` accumulant
  les quads et émettant les *draw calls* (batch par texture d'atlas).

## Fichiers impactés
- `Source/HMI/Graphics/SpriteBatch.h`, `SpriteBatch.cpp` (nouveau).
- Shaders HLSL (inline dans le `.cpp` ou `Source/HMI/Graphics/*.hlsl`).
- `Source/HMI/CMakeLists.txt` (nouvelles sources).

## Vérifications (obligatoires)
- Un quad texturé s'affiche à une position/taille données (vérification visuelle via la
  démo de TACHE-06).
- La transparence fonctionne (zones alpha de l'atlas) ; l'échantillonnage est net (nearest).
- Aucune fuite de ressource DirectX (RAII, `ComPtr`).

## Points d'attention
- RAII obligatoire sur toutes les ressources D3D (`ID3D11Buffer`, shaders, states) via
  `Microsoft::WRL::ComPtr` ; aucun `init()`/`cleanup()` manuel.
- `Core` reste ignorant de ce pipeline (dépendance `HMI → Core` uniquement).
- Gérer les erreurs d'initialisation (device, compilation shader) par exception à la
  frontière de démarrage (cf. politique d'erreurs).

## Définition de fait (DoD)
- Pipeline de quads texturés fonctionnel (blend + nearest), RAII, documenté ;
  build `/W4 /WX` sans avertissement.

## Exigences
`EX-REN-002`, `EX-REN-011`, `EX-ARCH-022`, `EX-NFR-041`.
