# TACHE-03 — Entrée/sortie, liaison de mécanismes, redimensionnement {#lot-14-tache-03-entree-sortie-mecanismes-redimension}

**Lot :** [LOT-14](epic.md) · **Emplacement :** `HMI/Editor` · **Statut :** à faire

## Contexte
Au-delà de la peinture simple de tuiles (TACHE-02), trois interactions ont une **sémantique
propre** : l'entrée et la sortie sont **uniques** par niveau, un interrupteur doit être **relié** à
une porte pour avoir un sens, et la grille peut devoir changer de **taille** en cours de conception.

## Travail à réaliser
- **Entrée/sortie** : poser `Entry`/`Exit` depuis la palette appelle
  `LevelDraft::setEntry`/`setExit` (TACHE-01), qui **déplace** l'occurrence existante plutôt que
  d'en créer une seconde — retour visuel immédiat (l'ancienne case redevient `Empty`).
- **Liaison interrupteur↔porte** : un mode « liaison » de la palette — cliquer un `Switch` puis une
  `Door` (ou l'inverse) les **lie** (`LevelDraft::linkMechanism`) ; une **ligne indicative** est
  dessinée entre les deux tant qu'elles restent visibles à l'écran (retour visuel simple, pas
  d'animation). Cliquer une liaison existante (ou un raccourci dédié) la **retire**
  (`unlinkMechanism`). Poser une `Door`/`Switch` sans liaison est autorisé (état intermédiaire
  pendant l'édition), mais bloqué par la validation à l'enregistrement (TACHE-05, `EX-LVL-004`).
- **Redimensionnement** : un contrôle d'interface (ex. champs largeur/hauteur + bouton « appliquer »
  dans la palette) appelle `LevelDraft::resize` ; agrandir ajoute des cases `Empty`, réduire tronque
  (avec confirmation si des tuiles significatives — entrée, sortie, mécanismes — seraient perdues).

## Fichiers impactés
- `Source/HMI/Interface/EditorScreen.h`/`.cpp`, `Source/HMI/Editor/` (extension de la palette/du
  contrôleur d'interaction de TACHE-02).
- Tests unitaires et d'intégration (`Source/Test/Unit/HMI/Editor/`,
  `Source/Test/Integration/`).

## Tests (obligatoires)
- Poser une deuxième `Entry` déplace la première (une seule au final) ; idem `Exit`.
- Séquence clic `Switch` puis clic `Door` produit une liaison dans le `LevelDraft` ; répéter sur une
  liaison existante la retire.
- Redimensionner plus petit tronque les cases hors bornes ; redimensionner plus grand préserve le
  contenu existant et complète en `Empty` ; une réduction qui **perdrait** l'entrée/la sortie/une
  liaison est signalée avant application (test du chemin de confirmation/refus).

## Points d'attention
- Toute la sémantique (unicité, liaison, redimensionnement) est déjà portée par `LevelDraft`
  (TACHE-01) : cette tâche **orchestre** l'interaction souris/clavier vers cette API, sans
  réimplémenter de règles côté `HMI`.
- Rester cohérent avec la représentation déjà utilisée pour l'état des portes en jeu (LOT-12,
  teinte ouverte/fermée) pour ne pas introduire un second vocabulaire visuel.

## Définition de fait (DoD)
- Entrée/sortie, liaison et redimensionnement opérationnels et testés (`ctest` vert), vérifiés
  visuellement ; build `/W4 /WX` ; Doxygen à jour.

## Exigences
`EX-EDIT-003`, `EX-EDIT-004`, `EX-EDIT-005`, `EX-LVL-004`.
