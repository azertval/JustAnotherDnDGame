# Premier essai PNJ — 19 septembre 2026

**Verdict : industrialisation non démontrée ; zéro PNJ autorisé à intégrer.**
Le socle de préparation/réception fonctionne, mais T00 reste ouverte. Les seuils ne sont
pas abaissés pour accepter les images produites. Aucun candidat ne remplace les assets du jeu.

## Production observée

39 appels réels au générateur intégré : 6 portraits (dont la reprise de Xorius),
5 planches complètes et 28 reprises d’animation. Les 39 réponses sont reçues, aucune requête
ne reste incertaine. Après extraction, 5 portraits et 28 bandes candidates sont disponibles.
Chaque tour conserve son prompt A+B+C, ses références projet figées hors Git, sa réponse originale,
ses empreintes et ses verdicts de traitement. Les retraitements ne sont pas comptés comme
de nouvelles générations. Coût et consommation de quota : inconnus, pas zéro inventé.

| PNJ | Tours | Constat bloquant |
|---|---:|---|
| Anariel | 14 | Attaque hors gabarit ; la dernière incantation dessine deux lames malgré la consigne. Les gardes des passes séparées changent de silhouette. |
| Jade | 4 | Les six bandes passent les mesures provisoires ; proportions de la réaction au coup et rendu sombre à revoir contre la garde. |
| Lizz | 7 | Les six bandes passent les mesures provisoires ; corps plus petit au sort et à l’attaque, échelle faussée par les bras levés. |
| Nakral | 8 | Réaction au coup hors ancrage ; attaque et sort touchent le bord utile. Épée et proportions varient entre passes. |
| Xorius | 6 | Marche encore discontinue et mouvement des jambes insuffisant ; l’arc dépasse la tête et fausse la mesure du corps. Palette trop sombre après réduction. |

Les cinq comparaisons ancien/nouveau ont été ouvertes et relues. Les GIF sont produits pour
les bandes présentes, avec les durées du moteur ; ils ne constituent pas une validation en jeu.
Les détails chiffrés et les journaux sont liés dans [le rapport](rapport-poc.md).

## Ce que l’essai établit

- La reprise d’une animation seule, sans duplication ni emprunt à une autre bande, est praticable.
- Les modèles de poses de la même animation aident le compte et l’espacement, sans garantir
  la locomotion, l’arme unique ou les proportions.
- Un rejet récent retire l’ancien candidat concerné. Une modification du brief, du traitement
  ou de la calibration invalide la revue. Une image illisible reste un échec journalisé.
- Les témoins manuels ont eux aussi des défauts mesurés (palette, hauteur, boucle) : ils servent
  à comparer, pas à justifier un relâchement des critères.

## Corrections nécessaires avant nouvelle campagne

1. Remplacer la hauteur de boîte englobante par des repères corporels explicites, vérifiés
   sur l’image : sommet du crâne, sol et pivot, effets et armes exclus. Garder une échelle
   unique par tour ; ne pas faire rentrer un débordement en réduisant le personnage.
2. Définir une palette de personnage couvrant aussi l’équipement et les effets. La palette
   du seul portrait perd ici des couleurs indispensables ; contrôler les couleurs exactes,
   pas uniquement leur nombre.
3. Calibrer et tester les dérives de position/corps et le raccord de boucle sur des défauts
   connus. Huit silhouettes distinctes ne prouvent pas huit phases correctes de marche.
4. Enrichir les fiches structurées : le pilote reprend encore l’apparence détaillée du bloc B
   de LOT-91 et les localisateurs vérifiés du Compendium ; l’extraction autonome du corpus
   vers les champs de fiche n’est pas implémentée.
5. Refaire les animations rejetées, relire les preuves, puis seulement tester les nouveaux
   descripteurs avec le lecteur du jeu et leur rendu dans la galerie. Finaliser ensuite
   la récupération d’installation après arrêt brutal et l’état « intégré » attesté par ces tests.

## Périmètre et provenance

Cet essai utilise exclusivement les ancres, portraits et bandes créés pour le projet.
Aucune illustration du corpus n’a été envoyée ou copiée dans l’historique versionné.
Les textes ont été confrontés aux pages PDF 10, 68, 83, 92 et 148 du Character Compendium.
Lizz sans arme est une adaptation du projet, pas une affirmation sur l’équipement du livre.

Le chat appelle le générateur ; la ligne de commande prépare, reçoit et contrôle les résultats.
Il n’existe pas ici de commande autonome capable d’invoquer le générateur du chat.
Les tests du jeu portant sur les assets déjà livrés ne valident pas les nouveaux candidats.
Les régions, textures et Ironhand restent dans les tâches suivantes, hors roadmap.

## Vérifications exécutées

- Python 3.13 : 105 tests des scripts réussis, dont 14 tests de la fabrique.
- `scripts/build.ps1 -Test` : construction réussie, aucun échec sur 769 cas ; un test de
  capture d’arène explicitement sauté par la suite existante. Les assets testés restent ceux livrés.
- Contrôles du dépôt : 18 sur 19 verts localement ; PSScriptAnalyzer 1.25.0 absent du poste.
  Aucun script PowerShell n’est modifié. Ce manque d’outil n’est pas présenté comme un succès.
- Ruff : scripts de la fabrique conformes. CI distante à constater sur la PR.
