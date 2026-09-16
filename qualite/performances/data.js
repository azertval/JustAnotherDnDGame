window.BENCHMARK_DATA = {
  "lastUpdate": 1789526335067,
  "repoUrl": "https://github.com/azertval/JustAnotherDnDGame",
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
      }
    ]
  }
}