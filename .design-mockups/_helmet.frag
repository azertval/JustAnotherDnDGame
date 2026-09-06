<helmet>
  <link rel="stylesheet" href="https://fonts.googleapis.com/css2?family=Press+Start+2P&family=Pixelify+Sans:wght@400;600;700&display=swap">
  <style>
    :root { color-scheme: dark; }
    body { margin: 0; background: #0b0e14; }
    a { color: #ffd133; } a:hover { color: #ffdb5c; }
    .screen {
      width: 1280px; height: 720px; position: relative; overflow: hidden;
      background: #12161f;
      font-family: 'Pixelify Sans', 'Courier New', monospace;
      color: #f2f2ff;
      --ink: #0a0d13;
      --surface: #1e2531;
      --surfaceAlt: #262f3e;
      --bevelHi: #46536b;
      --bevelLo: #10141c;
      --muted: #8b93a7;
      --dim: #4c5468;
    }
    /* Trame de fond : quadrillage 32px, le pas de tuile du jeu. */
    .grid-bg {
      position: absolute; inset: 0; pointer-events: none;
      background-image:
        linear-gradient(to right, rgba(255,255,255,.028) 2px, transparent 2px),
        linear-gradient(to bottom, rgba(255,255,255,.028) 2px, transparent 2px);
      background-size: 32px 32px;
    }
    .vignette { position: absolute; inset: 0; pointer-events: none;
      background: radial-gradient(120% 90% at 50% 40%, transparent 45%, rgba(0,0,0,.55) 100%); }
    /* Cadre pixel : anneau noir a coins entailles + biseau clair/sombre. */
    .frame {
      position: relative; background: var(--surface);
      box-shadow:
        0 -4px 0 var(--edge), 0 4px 0 var(--edge), -4px 0 0 var(--edge), 4px 0 0 var(--edge),
        inset 0 4px 0 var(--ornament), inset 4px 0 0 var(--ornament),
        inset 0 -4px 0 var(--shadow), inset -4px 0 0 var(--shadow);
    }
    .frame--accent {
      box-shadow:
        0 -4px 0 var(--edge), 0 4px 0 var(--edge), -4px 0 0 var(--edge), 4px 0 0 var(--edge),
        inset 0 4px 0 var(--accent), inset 4px 0 0 var(--accent),
        inset 0 -4px 0 #8a6a00, inset -4px 0 0 #8a6a00;
    }
    .title {
      font-family: 'Press Start 2P', 'Courier New', monospace;
      color: var(--accent); line-height: 1.35;
      text-shadow: 4px 4px 0 var(--edge);
    }
    .row {
      display: flex; align-items: center; gap: 16px;
      padding: 10px 18px; color: var(--muted);
      font-size: 28px; font-weight: 600; letter-spacing: 1px;
    }
    .row .caret { visibility: hidden; color: var(--accent); flex: none; }
    .row--focus { color: var(--accent); background: rgba(255,209,51,.10);
      box-shadow: inset 0 0 0 4px rgba(255,209,51,.30); }
    .row--focus .caret { visibility: visible; }
    .row--off { color: var(--dim); }
    .hint {
      display: flex; align-items: center; gap: 10px;
      font-size: 17px; color: var(--dim); letter-spacing: 1px;
    }
    .key {
      font-family: 'Press Start 2P', monospace; font-size: 11px; color: var(--muted);
      padding: 6px 8px; background: var(--surfaceAlt);
      box-shadow: 0 -2px 0 var(--edge), 0 2px 0 var(--edge), -2px 0 0 var(--edge), 2px 0 0 var(--edge),
                  inset 0 2px 0 var(--ornament), inset 0 -2px 0 var(--shadow);
    }
  </style>
</helmet>
