# LOT-01 — Fenêtre & boucle de jeu (Direct3D 11) {#lot-01}

> Statut : **terminé**. Premier lot : le socle d'exécution sur lequel tout le reste se greffe.

## Objectif
Disposer d'une application Windows qui **ouvre une fenêtre**, **initialise Direct3D 11**, tourne dans une **boucle de jeu à pas de temps fixe** et **efface l'écran** à une couleur, avec une fermeture et une libération des ressources propres (RAII).

À l'issue du lot : un exécutable qui affiche une fenêtre stable à 60 FPS, redimensionnable, se fermant proprement — sans encore aucun contenu de jeu.

## Périmètre

### Inclus
- Création d'une fenêtre Win32 (titre, icône, redimensionnable) et pompe de messages.
- Initialisation Direct3D 11 : device, contexte, swap chain, render target view.
- Boucle de jeu à **pas de temps fixe** (mise à jour déterministe) avec rendu découplé.
- Effacement de l'écran (clear color) et présentation (V-Sync).
- Gestion du **redimensionnement** de la fenêtre (redimensionnement du swap chain).
- Libération de toutes les ressources en RAII.

### Exclus (lots ultérieurs)
- Rendu de sprites, tuiles, textures.
- Entrées clavier/manette (LOT dédié).
- Logique de jeu, entités, niveaux.

## Exigences couvertes
- `EX-REN-001` — fonctionnement Windows 10/11 x64.
- `EX-REN-002` — rendu Direct3D 11.
- `EX-REN-003` — fenêtre Win32 redimensionnable, titre, icône.
- `EX-REN-020` — cible 60 images/seconde.
- `EX-REN-021` — logique à pas de temps fixe, rendu découplé.
- `EX-REN-022` — présentation synchronisée (V-Sync), pas de tearing.
- `EX-NFR-010` — logique de cadencement dans `Core`, testable sans fenêtre ni GPU.
- `EX-NFR-041` — ressources DirectX gérées en RAII.

## Découpage

> État : ✅ fait · 🔄 en cours · ⬜ non commencé.

| Tâche | Intitulé | Module | État |
|-------|----------|--------|:----:|
| [TACHE-01](tache-01-fenetre-win32.md) | Fenêtre Win32 & pompe de messages | `HMI` | ✅ Fait |
| [TACHE-02](tache-02-init-direct3d11.md) | Initialisation Direct3D 11 (RAII) | `HMI` | ✅ Fait |
| [TACHE-03](tache-03-boucle-pas-fixe.md) | Boucle à pas de temps fixe (testable) | `Core` | ✅ Fait |
| [TACHE-04](tache-04-effacement-presentation.md) | Effacement écran, présentation & redimensionnement | `HMI` | ✅ Fait |
| [TACHE-05](tache-05-integration.md) | Intégration `main` & vérification | `HMI` | ✅ Fait |

## Critères d'acceptation du lot
1. L'exécutable ouvre une fenêtre titrée, redimensionnable, et se ferme proprement (croix / Échap).
2. L'écran est effacé à une couleur constante et présenté sans tearing (V-Sync).
3. La fenêtre supporte le redimensionnement sans crash ni fuite (vérifié sous AddressSanitizer).
4. La logique de pas de temps fixe est couverte par des **tests unitaires** dans `Core`.
5. Build **sans avertissement** (`/W4 /WX`), `ctest` vert, **CI verte**.
6. API publique nouvelle documentée en Doxygen ; `CHANGELOG.md` mis à jour.

## Navigation des tâches
- @subpage lot-01-tache-01-fenetre-win32
- @subpage lot-01-tache-02-init-direct3d11
- @subpage lot-01-tache-03-boucle-pas-fixe
- @subpage lot-01-tache-04-effacement-presentation
- @subpage lot-01-tache-05-integration
