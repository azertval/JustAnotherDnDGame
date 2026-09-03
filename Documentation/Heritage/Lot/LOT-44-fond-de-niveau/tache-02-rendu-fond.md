# TACHE-02 — Rendu du fond de niveau {#lot-44-tache-02-rendu-fond}

**Lot :** [LOT-44](epic.md) · **Emplacement :** `Source/HMI/Graphics`, `Source/HMI/Game` · **Statut :** fait

## Contexte
Le viewport efface aujourd'hui son back buffer avec une couleur unie bleu-gris
(`GameViewport::renderFrame`) : c'est tout ce qu'il y a derrière les tuiles. Le calque *Background*
de *RenderLayer* a été réservé en LOT-40 sans être utilisé ; cette tâche est la première à
l'activer.

## Travail à réaliser
- **Composition** : quand le niveau désigne un asset de fond **et** que le mode est `Texture`,
  émettre un quad sur le calque *Background*, couvrant les bornes du niveau, avant tout le reste.
- **Ratio d'aspect préservé, recadrage par le centre** (décision de cadrage) : l'image conserve ses
  proportions et déborde sur la dimension excédentaire, plutôt que d'être étirée. Le calcul des UV
  de recadrage est une fonction **pure**, testable.
- **Trois cas distincts**, à ne pas confondre :
  - pas de fond désigné → **rien** n'est émis ; l'effacement uni reste visible ; ce n'est pas une
    anomalie, aucun avertissement ;
  - fond désigné et chargeable → le quad est émis ;
  - fond désigné mais **introuvable ou invalide** → damier magenta + `GRAPHICS_LOG_WARNING` nommant
    l'asset (`EX-NFR-040`).
- **Mode Physique** : aucun fond, jamais — la lecture des collisions doit rester sans distraction.
- **Culling** : le quad de fond couvre le niveau entier ; son test de visibilité doit porter sur sa
  **boîte englobante**, pas sur son point d'ancrage, sinon il disparaîtrait dès que la caméra
  s'éloigne du coin haut-gauche (piège signalé en LOT-40, TACHE-05).

## Fichiers impactés
- `Source/HMI/Graphics/BackgroundRenderer.{h,cpp}` (nouveau) ou extension de `SpriteRenderer`.
- `Source/HMI/Game/GameSession.{h,cpp}`, `Source/HMI/Graphics/DraftRenderer.{h,cpp}`.
- `Source/Test/Unit/HMI/Graphics/test_background_fit.cpp` (nouveau).

## Tests (obligatoires)
- **Cadrage** : pour plusieurs couples (ratio d'image, ratio de niveau), les UV calculés préservent
  les proportions et centrent le recadrage — fonction pure.
- **Les trois cas** ci-dessus, assertés via le *QuadRecorder* : aucun quad émis sans fond désigné ;
  un quad sur *Background* avec fond valide ; un quad de repli avec fond introuvable.
- Aucun quad de fond en mode Physique.

## Points d'attention
- Le fond est en **espace niveau**, pas en espace écran : il est cadré par la caméra comme le reste
  du contenu, y compris lors d'une bascule de salle (`EX-REN-015`). Il ne « suit » pas la caméra.
- Ne pas remplacer l'effacement du back buffer par le fond : l'effacement reste nécessaire (zones
  hors niveau, marges de cadrage).
- Le fond doit être **sous les ombres** (calque *Shadow*, LOT-55) : l'ordre des calques de LOT-40 le
  garantit, rien de particulier à faire, mais c'est à vérifier au moment de LOT-55.

## Définition de fait (DoD)
- Un fond valide s'affiche sous tout le contenu en mode Texture, sans déformation ; l'absence de
  fond et le fond introuvable sont traités différemment ; rien en mode Physique ; cadrage et cas de
  composition testés sans GPU ; `/W4 /WX` propre.

## Exigences
`EX-REN-044` (fond de niveau) ; réutilise `EX-REN-043` (calques), `EX-REN-046` (bascule),
`EX-REN-007` (contrat d'asset), `EX-NFR-040` (repli), `EX-NFR-004` (vérification sans GPU),
`EX-NFR-005` (culling).
