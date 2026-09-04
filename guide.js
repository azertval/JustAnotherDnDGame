var guide =
[
    [ "Comment lire ce guide", "guide.html#autotoc_md245", null ],
    [ "Architecture en deux couches", "guide.html#autotoc_md246", null ],
    [ "Plan du guide", "guide.html#autotoc_md247", null ],
    [ "Boucle de jeu et pas de temps fixe", "guide-boucle.html", [
      [ "Qu'est-ce qu'une boucle de jeu ?", "guide-boucle.html#autotoc_md48", null ],
      [ "Le piège du framerate variable", "guide-boucle.html#autotoc_md49", null ],
      [ "Le principe du pas de temps fixe", "guide-boucle.html#autotoc_md50", null ],
      [ "L'accumulateur : <a class=\"el\" href=\"classcore_1_1FixedTimestep.html\" title=\"core::FixedTimestep\">core::FixedTimestep</a>", "guide-boucle.html#autotoc_md51", [
        [ "Exemple chiffré", "guide-boucle.html#autotoc_md52", null ],
        [ "La « spirale de la mort »", "guide-boucle.html#autotoc_md53", null ],
        [ "<a class=\"el\" href=\"classcore_1_1FixedTimestep.html#ab7f7f880eef11df91ab7bc647e19bf56\" title=\"core::FixedTimestep::interpolationAlpha\">interpolationAlpha</a>", "guide-boucle.html#autotoc_md54", null ],
        [ "Les frames sans pas de simulation et les entrées", "guide-boucle.html#autotoc_md55", null ]
      ] ],
      [ "Conséquence pratique pour tout le code de simulation", "guide-boucle.html#autotoc_md56", null ],
      [ "Voir aussi", "guide-boucle.html#autotoc_md57", null ]
    ] ],
    [ "ECS : entités, composants, systèmes", "guide-ecs.html", [
      [ "Le problème que l'ECS résout", "guide-ecs.html#autotoc_md87", null ],
      [ "L'entité : <a class=\"el\" href=\"structcore_1_1Entity.html\" title=\"core::Entity\">core::Entity</a>", "guide-ecs.html#autotoc_md88", null ],
      [ "Le <a class=\"el\" href=\"classcore_1_1World.html\" title=\"core::World\">World</a>", "guide-ecs.html#autotoc_md89", null ],
      [ "Le stockage : sparse set (core::ComponentPool&lt;T&gt;)", "guide-ecs.html#autotoc_md90", [
        [ "Ajout et suppression : <em>swap-and-pop</em>", "guide-ecs.html#autotoc_md91", null ],
        [ "Exemple pas à pas", "guide-ecs.html#autotoc_md92", null ]
      ] ],
      [ "Les vues : core::View&lt;Components...&gt;", "guide-ecs.html#autotoc_md93", null ],
      [ "Les systèmes et l'ordre d'exécution", "guide-ecs.html#autotoc_md94", null ],
      [ "Voir aussi", "guide-ecs.html#autotoc_md95", null ]
    ] ],
    [ "Mathématiques du moteur", "guide-maths.html", [
      [ "<a class=\"el\" href=\"structcore_1_1Vector2.html\" title=\"core::Vector2\">Vector2</a> : un point ou une direction dans le monde", "guide-maths.html#autotoc_md154", [
        [ "<a class=\"el\" href=\"structcore_1_1Vector2.html#a5f3517fe219407f73a3ed5542091c31c\" title=\"core::Vector2::lengthSquared\">lengthSquared</a> : éviter la racine carrée", "guide-maths.html#autotoc_md155", null ],
        [ "Égalité approchée", "guide-maths.html#autotoc_md156", null ]
      ] ],
      [ "<a class=\"el\" href=\"structcore_1_1Aabb.html\" title=\"core::Aabb\">Aabb</a> : la boîte englobante alignée aux axes", "guide-maths.html#autotoc_md157", null ],
      [ "Conventions d'unités et de repère", "guide-maths.html#autotoc_md158", null ],
      [ "Comparaison flottante : pourquoi l'égalité stricte est dangereuse", "guide-maths.html#autotoc_md159", null ],
      [ "Voir aussi", "guide-maths.html#autotoc_md160", null ]
    ] ],
    [ "Physique du personnage", "guide-physique.html", [
      [ "1. Collision par balayage continu (swept AABB)", "guide-physique.html#autotoc_md183", [
        [ "Le problème : le <em>tunneling</em>", "guide-physique.html#autotoc_md184", null ],
        [ "La solution : tester tout le trajet, pas seulement l'arrivée", "guide-physique.html#autotoc_md185", null ],
        [ "Méthode retenue : balayage <b>par axe</b> avec clamp direct", "guide-physique.html#autotoc_md186", null ],
        [ "Pourquoi caler directement plutôt que d'interpoler", "guide-physique.html#autotoc_md187", null ],
        [ "Lire le résultat : <a class=\"el\" href=\"structcore_1_1SweepResult.html\" title=\"core::SweepResult\">core::SweepResult</a>", "guide-physique.html#autotoc_md188", null ]
      ] ],
      [ "2. Suivi de pente et d'arrondi (EX-GP-003, EX-GP-004)", "guide-physique.html#autotoc_md189", [
        [ "Pourquoi une pente (ou un arrondi) n'est jamais solide", "guide-physique.html#autotoc_md190", null ],
        [ "<span class=\"tt\">core::slopeSurfaceHeight</span> et <span class=\"tt\">core::resolveSlopeFollow</span>", "guide-physique.html#autotoc_md191", null ],
        [ "Le piège du mur adjacent (correction du balayage horizontal)", "guide-physique.html#autotoc_md192", null ],
        [ "Particularité de l'arrondi : tangente verticale à une extrémité", "guide-physique.html#autotoc_md193", null ]
      ] ],
      [ "3. Gravité et intégration", "guide-physique.html#autotoc_md194", [
        [ "Vitesse terminale newtonienne (EX-GP-019)", "guide-physique.html#autotoc_md195", null ]
      ] ],
      [ "4. Saut et <em>game feel</em>", "guide-physique.html#autotoc_md196", null ],
      [ "5. Dash 8 directions", "guide-physique.html#autotoc_md197", null ],
      [ "6. Wall jump et wall slide", "guide-physique.html#autotoc_md198", null ],
      [ "Ordre d'un pas (résumé)", "guide-physique.html#autotoc_md199", null ],
      [ "Voir aussi", "guide-physique.html#autotoc_md200", null ]
    ] ],
    [ "Niveaux : modèle, chargement, mécanismes, budgets", "guide-niveaux.html", [
      [ "Le modèle en mémoire", "guide-niveaux.html#autotoc_md161", [
        [ "Deux systèmes de coordonnées à ne pas confondre", "guide-niveaux.html#autotoc_md162", null ],
        [ "<a class=\"el\" href=\"namespacecore.html#ace99a1be913e6294e42e9ebb145eb875\" title=\"core::TileType\">core::TileType</a> : le vocabulaire des cases", "guide-niveaux.html#autotoc_md163", null ],
        [ "<a class=\"el\" href=\"classcore_1_1TileMap.html\" title=\"core::TileMap\">core::TileMap</a> : la grille", "guide-niveaux.html#autotoc_md164", null ],
        [ "<a class=\"el\" href=\"classcore_1_1Level.html\" title=\"core::Level\">core::Level</a> : le niveau assemblé", "guide-niveaux.html#autotoc_md165", null ]
      ] ],
      [ "Chargement JSON", "guide-niveaux.html#autotoc_md166", [
        [ "Exemple concret", "guide-niveaux.html#autotoc_md167", null ],
        [ "Validation", "guide-niveaux.html#autotoc_md168", null ],
        [ "Cadrage de caméra (<span class=\"tt\">cameraFraming</span>, <span class=\"tt\">LOT-64</span>)", "guide-niveaux.html#autotoc_md169", null ]
      ] ],
      [ "De la grille aux entités : <a class=\"el\" href=\"namespacecore.html#a69ac6b2c5ca32686a14824d1f1c04b2a\" title=\"core::buildLevelScene\">buildLevelScene</a>", "guide-niveaux.html#autotoc_md170", null ],
      [ "Mécanismes déclencheur ↔ porte", "guide-niveaux.html#autotoc_md171", [
        [ "Clé et porte verrouillée (<span class=\"tt\">EX-GP-023</span>, <span class=\"tt\">LOT-63</span>)", "guide-niveaux.html#autotoc_md172", null ]
      ] ],
      [ "Blocs poussables", "guide-niveaux.html#autotoc_md173", [
        [ "Blocs à taille réduite (<span class=\"tt\">×0.5</span>/<span class=\"tt\">×0.25</span>)", "guide-niveaux.html#autotoc_md174", null ]
      ] ],
      [ "Plateformes mobiles (<span class=\"tt\">EX-GP-026</span>, <span class=\"tt\">LOT-63</span>)", "guide-niveaux.html#autotoc_md175", null ],
      [ "Budget de mouvements et capacités du tableau", "guide-niveaux.html#autotoc_md176", null ],
      [ "Dangers avancés (<span class=\"tt\">LOT-31</span>)", "guide-niveaux.html#autotoc_md177", null ],
      [ "Issue et enchaînement", "guide-niveaux.html#autotoc_md178", null ],
      [ "Séquence de niveaux (donnée de contenu, <span class=\"tt\">LOT-59</span>)", "guide-niveaux.html#autotoc_md179", null ],
      [ "Garde-fou de couverture des mécaniques (<span class=\"tt\">EX-LVL-015</span>, <span class=\"tt\">LOT-65</span>)", "guide-niveaux.html#autotoc_md180", [
        [ "De la couverture à la profondeur (second temps du <span class=\"tt\">LOT-65</span>)", "guide-niveaux.html#autotoc_md181", null ]
      ] ],
      [ "Voir aussi", "guide-niveaux.html#autotoc_md182", null ]
    ] ],
    [ "Entrées et actions logiques", "guide-entrees.html", [
      [ "Le principe : ne jamais coder « en dur » une touche dans le gameplay", "guide-entrees.html#autotoc_md119", null ],
      [ "Échantillonner plutôt que réagir : <a class=\"el\" href=\"classhmi_1_1InputState.html\" title=\"hmi::InputState\">hmi::InputState</a>", "guide-entrees.html#autotoc_md120", [
        [ "Détecter les fronts, pas seulement l'état", "guide-entrees.html#autotoc_md121", null ],
        [ "Le cycle d'une frame", "guide-entrees.html#autotoc_md122", null ],
        [ "Le pont Qt → <a class=\"el\" href=\"namespacehmi.html#a83c98e56e30eafa606048853f7962e48\" title=\"hmi::Key\">Key</a> : <a class=\"el\" href=\"namespacehmi.html#a1b53de392afb0dfdf5cdef4543fb672d\" title=\"hmi::qtKeyToHmiKey\">qtKeyToHmiKey</a>", "guide-entrees.html#autotoc_md123", null ]
      ] ],
      [ "Traduire l'état en intention : <a class=\"el\" href=\"namespacehmi.html#a2c3341857c60f0bef0a01a6cd10ce5f7\" title=\"hmi::toPlayerInput\">hmi::toPlayerInput</a>", "guide-entrees.html#autotoc_md124", null ],
      [ "La manette : une seconde source, fusionnée en lecture (EX-CTRL-002, LOT-20)", "guide-entrees.html#autotoc_md125", null ],
      [ "Le menu d'options : <a class=\"el\" href=\"classhmi_1_1OptionsPage.html\" title=\"hmi::OptionsPage\">hmi::OptionsPage</a>", "guide-entrees.html#autotoc_md126", null ],
      [ "Remapper les touches et boutons : <a class=\"el\" href=\"classhmi_1_1GameKeyBindings.html\" title=\"hmi::GameKeyBindings\">GameKeyBindings</a>/<a class=\"el\" href=\"classhmi_1_1EditorKeyBindings.html\" title=\"hmi::EditorKeyBindings\">EditorKeyBindings</a> (LOT-29), <a class=\"el\" href=\"classhmi_1_1GamepadBindings.html\" title=\"hmi::GamepadBindings\">GamepadBindings</a> (LOT-30)", "guide-entrees.html#autotoc_md127", null ],
      [ "La langue de l'interface : <a class=\"el\" href=\"classhmi_1_1Localization.html\" title=\"hmi::Localization\">hmi::Localization</a>", "guide-entrees.html#autotoc_md128", null ],
      [ "Voir aussi", "guide-entrees.html#autotoc_md129", null ]
    ] ],
    [ "Rendu 2D : de l'ECS à l'écran", "guide-rendu.html", [
      [ "Vocabulaire de base : GPU, swap chain, back buffer", "guide-rendu.html#autotoc_md201", null ],
      [ "QRhi : une couche d'accès au GPU, pas un changement de cible", "guide-rendu.html#autotoc_md202", null ],
      [ "La surface de dessin : le viewport Qt (<span class=\"tt\">hmi::GameViewport</span>)", "guide-rendu.html#autotoc_md203", null ],
      [ "Unités monde et pixels : <a class=\"el\" href=\"classhmi_1_1Camera2D.html\" title=\"hmi::Camera2D\">hmi::Camera2D</a>", "guide-rendu.html#autotoc_md204", [
        [ "Cadrer un contenu plus grand que la fenêtre : <span class=\"tt\">fitZoom</span> et <span class=\"tt\">hmi::RoomGrid</span>", "guide-rendu.html#autotoc_md205", null ],
        [ "Le cadrage choisi par le niveau : trois modes (<span class=\"tt\">LOT-64</span>)", "guide-rendu.html#autotoc_md206", null ],
        [ "Mélanger plusieurs tailles de caméra : zones dessinées à la main et taille de suivi réglable (<span class=\"tt\">EX-LVL-007</span>, <span class=\"tt\">EX-REN-017</span>)", "guide-rendu.html#autotoc_md207", null ]
      ] ],
      [ "Le pipeline de dessin de sprites : <a class=\"el\" href=\"classhmi_1_1SpriteBatch.html\" title=\"hmi::SpriteBatch\">hmi::SpriteBatch</a>", "guide-rendu.html#autotoc_md208", [
        [ "Pourquoi « batcher » plutôt que dessiner un sprite à la fois", "guide-rendu.html#autotoc_md209", null ],
        [ "<a class=\"el\" href=\"structhmi_1_1SpriteQuad.html\" title=\"hmi::SpriteQuad\">SpriteQuad</a> : un rectangle texturé", "guide-rendu.html#autotoc_md210", null ],
        [ "Sommets, shaders, et échantillonnage <em>nearest</em>", "guide-rendu.html#autotoc_md211", null ],
        [ "<a class=\"el\" href=\"structhmi_1_1LineQuad.html\" title=\"hmi::LineQuad\">LineQuad</a> : un segment orienté (liens de mécanismes, <span class=\"tt\">LOT-37</span>)", "guide-rendu.html#autotoc_md212", null ]
      ] ],
      [ "<a class=\"el\" href=\"classhmi_1_1TextureAtlas.html\" title=\"hmi::TextureAtlas\">hmi::TextureAtlas</a> : un spritesheet, chargé depuis un fichier", "guide-rendu.html#autotoc_md213", [
        [ "Le pipeline de textures depuis fichiers, et son repli procédural", "guide-rendu.html#autotoc_md214", null ],
        [ "<span class=\"tt\">Source/Elements/Assets/</span> : convention et régénération", "guide-rendu.html#autotoc_md215", null ],
        [ "Les images du personnage : pourquoi elles vivent dans le même atlas", "guide-rendu.html#autotoc_md216", null ],
        [ "<span class=\"tt\">LOT-48</span> : spritesheet externe et découplage image/hitbox", "guide-rendu.html#autotoc_md217", null ],
        [ "L'animation : des clips en données, une progression générale (<span class=\"tt\">LOT-46</span>)", "guide-rendu.html#autotoc_md218", null ]
      ] ],
      [ "<a class=\"el\" href=\"classhmi_1_1SpriteRenderer.html\" title=\"hmi::SpriteRenderer\">hmi::SpriteRenderer</a> : le pont ECS → écran", "guide-rendu.html#autotoc_md219", [
        [ "Ne dessiner que ce qui se voit : le culling (<span class=\"tt\">LOT-40</span>)", "guide-rendu.html#autotoc_md220", null ],
        [ "Deux modes de rendu : Physique et Texture (<span class=\"tt\">LOT-41</span>)", "guide-rendu.html#autotoc_md221", null ],
        [ "Isoler un calque pour l'audit : <span class=\"tt\">hmi::LayerVisibility</span> (<span class=\"tt\">LOT-51</span>)", "guide-rendu.html#autotoc_md222", null ],
        [ "Interpoler le mouvement : <span class=\"tt\">hmi::PreviousPosition</span> et le facteur d'interpolation", "guide-rendu.html#autotoc_md223", null ],
        [ "Plans picturaux et parallaxe (<span class=\"tt\">LOT-69</span>)", "guide-rendu.html#autotoc_md224", [
          [ "Densité : un compromis de mémoire, jamais de cadrage", "guide-rendu.html#autotoc_md225", null ],
          [ "Profondeur : deux valeurs, pas trois", "guide-rendu.html#autotoc_md226", null ],
          [ "Parallaxe : portée par le plan, bornée, et seulement là où la caméra défile", "guide-rendu.html#autotoc_md227", null ],
          [ "Coût : constant en taille de niveau, et c'est tout le problème", "guide-rendu.html#autotoc_md228", null ],
          [ "Peindre un plan : le mode création", "guide-rendu.html#autotoc_md229", null ],
          [ "Les plans du contenu livré", "guide-rendu.html#autotoc_md230", null ]
        ] ]
      ] ],
      [ "Le texte dans la scène : <span class=\"tt\">hmi::BitmapFont</span> et <span class=\"tt\">hmi::TextRenderer</span> (<span class=\"tt\">LOT-52</span>)", "guide-rendu.html#autotoc_md231", null ],
      [ "Ombres du plan physique (<span class=\"tt\">LOT-55</span>)", "guide-rendu.html#autotoc_md232", null ],
      [ "Particules et secousse d'écran (<span class=\"tt\">LOT-53</span>)", "guide-rendu.html#autotoc_md233", [
        [ "L'émetteur, dans <span class=\"tt\">Core</span>, déterministe (<span class=\"tt\">core::ParticleSystem</span>)", "guide-rendu.html#autotoc_md234", null ],
        [ "Les déclencheurs, câblés dans <span class=\"tt\">hmi::GameSession</span>", "guide-rendu.html#autotoc_md235", null ],
        [ "Le rendu (<span class=\"tt\">hmi::ParticleRenderer</span>) et la secousse d'écran (<span class=\"tt\">hmi::Camera2D</span>)", "guide-rendu.html#autotoc_md236", null ]
      ] ],
      [ "Assembler la frame complète", "guide-rendu.html#autotoc_md237", null ],
      [ "Le programme d'habillage, livré (<span class=\"tt\">LOT-40</span> → <span class=\"tt\">LOT-55</span>)", "guide-rendu.html#autotoc_md238", null ],
      [ "Budget de rendu mesuré (<span class=\"tt\">LOT-62</span>)", "guide-rendu.html#autotoc_md239", [
        [ "Le test de non-régression du volume (<span class=\"tt\">Source/Test/Unit/HMI/Graphics/test_render_budget.cpp</span>)", "guide-rendu.html#autotoc_md240", [
          [ "Le second axe : la mémoire de texture des plans (<span class=\"tt\">LOT-69</span>)", "guide-rendu.html#autotoc_md241", null ]
        ] ],
        [ "Mesures de référence, à la date du <span class=\"tt\">LOT-62</span> (2026-08-12)", "guide-rendu.html#autotoc_md242", null ],
        [ "Compteur de diagnostic en jeu (<span class=\"tt\">F9</span>)", "guide-rendu.html#autotoc_md243", null ]
      ] ],
      [ "Voir aussi", "guide-rendu.html#autotoc_md244", null ]
    ] ],
    [ "Journalisation et assertions", "guide-journalisation.html", [
      [ "Pourquoi journaliser dans un jeu vidéo", "guide-journalisation.html#autotoc_md142", null ],
      [ "Les niveaux de gravité : <a class=\"el\" href=\"namespacecore.html#aa9b5a444ee11933c91d5e3235aa5b5e3\" title=\"core::LogLevel\">core::LogLevel</a>", "guide-journalisation.html#autotoc_md143", null ],
      [ "<a class=\"el\" href=\"classcore_1_1Logger.html\" title=\"core::Logger\">core::Logger</a> : filtrer puis diffuser", "guide-journalisation.html#autotoc_md144", null ],
      [ "Les sinks : où finissent les messages", "guide-journalisation.html#autotoc_md145", null ],
      [ "Les macros de journalisation, par catégorie", "guide-journalisation.html#autotoc_md146", [
        [ "Chaque module a sa propre catégorie", "guide-journalisation.html#autotoc_md147", null ],
        [ "Une règle de performance à respecter", "guide-journalisation.html#autotoc_md148", null ]
      ] ],
      [ "Le format d'une ligne : <a class=\"el\" href=\"namespacecore.html#aafa85d91ee91c84b123fff01f147615e\" title=\"core::formatLogLine\">core::formatLogLine</a>", "guide-journalisation.html#autotoc_md149", null ],
      [ "Configurer le niveau minimal au lancement", "guide-journalisation.html#autotoc_md150", [
        [ "Bootstrap réel : sinks différents en développement et en Release", "guide-journalisation.html#autotoc_md151", null ]
      ] ],
      [ "Assertions : <a class=\"el\" href=\"Assert_8h.html#a2811f3d423b22f1a90179b4ed095f054\" title=\"JADG_ASSERT\">JADG_ASSERT</a>, un outil différent", "guide-journalisation.html#autotoc_md152", null ],
      [ "Voir aussi", "guide-journalisation.html#autotoc_md153", null ]
    ] ],
    [ "Éditeur de niveaux intégré", "guide-editeur.html", [
      [ "Le problème : éditer un niveau sans (re)coder le moteur", "guide-editeur.html#autotoc_md96", null ],
      [ "<a class=\"el\" href=\"classcore_1_1LevelDraft.html\" title=\"core::LevelDraft\">core::LevelDraft</a> : un niveau qu'on peut défaire", "guide-editeur.html#autotoc_md97", [
        [ "Mécanismes : qui a le droit de se lier à qui", "guide-editeur.html#autotoc_md98", null ],
        [ "Lier des mécanismes dans l'éditeur (<span class=\"tt\">LOT-37</span>, <span class=\"tt\">EX-IHM-030</span>/<span class=\"tt\">EX-IHM-031</span>)", "guide-editeur.html#autotoc_md99", null ]
      ] ],
      [ "<a class=\"el\" href=\"classcore_1_1LevelWriter.html\" title=\"core::LevelWriter\">core::LevelWriter</a> : l'inverse du chargement, avec un piège", "guide-editeur.html#autotoc_md100", null ],
      [ "Peindre, c'est convertir un pixel en case", "guide-editeur.html#autotoc_md101", [
        [ "La palette et les outils : des panneaux Qt séparés du canevas", "guide-editeur.html#autotoc_md102", null ],
        [ "L'outil « Parcours » : dessiner la route d'un élément mobile", "guide-editeur.html#autotoc_md103", null ],
        [ "Trois outils, une même grille : <a class=\"el\" href=\"namespacehmi.html#a02048ad8ad69a87a10e8307ac2bd68dd\" title=\"hmi::EditorTool\">EditorTool</a>", "guide-editeur.html#autotoc_md104", null ],
        [ "Peindre par lot sans dupliquer la logique de peinture : <a class=\"el\" href=\"classcore_1_1LevelDraft.html#a7fb667e044067f5e9bf16f747d3f5f8e\" title=\"core::LevelDraft::paintRegion\">LevelDraft::paintRegion</a>", "guide-editeur.html#autotoc_md105", null ]
      ] ],
      [ "Annuler/refaire : pourquoi des instantanés complets", "guide-editeur.html#autotoc_md106", null ],
      [ "Essai immédiat : jouer sans quitter l'éditeur", "guide-editeur.html#autotoc_md107", null ],
      [ "Enregistrer : valider avant d'écrire, jamais l'inverse", "guide-editeur.html#autotoc_md108", null ],
      [ "Garde-fous contre la perte de travail", "guide-editeur.html#autotoc_md109", null ],
      [ "Cadrer un niveau plus grand que la fenêtre", "guide-editeur.html#autotoc_md110", null ],
      [ "Le mode « définition des textures » : auditer les calques sans les confondre avec <span class=\"tt\">F8</span> (<span class=\"tt\">LOT-51</span>)", "guide-editeur.html#autotoc_md111", null ],
      [ "Le mode création : peindre le décor du niveau (<span class=\"tt\">LOT-69</span>)", "guide-editeur.html#autotoc_md112", [
        [ "Le panneau « Plans »", "guide-editeur.html#autotoc_md113", null ],
        [ "Cycle de vie des fichiers", "guide-editeur.html#autotoc_md114", null ],
        [ "Peindre", "guide-editeur.html#autotoc_md115", null ],
        [ "La référence : un repère géométrique, pas un aperçu", "guide-editeur.html#autotoc_md116", null ]
      ] ],
      [ "Gérer ses fichiers de niveaux", "guide-editeur.html#autotoc_md117", null ],
      [ "Voir aussi", "guide-editeur.html#autotoc_md118", null ]
    ] ],
    [ "Atelier pixel art intégré", "guide-atelier-pixel-art.html", [
      [ "Le problème : quitter l'application pour changer un pixel", "guide-atelier-pixel-art.html#autotoc_md31", null ],
      [ "Écrire un PNG sans jamais laisser de fichier tronqué", "guide-atelier-pixel-art.html#autotoc_md32", null ],
      [ "Les outils : des fonctions pures sur un tampon de pixels", "guide-atelier-pixel-art.html#autotoc_md33", null ],
      [ "L'historique : local au canevas, et nommé", "guide-atelier-pixel-art.html#autotoc_md34", null ],
      [ "Le canevas : la géométrie d'abord, le widget ensuite", "guide-atelier-pixel-art.html#autotoc_md35", null ],
      [ "Palettes", "guide-atelier-pixel-art.html#autotoc_md36", null ],
      [ "Créer un asset à une taille forcément conforme", "guide-atelier-pixel-art.html#autotoc_md37", null ],
      [ "Voir le raccord avant de l'avoir posé", "guide-atelier-pixel-art.html#autotoc_md38", null ],
      [ "Une seule paire Annuler/Refaire, deux cibles", "guide-atelier-pixel-art.html#autotoc_md39", null ],
      [ "Voir aussi", "guide-atelier-pixel-art.html#autotoc_md40", null ]
    ] ],
    [ "Écrans, navigation et boucle de jeu", "guide-ecrans.html", [
      [ "La machine à états : <span class=\"tt\">hmi::ScreenFlow</span>", "guide-ecrans.html#autotoc_md75", [
        [ "Pages empilées et recouvrements : deux patrons distincts", "guide-ecrans.html#autotoc_md76", null ]
      ] ],
      [ "Qui déclenche les transitions : les signaux", "guide-ecrans.html#autotoc_md77", null ],
      [ "Le viewport partagé : éditeur <b>et</b> jeu", "guide-ecrans.html#autotoc_md78", null ],
      [ "Pause : suspendre sans perdre un pas", "guide-ecrans.html#autotoc_md79", null ],
      [ "Fin de niveau et fin de séquence", "guide-ecrans.html#autotoc_md80", null ],
      [ "Sélection de niveau et progression persistée (<span class=\"tt\">LOT-59</span> TACHE-05/06)", "guide-ecrans.html#autotoc_md81", [
        [ "Le modèle : <span class=\"tt\">hmi::Progression</span>", "guide-ecrans.html#autotoc_md82", null ],
        [ "La règle de déverrouillage : <span class=\"tt\">hmi::isLevelUnlocked</span>", "guide-ecrans.html#autotoc_md83", null ],
        [ "Le menu : trois intentions", "guide-ecrans.html#autotoc_md84", null ]
      ] ],
      [ "Où ça s'insère dans la boucle", "guide-ecrans.html#autotoc_md85", null ],
      [ "Voir aussi", "guide-ecrans.html#autotoc_md86", null ]
    ] ],
    [ "IHM Qt (refonte) — socle applicatif &amp; viewport Direct3D 11", "guide-ihm-qt.html", [
      [ "Pourquoi Qt", "guide-ihm-qt.html#autotoc_md130", null ],
      [ "Une seule cible : l'application Qt", "guide-ihm-qt.html#autotoc_md131", null ],
      [ "Le viewport : Direct3D 11 dans une fenêtre Qt", "guide-ihm-qt.html#autotoc_md132", null ],
      [ "La boucle : Qt pilote, le pas fixe est préservé", "guide-ihm-qt.html#autotoc_md133", null ],
      [ "Les entrées : événements Qt vers l'état partagé", "guide-ihm-qt.html#autotoc_md134", null ],
      [ "Jouer un niveau : <span class=\"tt\">hmi::GameSession</span> réutilisée", "guide-ihm-qt.html#autotoc_md135", null ],
      [ "Éditeur : docks, palette, peinture (LOT-35)", "guide-ihm-qt.html#autotoc_md136", [
        [ "Gestion des niveaux (LOT-36)", "guide-ihm-qt.html#autotoc_md137", null ],
        [ "Liens de mécanismes (LOT-37) et unification des menus (LOT-38)", "guide-ihm-qt.html#autotoc_md138", null ]
      ] ],
      [ "La taille : aucun écran ne contraint la fenêtre (LOT-73)", "guide-ihm-qt.html#autotoc_md139", null ],
      [ "Ce que cette page ne couvre pas", "guide-ihm-qt.html#autotoc_md140", null ],
      [ "Voir aussi", "guide-ihm-qt.html#autotoc_md141", null ]
    ] ],
    [ "Système de design et architecture de l'information", "guide-design-ihm.html", [
      [ "Pourquoi un système de design", "guide-design-ihm.html#autotoc_md58", null ],
      [ "Les jetons : une seule source, deux portées étanches", "guide-design-ihm.html#autotoc_md59", null ],
      [ "De jetons purs à une application habillée", "guide-design-ihm.html#autotoc_md60", null ],
      [ "Typographie : une police embarquée, un repli qui n'invente rien", "guide-design-ihm.html#autotoc_md61", null ],
      [ "Les commandes : une définition, trois surfaces", "guide-design-ihm.html#autotoc_md62", null ],
      [ "Netteté à toute échelle d'affichage", "guide-design-ihm.html#autotoc_md63", null ],
      [ "Thème clair/sombre", "guide-design-ihm.html#autotoc_md64", null ],
      [ "Architecture de l'information : ce qui informe reste, ce qui commande est unique", "guide-design-ihm.html#autotoc_md65", [
        [ "Une barre d'état structurée", "guide-design-ihm.html#autotoc_md66", null ],
        [ "Des panneaux groupés, et qui suivent l'outil", "guide-design-ihm.html#autotoc_md67", null ],
        [ "Un état, un contrôle", "guide-design-ihm.html#autotoc_md68", null ]
      ] ],
      [ "Une feuille par portée, et pourquoi (LOT-73)", "guide-design-ihm.html#autotoc_md69", [
        [ "Le Mode IA : enveloppe d'identité, contenu d'outil", "guide-design-ihm.html#autotoc_md70", null ]
      ] ],
      [ "Deux identités, deux règles d'échelle (LOT-68)", "guide-design-ihm.html#autotoc_md71", [
        [ "Ce qu'une feuille de style ne sait pas faire", "guide-design-ihm.html#autotoc_md72", null ]
      ] ],
      [ "Hiérarchie des surfaces, et espaces de travail (LOT-68)", "guide-design-ihm.html#autotoc_md73", null ],
      [ "Voir aussi", "guide-design-ihm.html#autotoc_md74", null ]
    ] ],
    [ "Audio", "guide-audio.html", [
      [ "La règle d'or, une fois de plus", "guide-audio.html#autotoc_md41", null ],
      [ "Le socle : <span class=\"tt\">hmi::AudioEngine</span>", "guide-audio.html#autotoc_md42", null ],
      [ "Le catalogue : <span class=\"tt\">hmi::SoundCatalog</span>", "guide-audio.html#autotoc_md43", null ],
      [ "Détection des transitions : <span class=\"tt\">hmi::GameEvents</span>", "guide-audio.html#autotoc_md44", null ],
      [ "La table : <span class=\"tt\">hmi::SoundTriggers</span>", "guide-audio.html#autotoc_md45", null ],
      [ "Provisionnement : Qt Multimedia", "guide-audio.html#autotoc_md46", null ],
      [ "Voir aussi", "guide-audio.html#autotoc_md47", null ]
    ] ]
];