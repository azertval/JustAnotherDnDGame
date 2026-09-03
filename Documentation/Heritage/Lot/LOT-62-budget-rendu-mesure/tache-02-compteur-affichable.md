# TACHE-02 — Compteur de cadence et de primitives affichable {#lot-62-tache-02-compteur-affichable}

**Lot :** [LOT-62](epic.md) · **Emplacement :** `Source/HMI/Game`, `Source/HMI/Graphics` ·
**Statut :** fait

## Contexte
`EX-NFR-001` demande 60 images par seconde. Personne ne sait si elle est tenue : rien ne mesure la
cadence, rien ne l'affiche. Le seul indice disponible est une trace de `hmi::SpriteRenderer`, émise
uniquement quand les statistiques changent, et illisible en cours de partie.

Or `LOT-52` a livré exactement le mécanisme qu'il faut : une police bitmap, un rendu de texte dans
la scène, et un affichage tête haute (`hmi::GameHud`) qui compose déjà des lignes à partir de l'état
du jeu. Ajouter un compteur, c'est ajouter des lignes à un affichage existant.

## Travail à réaliser
- **Mesure de la cadence** : moyenne glissante sur une fenêtre courte (constante nommée), calculée
  dans `hmi::GameViewport` à partir de l'horloge de rendu déjà présente (`_previousFrame`).
- **Lignes de diagnostic** : images par seconde, primitives composées, primitives soumises, lots de
  dessin, et nombre de pas de simulation consommés à la dernière image — cette dernière révèle
  immédiatement une boucle qui rattrape.
- **Activation explicite** : touche dédiée non remappable (comme `F8` pour la bascule de rendu,
  `LOT-41`) ou entrée d'options, désactivée par défaut.
- **Composition par une fonction pure**, sur le patron de `hmi::composeHudLines` (`LOT-52`) : à
  partir de valeurs mesurées, produire les lignes à afficher. Le calcul est testable, le dessin
  non.
- **Aucun effet sur la simulation** (`EX-ARCH-012`) : la mesure lit, elle ne modifie rien, et les
  lignes s'affichent sur le calque d'interface, hors du monde.
- **Coût négligeable quand l'affichage est éteint** : ne rien calculer plutôt que calculer sans
  afficher.

## Fichiers impactés
- `Source/HMI/Game/DiagnosticsHud.{h,cpp}` (nouveau) — composition pure des lignes.
- `Source/HMI/Game/GameViewport.{h,cpp}` — mesure de cadence, bascule.
- `Source/HMI/Input/GameKeyBindings.cpp` — touche dédiée, si le choix se porte sur une touche.
- `Source/Elements/Localization/{fr,en}.lang` — libellés éventuels.
- `Source/Test/Unit/HMI/Game/test_diagnostics_hud.cpp` (nouveau), `Source/Test/CMakeLists.txt`.

## Tests (obligatoires)
- La composition des lignes est **pure** et testée : valeurs données → lignes attendues, y compris
  aux cas limites (aucune image mesurée, division par zéro évitée).
- La moyenne glissante est correcte sur une suite de durées connues, et stable quand la cadence
  l'est.
- Affichage **éteint** → aucune ligne composée, aucun coût.
- Aucune primitive de diagnostic n'est émise dans le monde : les lignes vivent sur le calque
  d'interface — asserté via le *QuadRecorder*, comme le fait déjà `LOT-52`.
- Tests sans GPU.

## Points d'attention
- **Ne pas afficher une cadence instantanée** : `1 / dt` sur une image saute dans tous les sens et
  n'informe sur rien. Une moyenne glissante sur une fenêtre courte est la seule forme lisible.
- La cadence de **rendu** et le nombre de **pas de simulation** sont deux choses distinctes depuis
  le `LOT-33` : les afficher séparément, sous peine de rendre le diagnostic trompeur exactement
  quand il servirait.
- Ne pas ouvrir un second chemin de texte dans la scène : `hmi::TextRenderer` et la police bitmap du
  `LOT-52` sont là pour ça.
- Si le choix se porte sur une touche, vérifier qu'elle n'entre pas en conflit avec les raccourcis
  d'éditeur remappables (`LOT-29`) — `F8` a été rendue non remappable pour cette raison.
- L'affichage ne doit pas masquer le HUD de jeu (budgets de sauts et de dashs) : choisir un coin
  libre.

## Définition de fait (DoD)
- Un affichage de diagnostic activable montre cadence, primitives, lots et pas de simulation, avec
  une composition pure et testée, sans effet sur la simulation, sans coût quand il est éteint, et
  sans second chemin de rendu de texte ; `/W4 /WX` propre.

## Exigences
Réutilise `EX-NFR-001` (60 images par seconde — rendue observable ici), `EX-NFR-005` (primitives
observables), `EX-IHM-003` (affichage tête haute), `EX-REN-032` (texte dans la scène),
`EX-ARCH-012` (rendu sans effet sur la simulation), `EX-NFR-004` (vérification sans GPU),
`EX-CTRL-012` (touches).
