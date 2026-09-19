# T01 — Carte de test des textures dans le jeu

> À réaliser après la référence de taille validée de [T00](tache-00-socle-pilote-pnj.md), avant acceptation de toute texture régionale. Travail planifié, aucun test exécuté ici.
> Contrat obligatoire pour les reprises [CE-R01](tache-ce-r01-reprise-trois-sites.md) et toutes les séries futures contenant textures, tuiles ou pièces de décor.

- [ ] Créer une carte de test chargeable et parcourable dans le jeu, avec secteurs par jeu de matériaux, et un personnage témoin présent dans chaque secteur. Choisir un humanoïde validé du pilote ; figer son fichier, empreinte, pose, hauteur de garde de 45 pixels d’art et ancrage au sol. Ne pas utiliser l’ancien Ironhand comme étalon.
- [ ] Conserver les paramètres réels de projection, caméra et échelle. Le rendu actuel applique `FIGURE_SCALE = 1.25` à la figurine : 45 pixels d’art ne signifie pas 45 pixels écran. Consigner ces paramètres dans chaque rapport ; ne pas redimensionner le témoin pour faire passer un décor trop grand ou trop petit.
- [ ] Inventorier toutes les clés de texture, dimensions, emprises, ancres, orientations et affectations `appearance.json`. Partir du profil actuel de tuile 68 × 42 et palette 64 couleurs ; déclarer les dimensions propres aux pièces hautes ou étendues. Figer, avant génération, les rapports de taille attendus pour portes, marches, étals, murs et mobilier face au témoin.
- [ ] Construire une matrice de raccords autorisés : répétition de chaque sol sur au moins 3 × 3 cases, variantes voisines, transitions de matériaux, bords, angles, jonctions de murs, ouvertures, escaliers et pièces à emprise multiple réellement prises en charge. Marquer les raccords interdits et les transitions manquantes ; aucune clé utilisée ni adjacency requise sans cas de test.
- [ ] Parcourir les passages avec le témoin : devant/derrière les murs et objets, près des portes, angles et marches. Vérifier ligne de sol, profondeur, occultation, collisions et absence de flottement. Tester les orientations disponibles, le zoom nominal et les autres zooms supportés ; examiner également les pixels source à agrandissement entier.
- [ ] Produire des captures du rendu réel après chargement complet des textures, une vue générale, des détails de chaque jonction et une séquence du parcours. Ouvrir les images et les frames de la séquence pour une revue visuelle consignée, avec comparaison côte à côte ancien/nouveau et témoin identique.

## Verdict bloquant

Couverture exigée : 100 % des textures utilisées et des raccords requis ; zéro ressource manquante, zéro ancre/emprise différente du contrat, zéro trou ou recouvrement parasite dans les masques de couverture attendue. Distinguer les marges transparentes légitimes, l’occultation volontaire et les défauts. Vérifier PNG RGBA, alpha propre, palette et dimensions exactes. Mesurer les écarts de jonction en pixels et les rapports de taille au témoin ; figer leurs tolérances dans le profil avant de noter la production, sans les élargir pour accepter un échec. Les raccords de couleurs, motifs, perspective et éclairage demandent aussi une revue visuelle : une distance de couleur seule ne prouve pas la cohérence.

Une capture manquante, un test graphique sauté faute de backend, une couverture incomplète ou une revue indécise donne **à revoir**, jamais validé. Refaire les éléments fautifs puis rejouer leurs jonctions et le parcours complet affecté. Aucun changement de référence visuelle sans justification et nouvelle revue.

## Intégration technique prévue

Les tests actuels `Source/Test/Integration/test_exploration_carte_livree.cpp` vérifient chargement/composition du Colisée et marche à Martpart ; ils ne prouvent pas l’imbrication visuelle. Le test `Source/Test/Unit/HMI/Graphics/test_rhi_offscreen.cpp` protège un motif de rendu et peut être sauté sans backend : l’étendre ou le compléter ne dispense pas de la carte jouée.

- [ ] Ajouter les fixtures et tests dans la hiérarchie `Source/Test/`, les outils de préparation/capture/rapport dans `scripts/` ou `Tools/`, sans code dans ce dossier de lot. Prévoir le lancement explicite de la carte de test sans la rendre accessible dans la progression normale.
- [ ] Charger les candidats dans un espace de validation isolé avec le vrai renderer, avant promotion des assets livrés. Le statut reste généré/à revoir pendant ce test ; seul le succès de tous les contrôles autorise validé puis intégré.
- [ ] Exécuter les contrôles pertinents via `scripts/build.ps1 -Label integration` et les tests graphiques concernés, puis vérifier aussi les trois cartes livrées. Joindre backend, commit, empreintes, mesures, captures et verdicts au rapport de chaque PR. CI verte et preuve locale du rendu sont toutes deux nécessaires.

Les références du corpus restent hors Git. Les fixtures originales du projet, mesures et preuves de rendu du jeu peuvent être versionnées selon les conventions du dépôt.
