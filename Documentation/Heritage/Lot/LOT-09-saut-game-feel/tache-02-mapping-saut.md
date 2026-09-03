# TACHE-02 — Mapping du saut (Espace/W → intention) {#lot-09-tache-02-mapping-saut}

**Lot :** [LOT-09](epic.md) · **Emplacement :** `Source/HMI/Input` · **Statut :** à faire

## Contexte
Le saut ne doit pas dépendre d'une touche physique (`EX-CTRL-010`) : `toPlayerInput` (LOT-08)
traduit déjà le déplacement, on lui ajoute le **saut**. Il faut les **deux fronts** — pressé et
maintenu — que l'`InputState` (LOT-06) sait déjà fournir (`keyPressed`, `keyDown`).

## Travail à réaliser
- Étendre l'énumération `Key` si nécessaire (l'espace existe déjà : `Space` ; `W` a été ajouté au
  LOT-08).
- Dans `toPlayerInput`, renseigner :
  - `jumpPressed` = `keyPressed(Space) || keyPressed(W)` (front montant, pour déclencher/bufferiser) ;
  - `jumpHeld` = `keyDown(Space) || keyDown(W)` (maintenu, pour la hauteur variable).
- Conserver la correspondance de déplacement existante (gauche/droite inchangée).

## Fichiers impactés
- `Source/HMI/Input/PlayerInputMapper.cpp` (et `.h` si le contrat évolue).
- Test unitaire du mapping.

## Tests (obligatoires)
- `Space` **pressée** (front) → `jumpPressed == true` ; `Space` maintenue mais pas au front →
  `jumpPressed == false`, `jumpHeld == true`.
- `W` équivalent à `Space` pour le saut.
- Aucune touche de saut → `jumpPressed == false` et `jumpHeld == false`.
- Le déplacement (`moveX`) reste correct en présence d'un saut (indépendance des axes).

## Points d'attention
- **Pressé vs maintenu** : `keyPressed` = front d'une frame (déclenche une fois) ; `keyDown` =
  maintenu (continu). Ne pas confondre — c'est ce qui distingue « sauter » de « garder l'appui ».
- **Pur et testable** : le mapper lit un `InputState` injecté, aucune fenêtre (`EX-NFR-010`).
- La couche `Core` ne verra que `PlayerInput` : un futur remappage/manette (`EX-CTRL-012`) ne
  touchera que cette fonction.

## Définition de fait (DoD)
- Saut mappé (pressé + maintenu) et **testé** (`ctest` vert) ; build `/W4 /WX`.

## Exigences
`EX-CTRL-010`, `EX-CTRL-011`, `EX-NFR-010`.
