# Exigences retirées {#spec-retirees}

> Deux familles d'exigences sont retirées **en entier** par le `LOT-88` : aucune ne décrit plus
> rien du jeu ni du dépôt. Leurs ancres restent ici, jamais renumérotées : les lots livrés s'y
> réfèrent, et un numéro réutilisé ferait mentir leur histoire. Les exigences retirées une à une,
> dans une famille qui vit encore, restent à la fin de leur propre spécification.
>
> Une exigence retirée n'est citée par aucun code : `scripts/lint_exigences.py` le vérifie.

## Solveur d'apprentissage automatique (`EX-IA-*`)

Un agent entraîné sans framework d'apprentissage, capable de terminer un niveau seul et de rejouer
sa solution en jeu. Le programme et son code ont été retirés au `LOT-01`.

- \anchor EX-IA-001 **EX-IA-001** *(retirée au `LOT-88`)* — calcul tensoriel et générateur
  pseudo-aléatoire maison.
- \anchor EX-IA-002 **EX-IA-002** *(retirée au `LOT-88`)* — différentiation automatique.
- \anchor EX-IA-003 **EX-IA-003** *(retirée au `LOT-88`)* — réseaux de neurones.
- \anchor EX-IA-004 **EX-IA-004** *(retirée au `LOT-88`)* — optimiseurs de descente de gradient.
- \anchor EX-IA-005 **EX-IA-005** *(retirée au `LOT-88`)* — jeu sans fenêtre pour l'entraînement.
- \anchor EX-IA-006 **EX-IA-006** *(retirée au `LOT-88`)* — observation de l'état du jeu.
- \anchor EX-IA-007 **EX-IA-007** *(retirée au `LOT-88`)* — espace d'action de l'agent.
- \anchor EX-IA-008 **EX-IA-008** *(retirée au `LOT-88`)* — séquence d'actions gagnante.
- \anchor EX-IA-009 **EX-IA-009** *(retirée au `LOT-88`)* — signal de récompense.
- \anchor EX-IA-010 **EX-IA-010** *(retirée au `LOT-88`)* — journal d'entraînement.
- \anchor EX-IA-011 **EX-IA-011** *(retirée au `LOT-88`)* — algorithme évolutionniste.
- \anchor EX-IA-012 **EX-IA-012** *(retirée au `LOT-88`)* — entraînement niveau par niveau.
- \anchor EX-IA-013 **EX-IA-013** *(retirée au `LOT-88`)* — gradient de politique.
- \anchor EX-IA-014 **EX-IA-014** *(retirée au `LOT-88`)* — réduction de variance.
- \anchor EX-IA-015 **EX-IA-015** *(retirée au `LOT-88`)* — apprentissage par valeur.
- \anchor EX-IA-016 **EX-IA-016** *(retirée au `LOT-88`)* — harnais de benchmark.
- \anchor EX-IA-017 **EX-IA-017** *(retirée au `LOT-88`)* — taux de réussite d'un modèle.
- \anchor EX-IA-018 **EX-IA-018** *(retirée au `LOT-88`)* — validation d'un fichier de rejeu.
- \anchor EX-IA-019 **EX-IA-019** *(retirée au `LOT-88`)* — rejeu en partie réelle.
- \anchor EX-IA-020 **EX-IA-020** *(retirée au `LOT-88`)* — exécutable en ligne de commande.
- \anchor EX-IA-021 **EX-IA-021** *(retirée au `LOT-88`)* — garde-fou d'intégration continue.
- \anchor EX-IA-022 **EX-IA-022** *(retirée au `LOT-88`)* — écran « Mode IA ».
- \anchor EX-IA-023 **EX-IA-023** *(retirée au `LOT-88`)* — champ de distances de la récompense de
  progression.

## Décors et plans picturaux (`EX-DEC-*`)

Des images peintes couvrant un niveau entier, avec densité, profondeur et parallaxe, peintes dans
l'éditeur ; et avant elles des décors-sprites manipulables. Le lieu d'une carte se dessine
désormais avec les pièces de sa planche (`EX-VIS-008`, `EX-REN-010`).

- \anchor EX-DEC-001 **EX-DEC-001** *(retirée au `LOT-88`)* — décor posé librement.
- \anchor EX-DEC-002 **EX-DEC-002** *(retirée au `LOT-88`)* — décors devant ou derrière le
  personnage.
- \anchor EX-DEC-003 **EX-DEC-003** *(retirée au `LOT-88`)* — plan affiché fidèle à l'asset.
- \anchor EX-DEC-004 **EX-DEC-004** *(retirée au `LOT-88`)* — décors en entités de la simulation.
- \anchor EX-DEC-005 **EX-DEC-005** *(retirée au `LOT-88`)* — collision propre à un décor.
- \anchor EX-DEC-006 **EX-DEC-006** *(retirée au `LOT-88`)* — parallaxe d'un décor.
- \anchor EX-DEC-010 **EX-DEC-010** *(retirée au `LOT-88`)* — édition des décors.
- \anchor EX-DEC-020 **EX-DEC-020** *(retirée au `LOT-88`)* — manipulation des décors en jeu.
- \anchor EX-DEC-021 **EX-DEC-021** *(retirée au `LOT-88`)* — déterminisme de cette manipulation.
- \anchor EX-DEC-030 **EX-DEC-030** *(retirée au `LOT-88`)* — conversion d'une photo en pixel art.
- \anchor EX-DEC-031 **EX-DEC-031** *(retirée au `LOT-88`)* — paramètres de cette conversion.
- \anchor EX-DEC-032 **EX-DEC-032** *(retirée au `LOT-88`)* — enregistrement de l'image convertie.
- \anchor EX-DEC-040 **EX-DEC-040** *(retirée au `LOT-88`)* — plan pictural couvrant le niveau.
- \anchor EX-DEC-041 **EX-DEC-041** *(retirée au `LOT-88`)* — densité d'un plan.
- \anchor EX-DEC-042 **EX-DEC-042** *(retirée au `LOT-88`)* — profondeur d'un plan.
- \anchor EX-DEC-043 **EX-DEC-043** *(retirée au `LOT-88`)* — parallaxe d'un plan.
- \anchor EX-DEC-044 **EX-DEC-044** *(retirée au `LOT-88`)* — coût borné des plans d'un niveau.
- \anchor EX-DEC-045 **EX-DEC-045** *(retirée au `LOT-88`)* — peinture des plans dans l'éditeur.
