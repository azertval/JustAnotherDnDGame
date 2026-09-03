# TACHE-07 — Intégration main (boucle pilotée par l'écran) {#lot-06-tache-07-integration-main}

**Lot :** [LOT-06](epic.md) · **Emplacement :** `Source/HMI/main.cpp` · **Statut :** à faire

## Contexte
Tâche d'assemblage. `main` construit aujourd'hui la scène et la rend directement. On remplace
cela par une boucle pilotée par le **gestionnaire d'écrans**, démarrant sur le **menu**.

## Travail à réaliser
- Initialiser les ressources partagées (device, `SpriteBatch`, atlas, `BitmapFont`,
  **catalogue de traduction** chargé sur la langue par défaut, TACHE-03) et le gestionnaire
  d'écrans avec le `MenuScreen` comme écran de départ.
- Boucle : **échantillonner les entrées une fois par frame** (nouvelle frame + pompe de
  messages), puis `update(input, fixedDelta)` de l'écran courant, puis `clear` / `render` /
  `present`. Appliquer les transitions ; **quitter** proprement sur demande (« Quitter » ou croix).
- Répercuter le redimensionnement (swap chain + caméra des écrans concernés).
- Retirer de `main` la scène de démo (déplacée dans `GameScreen`, TACHE-06).

## Fichiers impactés
- `Source/HMI/main.cpp`.

## Vérifications (obligatoires)
- Au lancement, l'exécutable affiche le **menu** (et non la scène directement).
- Chaque option agit : « Quitter » ferme ; « Charger niveau » et « Mode Edition » ouvrent leur
  écran ; Échap revient au menu.
- Entrées lues **une fois par frame**, avant la logique ; fermeture propre ; pas de fuite.
- Vérification **visuelle** de bout en bout (lancer l'exécutable), pas seulement la compilation.

## Points d'attention
- Ordre par frame : nouvelle frame d'entrées → messages → update écran → rendu.
- La demande de fermeture d'un écran (« Quitter ») doit atteindre la condition d'arrêt de la boucle.
- Gestion des exceptions inchangée (frontière de démarrage).

## Définition de fait (DoD)
- L'exécutable démarre sur le menu ; les trois options et Échap fonctionnent ; tous les
  critères d'acceptation de l'[epic](epic.md) satisfaits ; build `/W4 /WX`, CI verte,
  Doxygen et `CHANGELOG.md` à jour.

## Exigences
`EX-REN-030`, `EX-CTRL-021`, `EX-REN-020`, `EX-REN-021`.
