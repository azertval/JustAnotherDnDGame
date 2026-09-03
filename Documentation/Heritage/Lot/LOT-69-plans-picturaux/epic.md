# LOT-69 — Plans picturaux, parallaxe réglable et mode création {#lot-69}

> Statut : **livré**. Prérequis : [LOT-49](@ref lot-49) et [LOT-50](@ref lot-50)
> (le système qu'il remplace), [LOT-54](@ref lot-54) (atelier pixel art, réutilisé tel quel),
> [LOT-51](@ref lot-51) (visibilité par calque), [LOT-64](@ref lot-64) (modes de cadrage),
> `LOT-68` (espaces de travail de l'éditeur — documenté par ses exigences, sans dossier dédié).

## Objectif

Rendre le décor **peignable** plutôt que composé, et lui donner enfin de la profondeur.

Le système livré en `LOT-49`/`LOT-50` habille un niveau en **posant des sprites** : un décor est un
PNG placé à une position, avec échelle et rotation. Trois limites le rendent impropre à ce que le
level designer attend :

- **Trois couches figées.** `core::DecorLayer` ne vaut que `Background`, `Decor` ou `Foreground`, et
  `hmi::decorRenderLayer` projette même les deux premières sur le **même** calque de rendu. Sept
  plans de profondeur sont inexprimables.
- **Parallaxe codée en dur.** `Source/HMI/Graphics/Parallax.h` fige `0.5` / `1.0` / `1.15` ; aucun
  niveau ne règle quoi que ce soit. Pire, le facteur est **neutralisé en mode caméra suivi**
  (`LOT-64`) — le seul mode où la caméra défile réellement, donc le seul où la parallaxe se verrait.
  Dans les deux modes où elle s'applique, la caméra ne défile pas : ce n'est qu'un décalage statique.
- **Composer n'est pas dessiner.** Poser sept PNG ne permet pas de peindre une fresque. La demande
  — « le niveau complet en mode édition de texture » — est un changement de nature : le décor
  devient une **surface peinte à l'échelle du niveau**.

Le lot lève au passage une dette qui n'est pas celle des décors, mais qui retomberait entièrement
sur son mode création : `hmi::GameViewport` est une **fenêtre native** embarquée
(`QWidget::createWindowContainer`). Le projet a **déjà payé deux défauts réels** pour ce choix en
`LOT-59` — un écran de pause qui ne s'affichait pas, puis `Qt::Tool` volant le focus — contournés
par une fenêtre de haut niveau à géométrie synchronisée en coordonnées écran. Un mode création a
besoin de poignées, de règles et de repères **au-dessus** du canevas : les bâtir sur une fenêtre
native, ce serait reproduire délibérément une classe de défauts déjà constatée.

## Périmètre

### Inclus
- **Plans picturaux** (`EX-DEC-040` à `EX-DEC-045`) : nombre libre, ordre significatif, un PNG par
  plan à côté du niveau, densité réglable (16/8/4 px par unité), opacité, profondeur.
- **Format de niveau** correspondant (`EX-LVL-009`), champ `decors` obsolète **ignoré avec
  avertissement** plutôt que rejeté.
- **Parallaxe portée par le plan** (`EX-DEC-043`), par axe, activable par niveau, translation
  **bornée** pour que le plan couvre toujours le cadrage.
- **Mode création** (`EX-EDIT-046`) : troisième espace de travail, peinture 1:1 du niveau entier,
  pelure d'oignon, isolation de plan, zoom descendant **sous** le 1:1.
- **Panneau « Plans »** (`EX-EDIT-047`) : ordre, densité, parallaxe, opacité, visibilité, cycle de
  vie des fichiers.
- **Rendu sur QRhi** et fin de la fenêtre native (`EX-REN-050`), sans changer la cible Direct3D 11
  (`EX-REN-002`).
- **Budget de mémoire de texture** (`EX-NFR-043`), testé par niveau livré.
- **Retrait complet** du système de décors-sprites : format, modèle, gestes, panneau, tests, assets.

### Exclus
- **Le pipeline photo → pixel art** (`EX-DEC-030`/`031`) reste post-MVP. Le lot rend les plans
  peignables ; il n'ajoute pas de conversion d'image.
- **Aucun nouveau tableau n'est conçu.** `demo-final` est migré pour préserver son intention
  visuelle ; dessiner un niveau qui exploite vraiment sept plans est un acte de *level design*,
  distinct de ce lot d'outillage.
- **Le refactor du constructeur de `core::Level`** (19 paramètres après ce lot) n'est pas fait. La
  surface du lot est déjà maximale ; la dette est signalée, pas traitée.
- **Aucun objet décoratif ponctuel réutilisable** ne subsiste — voir la perte assumée ci-dessous.

## Décisions de cadrage

- **Remplacement net, pas cohabitation.** Maintenir décors et plans en parallèle doublerait le
  format, le rendu et l'éditeur pour un système destiné à disparaître. Le prix est explicite : il
  n'existe plus de motif décoratif **ponctuel réutilisable** — un tonneau présent dans dix niveaux
  doit être peint (ou collé) dans chaque plan.
- **Le CPU reste l'autorité sur les pixels ; le GPU n'est que l'affichage.** Qt Canvas Painter sait
  peindre dans une texture hors écran, mais l'historique exigerait alors une **relecture GPU par
  geste** et `hmi::PixelOperations` deviendrait intestable sans carte graphique (`EX-NFR-004`). Une
  `DecodedImage` par plan est la source de vérité ; seuls les **rectangles salis** sont téléversés.
  Corollaire : Canvas Painter est confiné derrière une façade, ce qui neutralise le risque lié à son
  statut *Technology Preview*.
- **Le canevas de peinture est `hmi::PixelCanvas`, pas un nouveau widget.** Il sait déjà peindre,
  sélectionner, annuler, coller et contraindre à une palette. Le viewport n'apporterait qu'une
  chose — la fidélité de la référence — contre la réécriture de tout le reste.
- **La référence est un repère géométrique, pas un aperçu.** Ni raccords automatiques, ni skins, ni
  animations. Le dire explicitement évite une attente déçue : l'aperçu fidèle reste l'essai.
- **Aucune valeur de calque par plan.** `EX-REN-014` impose un ordonnancement unique ; le nombre de
  plans est libre. Les plans occupent deux calques existants, leur rang fournissant le tri fin.
- **La parallaxe reste relative au centre de la salle.** Un décalage absolu en espace niveau avait
  été envisagé, puis écarté : la formule existante est précisément ce qui empêche le saut à la
  bascule de salle (`EX-REN-015`), et un plan continu n'y perd rien.
- **La parallaxe est réactivée en mode suivi**, inversant la décision du `LOT-64`. Elle l'avait
  coupée parce qu'un **décor** est un objet collé au contenu et paraissait « suivre » la caméra ; un
  plan est un fond, l'argument ne tient plus.
- **Les garde-fous de coût sont dans le format**, vérifiés au chargement, pas laissés à l'usage.
- **Qt 6.11 est un plancher, pas un confort.** C'est la première version fournissant Canvas Painter.
  Quitter Qt 6.8 LTS est assumé : au-delà de 6.8.3, cette branche n'est plus publiée qu'en licence
  commerciale — le support long terme ne bénéficiait pas à ce projet.

## Exigences couvertes

`EX-DEC-040`, `EX-DEC-041`, `EX-DEC-042`, `EX-DEC-043`, `EX-DEC-044`, `EX-DEC-045`, `EX-LVL-009`,
`EX-EDIT-046`, `EX-EDIT-047`, `EX-REN-049`, `EX-REN-050`, `EX-NFR-043` (nouvelles).
Amendées : `EX-REN-002` (Direct3D 11 via QRhi), `EX-BUILD-010` (l'outil de provisionnement est
lui-même épinglé).
Réutilisées : `EX-DEC-003`, `EX-DEC-032`, `EX-REN-014`, `EX-REN-015`, `EX-REN-046`, `EX-ARCH-012`,
`EX-ARCH-022`, `EX-LVL-004`, `EX-LVL-005`, `EX-EDIT-045`, `EX-NFR-004`, `EX-NFR-005`, `EX-NFR-010`,
`EX-NFR-011`, `EX-NFR-031`, `EX-NFR-040`.

**Retirées** (ancres conservées dans [`decors.md`](@ref spec-decors), section « Exigences
retirées ») : `EX-DEC-001`, `EX-DEC-002`, `EX-DEC-004`, `EX-DEC-005`, `EX-DEC-006`, `EX-DEC-010`,
`EX-DEC-020`, `EX-DEC-021`.

## Découpage

> État : ✅ fait · 🔄 en cours · ⬜ non commencé.

| Tâche | Intitulé | Emplacement | État |
|-------|----------|-------------|:----:|
| TACHE-01 | Passage à Qt 6.11 et provisionnement reproductible | `Source/HMI`, `.github/workflows`, `Documentation` | ✅ |
| TACHE-02 | Rendu sur QRhi et fin de la fenêtre native | `Source/HMI/{Graphics,Game,Interface}` | ✅ |
| TACHE-03 | Modèle de plan, format de niveau et brouillon annulable | `Source/Core/Levels` | ✅ |
| TACHE-04 | Retrait du système de décors | `Source/{Core,HMI,Test,Elements}` | ✅ |
| TACHE-05 | Composition et rendu des plans | `Source/HMI/Graphics` | ✅ |
| TACHE-06 | Parallaxe portée par le plan | `Source/HMI/{Graphics,Game}` | ✅ |
| TACHE-07 | Référence peignable et extensions du canevas | `Source/HMI/Editor` | ✅ |
| TACHE-08 | Espace « Plans », panneau et cycle de vie des fichiers | `Source/HMI/{Editor,Interface}`, `Source/Elements/UI` | ✅ |
| TACHE-09 | Budget de mémoire de texture | `Source/HMI/Graphics`, `Source/Test` | ✅ |
| TACHE-10 | Contenu, documentation et référentiel | `Source/Elements/Levels`, `Documentation` | ✅ |

### Séquencement

`TACHE-01` puis `TACHE-02` d'abord : ce sont des prérequis d'infrastructure, sans fonctionnalité
visible, mais validables **isolément** en CI avant qu'on bâtisse dessus. Puis `TACHE-03` et
`TACHE-04` ensemble — retirer les décors avant d'avoir les plans laisserait l'éditeur sans habillage.
Puis `TACHE-05`/`TACHE-06` (rendu et parallaxe), `TACHE-07`/`TACHE-08` (interface), `TACHE-09` une
fois tous les consommateurs livrés (même logique que le `LOT-62` dans le programme `0.1.0`), et
`TACHE-10` en clôture : les statuts ne se figent qu'à la fin, patron du `LOT-66`.

## Critères d'acceptation du lot

1. Un niveau sans plan déclaré se charge et se joue **exactement** comme avant le lot.
2. Un niveau portant encore `decors` se charge **sans erreur**, l'ignore, journalise un
   avertissement, et sa réécriture ne contient plus le champ.
3. Une route de plans à quatre entrées se compose dans l'ordre déclaré, quel que soit l'ordre de
   parcours interne — propriété **figée par un test**, car elle est invisible autrement.
4. À facteur `1.0`, un plan est rigoureusement immobile par rapport aux tuiles. À facteur
   quelconque, il couvre **toujours** le cadrage, y compris caméra en bord de niveau.
5. Le pixel art reste **net** après le portage QRhi : filtrage *nearest* et zoom entier vérifiés,
   pas seulement constatés.
6. Un coup de pinceau dans un plan n'apparaît **jamais** dans l'historique d'édition du niveau, ni
   réciproquement.
7. Les tests existants de `hmi::PixelOperations` et `hmi::PixelCanvasGeometry` passent **sans
   retouche** — c'est la preuve que la réutilisation de `LOT-54` est réelle et non une réécriture.
8. Un plan supplémentaire à densité native sur un niveau au plafond fait **échouer** le test de
   budget : le garde-fou est vérifié dans le sens qui compte.
9. L'écran de pause s'affiche et reçoit le focus une fois redevenu un widget enfant ordinaire —
   non-régression des deux défauts du `LOT-59`.
10. `ctest` à 100 %, lint d'exigences vert, cahier de test régénéré.

## Dépendances

Aucun lot ne dépend de celui-ci. Il retire les acquis du `LOT-49`/`LOT-50`, réutilise intégralement
l'atelier du `LOT-54`, s'appuie sur les espaces de travail du `LOT-68` et sur les modes de cadrage du
`LOT-64`.

## Risques

| Risque | Gravité | Traitement |
|---|---|---|
| Portage QRhi : pixel art devenu flou | **Élevée** | Test de non-régression visuelle ; *nearest* et zoom entier vérifiés explicitement (TACHE-02) |
| Zoom rationnel : régression de l'atelier livré | **Élevée** | Extension compatible, dénominateur 1 par défaut ; tests existants inchangés (TACHE-07) |
| Canvas Painter en *Technology Preview* | Moyenne | Confiné derrière une façade ; jamais l'autorité sur les pixels |
| Ordre des plans faussé par le rang de texture | Moyenne | `ComposedScene::sort()` trie par calque, **rang de texture**, puis `sortOrder` : à figer par un test (TACHE-05) |
| Explosion mémoire sur grands niveaux | Moyenne | Densité par plan, garde-fous au chargement (TACHE-03), budget testé (TACHE-09) |
| `aqtinstall` publié ne sait pas installer Qt ≥ 6.11 | Levé | `aqtsource` sur commit épinglé (TACHE-01) |

## Navigation des tâches

- @subpage lot-69-tache-01-qt-6-11
- @subpage lot-69-tache-02-rendu-qrhi
- @subpage lot-69-tache-03-modele-plan
- @subpage lot-69-tache-04-retrait-decors
- @subpage lot-69-tache-05-composition-plans
- @subpage lot-69-tache-06-parallaxe-plans
- @subpage lot-69-tache-07-reference-canevas
- @subpage lot-69-tache-08-espace-plans
- @subpage lot-69-tache-09-budget-memoire
- @subpage lot-69-tache-10-contenu-documentation
