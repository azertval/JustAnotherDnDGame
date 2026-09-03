# LOT-52 — Texte, police bitmap et affichage tête haute {#lot-52}

> Statut : **fait**. Prérequis : [LOT-40](@ref lot-40) (*TextureCache*, calque *UI*),
> [LOT-43](@ref lot-43) (bibliothèque d'assets).

## Objectif
Donner au jeu la capacité d'**afficher du texte dans la scène rendue**, et s'en servir pour montrer
au joueur une information qui existe depuis longtemps sans avoir jamais été visible.

`EX-REN-032` (« le jeu doit afficher du texte ») a bien été implémentée par le passé — une police
bitmap *hmi::BitmapFont* alimentait l'interface « maison » — puis **délibérément retirée** au
`LOT-38` (commit `8338bc15`) avec toute la pile d'UI historique, Qt reprenant l'affichage du texte
**hors-jeu**. Le raisonnement était juste : un menu Qt n'a rien à faire dans `SpriteBatch`.

Mais la conséquence, elle, n'a pas été traitée : il n'existe plus **aucune** notion de police ni de
glyphe dans `Source/HMI/Graphics`, et le viewport Direct3D 11 ne sait donc afficher aucun texte
**dans la scène de jeu**. Qt compose ses widgets par-dessus la fenêtre, dans une couche indépendante
de la caméra du monde ; il ne peut pas afficher une information ancrée au jeu.

La conséquence la plus visible : `LOT-12` a introduit des **budgets de sauts et de dashs** par
tableau (`EX-GP-024`) — une contrainte de puzzle qui refuse une action quand le budget est épuisé —
et le joueur n'a **aucun moyen de savoir combien il lui en reste**.

Ce lot réintroduit donc la brique de texte **du bon côté de la frontière** : dans le pipeline de
rendu de la scène, pas dans l'interface hors-jeu. L'implémentation retirée au `LOT-38` reste
consultable dans l'historique Git (`Source/HMI/Graphics/BitmapFont.{h,cpp}`) et constitue un point
de départ, à condition de la rebrancher sur le *TextureCache* et les calques de `LOT-40` plutôt que
sur l'ancien chemin.

## Périmètre

### Inclus
- **Police bitmap** : atlas de glyphes (PNG) accompagné de ses métriques (fichier de description),
  chargé par le *TextureCache* et validé par le contrat d'asset (LOT-40). Repli procédural si l'atlas
  est absent, dans la lignée de `EX-REN-042`.
- ***TextRenderer*** : composition d'une chaîne en une suite de quads soumis sur le calque *UI* de
  *RenderLayer* (réservé en LOT-40), via `SpriteBatch` — **aucune nouvelle dépendance**, aucun
  nouveau chemin de rendu.
- **Affichage tête haute minimal** (`EX-IHM-003`) : budgets de sauts et de dashs restants, nom du
  tableau en cours. Affiché en jeu et en mode essai.
- **Traduction** : tout libellé passe par `hmi::Localization` (`EX-REN-033`) ; les chaînes sont
  ajoutées aux deux catalogues de langue.
- Mesure de texte (largeur, hauteur) exposée comme fonction **pure**, pour permettre le cadrage sans
  GPU.

### Exclus (hors périmètre de ce lot)
- Écrans de pause et de fin de niveau (`EX-REN-031`, non implémentée) : ce lot fournit la brique
  d'affichage, pas les écrans. Ils restent hors du programme d'habillage.
- Police vectorielle, crénage, texte multi-lignes justifié, texte enrichi.
- Dialogues, sous-titres, tutoriel textuel — la conception des niveaux reste un tutoriel implicite
  sans texte.
- Affichage de diagnostic (compteur d'images, boîtes de collision) : la brique le permettra, ce lot
  ne le livre pas.

## Décisions de cadrage
- **Réintroduction ciblée, pas retour en arrière** : le retrait de *hmi::BitmapFont* au `LOT-38`
  était justifié pour l'**interface hors-jeu** et n'est pas remis en cause — menus, options et
  éditeur restent en Qt. Ce lot ne rétablit le texte que **dans la scène rendue**, là où Qt ne peut
  structurellement pas intervenir.
- **Police bitmap, pas de rendu de police vectorielle** : le jeu est en pixel art avec filtrage
  *nearest* (`EX-ARCH-022`) ; une police bitmap est le rendu **correct**, pas un compromis. Elle
  réutilise intégralement le chemin existant (atlas + quads + `SpriteBatch`) et n'ajoute aucune
  dépendance, là où un moteur de texte vectoriel en imposerait une.
- **Le texte est un asset comme un autre** : même cache, même contrat, même repli. Aucune exception
  dans le pipeline.
- **Périmètre du HUD volontairement minimal** : les budgets et le nom du tableau, rien d'autre. Le
  besoin est identifié et précis ; élargir maintenant reviendrait à concevoir une interface de jeu
  sans en avoir établi le besoin.
- **Placé dans le programme d'habillage** parce qu'il en partage entièrement l'infrastructure
  (*TextureCache*, calques, contrat d'asset). Il ne dépend d'aucun autre lot que LOT-40, et peut donc
  être avancé si le besoin devient prioritaire.

## Exigences couvertes
- Nouvelle : `EX-IHM-003` (affichage tête haute des budgets et du tableau courant).
- Re-concrétisée : `EX-REN-032` (affichage de texte, cette fois **dans la scène rendue** — l'ancienne
  implémentation d'UI a été retirée au `LOT-38`).
- Réutilisées : `EX-REN-033` (traduction), `EX-REN-042` (assets externalisés avec repli),
  `EX-REN-007` (contrat d'asset), `EX-REN-043` (calques), `EX-GP-024` (budgets de mouvements),
  `EX-ARCH-012` (aucun effet sur la simulation), `EX-ARCH-022` (*nearest*).

## Découpage

> État : ✅ fait · 🔄 en cours · ⬜ non commencé.

| Tâche | Intitulé | Emplacement | État |
|-------|----------|-------------|:----:|
| [TACHE-01](tache-01-police-bitmap.md) | Police bitmap : atlas de glyphes, métriques, chargement, repli procédural | `Source/HMI/Graphics` | ✅ |
| [TACHE-02](tache-02-text-renderer.md) | *TextRenderer* : composition d'une chaîne en quads sur le calque *UI*, mesure pure | `Source/HMI/Graphics` | ✅ |
| [TACHE-03](tache-03-hud.md) | Affichage tête haute (budgets, nom du tableau) + clés de traduction | `Source/HMI/Game`, `Source/Elements/Localization` | ✅ |

## Critères d'acceptation du lot
1. Une chaîne s'affiche dans le viewport du jeu, nette, sans filtrage flou.
2. Les budgets de sauts et de dashs restants sont visibles en jeu et décroissent à l'usage ; un
   niveau sans budget n'affiche rien de superflu.
3. Tous les libellés passent par le catalogue de traduction et existent dans les deux langues.
4. En l'absence d'atlas de police, le jeu reste lisible (repli) et ne plante pas.
5. La mesure de texte est testée sans GPU ; build `/W4 /WX`, Doxygen, lint verts.

## Dépendances
Bâtit sur [LOT-40](@ref lot-40) (*TextureCache*, calque *UI*, contrat d'asset) et
[LOT-43](@ref lot-43) (import d'assets). Rétablit, du côté de la scène rendue, une capacité retirée
par [LOT-38](@ref lot-38). Aucune dépendance sur les autres lots du programme : il peut être avancé
dans la séquence si le besoin devient prioritaire.

## Navigation des tâches
- @subpage lot-52-tache-01-police-bitmap
- @subpage lot-52-tache-02-text-renderer
- @subpage lot-52-tache-03-hud
