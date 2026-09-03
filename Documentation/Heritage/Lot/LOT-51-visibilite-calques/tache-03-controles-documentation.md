# TACHE-03 — Contrôles d'interface et documentation {#lot-51-tache-03-controles-documentation}

**Lot :** [LOT-51](epic.md) · **Emplacement :** `Source/HMI/Editor`, `Documentation` · **Statut :** fait

## Contexte
Deux commandes du projet agissent désormais sur le rendu, et elles sont proches sans être
équivalentes : `F8` **compose** le rendu tel que le joueur le verra ; les visibilités par calque
**décomposent** pour auditer. Les confondre conduirait à croire qu'un calque masqué l'est aussi en
jeu.

L'interface et la documentation doivent lever cette ambiguïté sans qu'on ait à l'expliquer.

## Travail à réaliser
- **Contrôles de visibilité** dans l'interface d'édition : une case à cocher par calque, groupées et
  ordonnées **dans l'ordre de dessin** — c'est ce qui rend la pile de calques compréhensible d'un
  coup d'œil.
- **Libellés sans ambiguïté** : distinguer explicitement « Aperçu » (ces contrôles, édition
  seulement) de « Jeu » (`F8`, ce que le joueur voit). Un texte d'aide ou une infobulle rappelant que
  masquer un calque n'affecte pas le jeu.
- **Action « tout afficher »** : indispensable, sinon on reste bloqué dans un état d'audit sans
  savoir comment en sortir.
- **Traduction** de toutes les chaînes, clés ajoutées aux deux catalogues.
- **Documentation** :
  - `Documentation/Guide/guide-editeur.md` — section sur le mode de définition des textures, la
    différence avec `F8`, et l'usage typique (« quels types n'ont pas de skin ? »).
  - `Documentation/Guide/guide-rendu.md` — mise à jour de la section sur les calques.
  - `Documentation/Manuel/partager-un-niveau.md` — mention côté level designer.

## Fichiers impactés
- `Source/HMI/Editor/TexturePanel.{h,cpp}` (nouvel onglet « Calques »),
  `Source/Elements/UI/TexturePanel.ui`.
- `Source/HMI/Interface/MainWindow.cpp` (câblage panneau ↔ `GameViewport`).
- `Source/Elements/Localization/fr.lang`, `en.lang`.
- `Documentation/Guide/guide-editeur.md`, `guide-rendu.md`,
  `Documentation/Manuel/partager-un-niveau.md`.

## Tests (obligatoires)
- Chaque clé de traduction utilisée existe dans les deux catalogues.
- L'action « tout afficher » restaure bien l'ensemble des calques.
- Vérification manuelle des combinaisons principales : chaque calque isolé, puis tout visible.

## Points d'attention
- **Ordre de dessin dans l'interface** : lister les calques dans le désordre annulerait l'essentiel
  du bénéfice pédagogique du contrôle.
- Ne pas placer ces contrôles là où ils pourraient être pris pour des réglages de jeu (menu Options,
  par exemple) : ils appartiennent à l'éditeur.
- Le manuel s'adresse à un non-développeur : décrire l'usage (« vérifier ce qui est configuré sur
  chaque plan »), pas l'implémentation.

## Définition de fait (DoD)
- Les contrôles existent, sont ordonnés comme la pile de calques, entièrement traduits, et
  distinguent sans ambiguïté l'aperçu du rendu de jeu ; une action restaure tout ; guide et manuel
  sont à jour ; Doxygen et lint verts.

## Exigences
`EX-EDIT-044` (visibilité par calque) ; réutilise `EX-REN-046` (bascule, distincte), `EX-REN-033`
(traduction), `EX-IHM-010` (fenêtre à panneaux).
