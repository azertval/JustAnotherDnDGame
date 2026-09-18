# Source/Test/Systeme/

Tests **système** (bout en bout) : un parcours d'utilisateur complet, sur les **données livrées**,
**sans la couche GPU** (fenêtre, rendu — vérifiée visuellement, cf. conventions).

- `test_parcours_edition_rpg.cpp` — le parcours d'auteur du `LOT-11` : produire une carte du RPG
  sans écrire de JSON (trois couches, un PNJ, un coffre, un portail, une rencontre), l'enregistrer,
  la recharger et la peupler comme le fait l'essai immédiat de l'éditeur.

Distinction : `Unit/` teste une brique isolée ; `Integration/` teste quelques briques assemblées ;
`Systeme/` rejoue un **scénario complet** sur le contenu livré.
