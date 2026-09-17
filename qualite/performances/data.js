window.BENCHMARK_DATA = {
  "lastUpdate": 1789662660546,
  "repoUrl": "https://github.com/azertval/JustAnotherRpgGame",
  "entries": {
    "Combat et niveaux (Release, windows-2022)": [
      {
        "commit": {
          "author": {
            "name": "azertval",
            "username": "azertval",
            "email": "valentin.eloy@gmail.com"
          },
          "committer": {
            "name": "GitHub",
            "username": "web-flow",
            "email": "noreply@github.com"
          },
          "id": "0efad242f23214e99fc0bc4bc2c6cc944153d58b",
          "message": "CI — Refonte phase 4 : Qt, données, vitrine (#56)\n\n* ci: refonte phase 4 — Qt, données, vitrine\n\nLa filière contenu et l'interface reçoivent les mêmes garde-fous que le C++.\n\n- QmlTests (Qt Quick Test) : chaque .ui.qml de Jadg.Ui se construit sans\n  avertissement, comportement des briques OrnateButton/OrnateCheck, et les\n  15 écrans comparés à leur capture de référence (rendu logiciel, 960x540).\n- check_translations.py : catalogue .ts achevé et cohérent ; build-ninja\n  relance lupdate (-no-obsolete) pour prouver qu'il est à jour du code.\n  Deux chaînes manquantes traduites, trois entrées mortes retirées.\n- Minidump sur plantage (HMI/Platform/CrashDump) pour le jeu et l'éditeur ;\n  --crash-test éprouvé par le test de fumée des archives, qui n'embarquent\n  plus Logs/ ni Crashes/.\n- pyproject.toml + uv.lock (jsonschema, pytest), scripts/tests sous pytest\n  à la place des étapes --auto-test, rapport JUnit dans test-report.\n- PSScriptAnalyzer (check_powershell.py) ; deux défauts de setup_dev.ps1.\n- Site qualité sur gh-pages (/qualite/) : couverture de main par domaine,\n  performances de la nuit ; coverage.ps1 partagé avec ci.yml.\n- Renovate pour les GIT_TAG de FetchContent, Dependabot pour uv.lock.\n\nCo-Authored-By: Claude Opus 5 <noreply@anthropic.com>\n\n* fix(crash-dump): écrire le minidump depuis un thread dédié\n\nEn CI, sous le générateur Visual Studio en Debug, MiniDumpWriteDump\néchouait quand le thread fautif se décrivait lui-même depuis son filtre SEH\n(CrashDumpTest.EcritUnMinidumpAvecLeContexteDUneException) ; Ninja et Release\npassaient. L'écriture a désormais lieu sur un thread que l'appelant attend,\ncomme le recommande la documentation de dbghelp, et GetLastError en donne la\nraison en cas d'échec.\n\n--crash-test lève EXCEPTION_ACCESS_VIOLATION au lieu de déréférencer un\npointeur nul : même code de sortie et même chemin par le filtre, sans le\ncomportement indéfini que cppcheck signalait (Code scanning).\n\nCo-Authored-By: Claude Opus 5 <noreply@anthropic.com>\n\n* fix(crash-dump): dump réduit si le dump riche échoue\n\nSur le runner, MiniDumpWriteDump avec contexte d'exception échoue de façon\nintermittente en 0x800706F8 (ERROR_INVALID_USER_BUFFER), sous Ninja comme\nsous Visual Studio, jamais sur le poste (300 répétitions). Cause la plus\nprobable : le balayage de la mémoire référencée par les piles lit des pages\nqui changent avant l'écriture. Second essai sans ce balayage, qui garde piles\net contexte ; le test exige désormais un flux d'exception dans le dump.\n\nCo-Authored-By: Claude Opus 5 <noreply@anthropic.com>\n\n* fix(crash-dump): replis sur un contexte sans état étendu\n\nLe dump réduit échouait lui aussi sur le runner (0x800706F8) : ce n'est pas\nle balayage mémoire mais le contexte d'exception d'origine que dbghelp refuse,\nsur une partie des runners seulement (poste : i7-8700 sans AVX-512).\nHypothèse retenue : l'état étendu du processeur (CONTEXT_XSTATE).\n\nTrois essais : dump riche avec le contexte d'origine, dump réduit avec une\ncopie limitée au CONTEXT de base, dump réduit avec ClientPointers. Chaque repli\néprouvé seul sur le poste : dump valide avec flux d'exception.\n\nCo-Authored-By: Claude Opus 5 <noreply@anthropic.com>\n\n---------\n\nCo-authored-by: Claude Opus 5 <noreply@anthropic.com>",
          "timestamp": "2026-09-15T20:53:45Z",
          "url": "https://github.com/azertval/JustAnotherDnDGame/commit/0efad242f23214e99fc0bc4bc2c6cc944153d58b"
        },
        "date": 1789526332740,
        "tool": "googlecpp",
        "benches": [
          {
            "name": "ReachableAreaDashing",
            "value": 158352.86830356842,
            "unit": "ns/iter",
            "extra": "iterations: 8960\ncpu: 160435.26785714287 ns\nthreads: 1"
          },
          {
            "name": "FindPathAcrossGrid",
            "value": 216280.85937500428,
            "unit": "ns/iter",
            "extra": "iterations: 6400\ncpu: 217285.15625 ns\nthreads: 1"
          },
          {
            "name": "LineOfSightAcrossGrid",
            "value": 56996.785728634626,
            "unit": "ns/iter",
            "extra": "iterations: 24889\ncpu: 57128.65121137852 ns\nthreads: 1"
          },
          {
            "name": "CoverFromWithInterposed",
            "value": 161408.54511970468,
            "unit": "ns/iter",
            "extra": "iterations: 8145\ncpu: 161141.80478821363 ns\nthreads: 1"
          },
          {
            "name": "PlanTurnFourVersusFour",
            "value": 1107256.718750005,
            "unit": "ns/iter",
            "extra": "iterations: 1280\ncpu: 1098632.8125 ns\nthreads: 1"
          },
          {
            "name": "LoadShippedLevel",
            "value": 907769.5914266576,
            "unit": "ns/iter",
            "extra": "iterations: 1493\ncpu: 900033.4896182184 ns\nthreads: 1"
          }
        ]
      },
      {
        "commit": {
          "author": {
            "name": "azertval",
            "username": "azertval",
            "email": "valentin.eloy@gmail.com"
          },
          "committer": {
            "name": "GitHub",
            "username": "web-flow",
            "email": "noreply@github.com"
          },
          "id": "d7792a077071db467f10679376a39fbaa20e295f",
          "message": "Analyse statique — corriger les 896 alertes clang-tidy de Code scanning (#63)\n\n* clang-tidy : corrections mécaniques des alertes Code scanning\n\nFixits clang-tidy appliqués sur les 100 fichiers signalés : suffixes littéraux en majuscules,\nparenthèses des expressions mixtes, initialiseurs désignés, auto, std::ranges, emplace,\nparamètres passés par référence constante, et autres réécritures sans effet de comportement.\n\nFileLogSink : std::endl remplacé par '\\n' suivi d'un flush explicite, pour garder l'écriture\nimmédiate sur disque que les tests exigent.\n\nCo-Authored-By: Claude Opus 5 <noreply@anthropic.com>\n\n* clang-tidy : corrections manuelles des alertes Code scanning restantes\n\n- Complexité cognitive : fonctions découpées en étapes nommées (main, LevelWriter::buildJson,\n  CharacterSheet, Dialogue, EnemyAi, MainWindow::buildUi, GameViewport::tick…).\n- Défauts réels : cast élargissant après la multiplication (AssetMarker, LineOfSight), arrondi\n  par std::lround (CharacterSheetValues), compteurs flottants de PixelCanvas, garde nullptr dans\n  MainWindow::raisePanel, cas Key::E manquant dans KeyName.\n- Champs d'agrégats explicites, std::array, unions DirectXMath lues par l'API, qobject_cast,\n  constantes inutilisées supprimées, nommage aligné sur .clang-tidy.\n- JADG_ASSERT n'enveloppe plus la condition dans une négation.\n- 9 NOLINT justifiés : drapeaux MINIDUMP_TYPE, balayage des codes de touche, méthodes lues par\n  QML, reinterpret_cast imposé par QImage.\n\nCo-Authored-By: Claude Opus 5 <noreply@anthropic.com>\n\n* CHANGELOG : alertes clang-tidy corrigées\n\nCo-Authored-By: Claude Opus 5 <noreply@anthropic.com>\n\n* MechanismController : documenter les paramètres de setDoorOpen (Doxygen)\n\nCo-Authored-By: Claude Opus 5 <noreply@anthropic.com>\n\n---------\n\nCo-authored-by: Claude Opus 5 <noreply@anthropic.com>",
          "timestamp": "2026-09-17T00:18:16Z",
          "url": "https://github.com/azertval/JustAnotherRpgGame/commit/d7792a077071db467f10679376a39fbaa20e295f"
        },
        "date": 1789612715208,
        "tool": "googlecpp",
        "benches": [
          {
            "name": "ReachableAreaDashing",
            "value": 130823.49337082541,
            "unit": "ns/iter",
            "extra": "iterations: 9956\ncpu: 130260.64684612294 ns\nthreads: 1"
          },
          {
            "name": "FindPathAcrossGrid",
            "value": 177600.65622069975,
            "unit": "ns/iter",
            "extra": "iterations: 7467\ncpu: 177865.94348466586 ns\nthreads: 1"
          },
          {
            "name": "LineOfSightAcrossGrid",
            "value": 50729.56688272962,
            "unit": "ns/iter",
            "extra": "iterations: 27152\ncpu: 50640.83677077195 ns\nthreads: 1"
          },
          {
            "name": "CoverFromWithInterposed",
            "value": 147044.3350743485,
            "unit": "ns/iter",
            "extra": "iterations: 9956\ncpu: 147524.10606669346 ns\nthreads: 1"
          },
          {
            "name": "PlanTurnFourVersusFour",
            "value": 909435.8576052382,
            "unit": "ns/iter",
            "extra": "iterations: 1545\ncpu: 920307.4433656958 ns\nthreads: 1"
          },
          {
            "name": "LoadShippedLevel",
            "value": 619398.320537539,
            "unit": "ns/iter",
            "extra": "iterations: 2084\ncpu: 622300.8637236084 ns\nthreads: 1"
          }
        ]
      },
      {
        "commit": {
          "author": {
            "name": "azertval",
            "username": "azertval",
            "email": "valentin.eloy@gmail.com"
          },
          "committer": {
            "name": "GitHub",
            "username": "web-flow",
            "email": "noreply@github.com"
          },
          "id": "7821d11b506195e3fa8fca90c7bf7aa9514270c8",
          "message": "Analyse statique — corriger les 113 alertes restantes de Code scanning (#64)\n\n* Analyse statique — corriger les 113 alertes restantes de Code scanning\n\nL'analyse clang-tidy de main (d7792a077) relevait 112 alertes, et CppCheck une, dans les\nfichiers arrivés avec l'éditeur (LOT-11) et la galerie des assets : ils n'étaient pas dans le\npérimètre de la PR #63.\n\n- Correctifs mécaniques : suffixes littéraux, parenthèses, initialiseurs désignés complets,\n  std::ranges, std::cmp_*.\n- Complexité cognitive : validateMapEntities, EntityPanel::rebuildForm,\n  WorldGraphView::paintNodes et AssetGalleryRenderer::compose découpées en fonctions.\n- Transtypages : qobject_cast dans le rendu de la galerie (comme l'arène), dynamic_cast pour\n  l'infobulle du graphe ; tableau C de couleur d'effacement remplacé par std::array.\n- Analyseur : le nom d'un point d'arrivée n'est plus déplacé dans la boucle ; LevelWriter\n  n'emploie plus std::visit, que l'analyseur ne suivait pas.\n- CppCheck : le catalogue des dialogues est tenu dans une variable avant d'être parcouru.\n- Deux NOLINT justifiés en ligne : reinterpret_cast de lecture d'en-tête PNG, et _form créé\n  après setupUi.\n\nclang-tidy en local : 0 diagnostic sur les 13 fichiers ; build et 1289 tests verts.\n\nCo-Authored-By: Claude Opus 5 <noreply@anthropic.com>\n\n* Changelog — les 113 alertes restantes de Code scanning\n\nCo-Authored-By: Claude Opus 5 <noreply@anthropic.com>\n\n---------\n\nCo-authored-by: Claude Opus 5 <noreply@anthropic.com>",
          "timestamp": "2026-09-17T07:05:54Z",
          "url": "https://github.com/azertval/JustAnotherRpgGame/commit/7821d11b506195e3fa8fca90c7bf7aa9514270c8"
        },
        "date": 1789662656973,
        "tool": "googlecpp",
        "benches": [
          {
            "name": "ReachableAreaDashing",
            "value": 167720.66298341966,
            "unit": "ns/iter",
            "extra": "iterations: 8145\ncpu: 168815.22406384285 ns\nthreads: 1"
          },
          {
            "name": "FindPathAcrossGrid",
            "value": 227812.90808637897,
            "unit": "ns/iter",
            "extra": "iterations: 5973\ncpu: 227586.63987945757 ns\nthreads: 1"
          },
          {
            "name": "LineOfSightAcrossGrid",
            "value": 65373.469387753314,
            "unit": "ns/iter",
            "extra": "iterations: 21854\ncpu: 65777.4320490528 ns\nthreads: 1"
          },
          {
            "name": "CoverFromWithInterposed",
            "value": 189187.81304405027,
            "unit": "ns/iter",
            "extra": "iterations: 7467\ncpu: 188328.64604258738 ns\nthreads: 1"
          },
          {
            "name": "PlanTurnFourVersusFour",
            "value": 1185325.9821428708,
            "unit": "ns/iter",
            "extra": "iterations: 1120\ncpu: 1185825.892857143 ns\nthreads: 1"
          },
          {
            "name": "LoadShippedLevel",
            "value": 771829.0625000179,
            "unit": "ns/iter",
            "extra": "iterations: 1600\ncpu: 771484.375 ns\nthreads: 1"
          }
        ]
      }
    ]
  }
}