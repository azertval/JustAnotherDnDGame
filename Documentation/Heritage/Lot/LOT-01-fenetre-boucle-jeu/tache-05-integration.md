# TACHE-05 — Intégration main & vérification {#lot-01-tache-05-integration}

**Lot :** [LOT-01](epic.md) · **Module :** `Source/HMI` · **Statut :** terminé

## Contexte
Dernière tâche du lot : assembler fenêtre, device et boucle de temps fixe dans le point d'entrée, et vérifier le comportement d'ensemble.

## Travail à réaliser
- Réécrire `Source/HMI/main.cpp` pour :
  1. Créer la `Window` (TACHE-01) puis le `GraphicsDevice` (TACHE-02).
  2. Boucle principale : mesurer le temps réel écoulé, alimenter `FixedTimestep` (TACHE-03), exécuter les pas de mise à jour (vide pour l'instant), puis **effacer + présenter** (TACHE-04).
  3. Router l'événement de **redimensionnement** vers le device.
  4. Quitter proprement sur demande de fermeture (croix) ou touche **Échap**.
- Encadrer le démarrage par une gestion d'exception à la frontière (échec d'init → message clair, sortie contrôlée).

## Fichiers impactés
- `Source/HMI/main.cpp`.
- `CHANGELOG.md` (section *Non publié*).

## Vérification (manuelle + automatique)
- **Manuelle** : la fenêtre s'ouvre, affiche la couleur, se redimensionne, se ferme (croix et Échap) ; le processus se termine avec le code 0.
- **Sanitizer** : exécution ponctuelle avec `-DENABLE_ASAN=ON` sans rapport d'erreur.
- **Automatique** : `ctest` vert (tests de `FixedTimestep`), build `/W4 /WX` sans avertissement, **CI verte**.

## Définition de fait (DoD)
- Tous les critères d'acceptation de l'[epic](epic.md) sont satisfaits.
- `CHANGELOG.md` mis à jour ; documentation Doxygen générable sans erreur.

## Exigences
`EX-REN-020`, `EX-REN-021`, `EX-NFR-013`, `EX-NFR-022`.
