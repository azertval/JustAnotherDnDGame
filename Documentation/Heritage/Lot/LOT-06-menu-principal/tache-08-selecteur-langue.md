# TACHE-08 — Sélecteur de langue {#lot-06-tache-08-selecteur-langue}

**Lot :** [LOT-06](epic.md) · **Emplacement :** `Source/HMI/Interface`, `Source/HMI/Graphics`, `Source/Elements/Localization` · **Statut :** fait

## Contexte
Le catalogue de traduction (TACHE-03) sait charger n'importe quelle langue, mais l'application
démarre en français sans moyen d'en changer. Cette tâche ajoute un **bouton de langue** en bas
à droite du menu, matérialisé par le **drapeau de la langue courante**, qui bascule entre
**français** et **anglais** (`EX-REN-033`). Elle introduit le catalogue anglais et des icônes
de drapeaux.

## Travail à réaliser
- **Catalogue anglais** `en.lang` (mêmes clés que `fr.lang`) dans `Source/Elements/Localization`.
- **`FlagIcons`** (`HMI/Graphics`) : texture d'icônes **générée en code** contenant le drapeau
  français (trois bandes) et un drapeau du Royaume-Uni approché (croix + sautoir) pour
  l'anglais ; expose la région par langue (`EX-REN-011`).
- **`LanguageSelector`** (`HMI/Interface`) : logique **pure** du bouton — rectangle ancré en
  bas à droite, détection du **clic** gauche, bascule vers l'autre langue. Testable hors GPU
  (`EX-NFR-010`).
- **`MenuScreen`** : détient une référence **mutable** au catalogue ; à un clic sur le bouton,
  recharge la langue (`loadLanguage`), ce qui met à jour les libellés dès la frame suivante ;
  dessine l'icône du drapeau de la langue active (`RenderContext` porte les `FlagIcons`).

## Fichiers impactés
- `Source/Elements/Localization/en.lang` (nouveau).
- `Source/HMI/Graphics/FlagIcons.h`, `FlagIcons.cpp` (nouveau).
- `Source/HMI/Interface/LanguageSelector.h`, `LanguageSelector.cpp` (nouveau).
- `Source/HMI/Interface/RenderContext.h`, `MenuScreen.h`, `MenuScreen.cpp`, `HMI/main.cpp`.
- `Source/HMI/CMakeLists.txt`, `Source/Test/CMakeLists.txt`.

## Tests (obligatoires)
- Le rectangle du bouton est ancré au coin **bas-droit** (marge comprise).
- La bascule renvoie l'**autre** langue (fr ↔ en), avec un défaut robuste.
- Un **clic dans** le bouton demande la bascule ; un clic **hors** du bouton, ou l'absence de
  clic, ne bascule pas.

## Points d'attention
- **Séparer** la logique (`LanguageSelector`, testable) du dessin (icône via `SpriteBatch`).
- Le rectangle dépend de la taille de la surface : `MenuScreen` mémorise les dimensions du
  dernier rendu pour le test de clic (décalage d'au plus une frame, sans impact perceptible).
- Bascule **récupérable** : si le fichier de la langue cible est absent, la langue courante est
  conservée (`EX-NFR-040`).
- Le bouton est à l'opposé des options : un clic dessus ne déclenche aucune action de menu.

## Définition de fait (DoD)
- Le menu affiche le drapeau de la langue courante ; un clic bascule fr ↔ en et met à jour
  tous les libellés ; logique testée (`ctest` vert) ; build `/W4 /WX`, documenté.
- Vérifié visuellement (menu français ↔ anglais, drapeaux France / Royaume-Uni).

## Exigences
`EX-REN-033`, `EX-REN-030`, `EX-REN-011`, `EX-NFR-010`, `EX-NFR-040`.
