<div class="screen" style="--accent: {{accent}}">
  <div class="grid-bg"></div>

  <div style="position: absolute; inset: 0; display: flex; flex-direction: column; padding: 56px 88px 40px;">
    <!-- En-tete : titre a gauche, entree « Retour » a droite, comme l'ecran Options. -->
    <div style="display: flex; align-items: baseline;">
      <div class="title" style="font-size: 32px;">MODE IA</div>
      <div style="flex-grow: 1;"></div>
      <div style="display: flex; align-items: center; gap: 12px; font-size: 21px; font-weight: bold; color: var(--muted);"><span style="visibility: hidden;">@CARET@</span><span>Retour</span></div>
    </div>
    <div style="height: 30px;"></div>

    <!-- Trois onglets, ceux du .ui. L'actif est un aplat accent, comme dans Options. -->
    <div style="display: flex; gap: 8px;">
      <div style="padding: 14px 26px; font-size: 21px; font-weight: 600; color: #12161f; background: var(--accent); box-shadow: 0 -4px 0 var(--edge), -4px 0 0 var(--edge), 4px 0 0 var(--edge), inset 0 4px 0 rgba(255,255,255,.35);">Entraînement</div>
      <div style="padding: 14px 26px; font-size: 21px; font-weight: 600; color: var(--muted); background: var(--surfaceAlt); box-shadow: 0 -4px 0 var(--edge), -4px 0 0 var(--edge), 4px 0 0 var(--edge), inset 0 4px 0 var(--ornament);">Validation &amp; sauvegarde</div>
      <div style="padding: 14px 26px; font-size: 21px; font-weight: 600; color: var(--muted); background: var(--surfaceAlt); box-shadow: 0 -4px 0 var(--edge), -4px 0 0 var(--edge), 4px 0 0 var(--edge), inset 0 4px 0 var(--ornament);">Rejeu</div>
    </div>

    <!-- Cadre a bordure franche autour du contenu (EX-IHM-070) : c'est ce que la planche
         ajoute a l'ecran livre au LOT-ANNEXE-21, qui n'en portait aucun. -->
    <div class="frame" style="flex-grow: 1; padding: 30px 40px; display: flex; gap: 40px;">

      <!-- Colonne gauche : les hyperparametres, un par ligne. -->
      <div style="width: 520px; display: flex; flex-direction: column; gap: 18px;">
        <!-- Ligne focalisee : curseur explicite ET teinte. La teinte seule ne suffit pas
             (EX-IHM-071) — c'est le manque que cette planche corrige. -->
        <div style="display: flex; align-items: center; gap: 20px; padding: 6px 14px; margin: -6px -14px; background: rgba(255,209,51,.10); box-shadow: inset 0 0 0 4px rgba(255,209,51,.30);">
          <div style="width: 34px; flex: none; color: var(--accent);">@CARET@</div>
          <div style="width: 260px; font-size: 22px; color: var(--accent);">Niveau</div>
          <div style="flex-grow: 1; padding: 6px 12px; font-size: 20px; color: #f2f2ff; background: var(--surfaceAlt); box-shadow: 0 -4px 0 var(--edge), 0 4px 0 var(--edge), -4px 0 0 var(--edge), 4px 0 0 var(--edge);">demo-final</div>
        </div>

        <div style="display: flex; align-items: center; gap: 20px;">
          <div style="width: 34px; flex: none;"></div>
          <div style="width: 260px; font-size: 22px;">Algorithme</div>
          <div style="flex-grow: 1; display: flex; gap: 18px; font-size: 19px; color: var(--muted);">
            <span style="color: var(--accent);">■ Évolutif</span><span>□ REINFORCE</span><span>□ DQN</span>
          </div>
        </div>

        <div style="display: flex; align-items: center; gap: 20px;">
          <div style="width: 34px; flex: none;"></div>
          <div style="width: 260px; font-size: 22px;">Taille de population</div>
          <div style="width: 120px; padding: 6px 12px; font-size: 20px; color: #f2f2ff; background: var(--surfaceAlt); box-shadow: 0 -4px 0 var(--edge), 0 4px 0 var(--edge), -4px 0 0 var(--edge), 4px 0 0 var(--edge);">64</div>
        </div>

        <div style="display: flex; align-items: center; gap: 20px;">
          <div style="width: 34px; flex: none;"></div>
          <div style="width: 260px; font-size: 22px;">Taux de mutation</div>
          <div style="width: 120px; padding: 6px 12px; font-size: 20px; color: #f2f2ff; background: var(--surfaceAlt); box-shadow: 0 -4px 0 var(--edge), 0 4px 0 var(--edge), -4px 0 0 var(--edge), 4px 0 0 var(--edge);">0,080</div>
        </div>

        <div style="display: flex; align-items: center; gap: 20px;">
          <div style="width: 34px; flex: none;"></div>
          <div style="width: 260px; font-size: 22px;">Épisodes / générations max</div>
          <div style="width: 120px; padding: 6px 12px; font-size: 20px; color: #f2f2ff; background: var(--surfaceAlt); box-shadow: 0 -4px 0 var(--edge), 0 4px 0 var(--edge), -4px 0 0 var(--edge), 4px 0 0 var(--edge);">300</div>
        </div>

        <div style="display: flex; align-items: center; gap: 20px;">
          <div style="width: 34px; flex: none;"></div>
          <div style="width: 260px; font-size: 22px;">Graine</div>
          <div style="width: 120px; padding: 6px 12px; font-size: 20px; color: #f2f2ff; background: var(--surfaceAlt); box-shadow: 0 -4px 0 var(--edge), 0 4px 0 var(--edge), -4px 0 0 var(--edge), 4px 0 0 var(--edge);">7777</div>
        </div>

        <div style="flex-grow: 1;"></div>
        <!-- Boutons d'action : aplat accent, contour franc. Le curseur de focus se pose a leur
             gauche, hors du bouton, comme sur les lignes ci-dessus. -->
        <div style="display: flex; gap: 16px;">
          <div style="padding: 12px 28px; font-size: 20px; color: #12161f; background: var(--accent); box-shadow: 0 -4px 0 var(--edge), 0 4px 0 var(--edge), -4px 0 0 var(--edge), 4px 0 0 var(--edge);">Lancer l'entraînement</div>
          <div style="padding: 12px 28px; font-size: 20px; color: var(--muted); background: var(--surfaceAlt); box-shadow: 0 -4px 0 var(--edge), 0 4px 0 var(--edge), -4px 0 0 var(--edge), 4px 0 0 var(--edge);">Arrêter l'entraînement</div>
        </div>
      </div>

      <!-- Colonne droite : le tableau des generations, qui se remplit pendant le run. -->
      <div style="flex-grow: 1; display: flex; flex-direction: column;">
        <div style="display: flex; font-size: 17px; color: var(--muted); letter-spacing: 1px; background: var(--surfaceAlt); box-shadow: 0 4px 0 var(--edge);">
          <div style="flex: 1; padding: 10px 8px;">GÉNÉRATION</div>
          <div style="flex: 1.4; padding: 10px 8px;">MEILLEURE RÉCOMPENSE</div>
          <div style="flex: 1.3; padding: 10px 8px;">RÉCOMPENSE MOYENNE</div>
          <div style="flex: 1; padding: 10px 8px;">TAUX DE RÉUSSITE</div>
        </div>
        <div style="display: flex; font-size: 19px; padding: 2px 0; background: var(--surface);"><div style="flex: 1; padding: 8px;">12</div><div style="flex: 1.4; padding: 8px;">18,42</div><div style="flex: 1.3; padding: 8px;">9,10</div><div style="flex: 1; padding: 8px;">0,25</div></div>
        <div style="display: flex; font-size: 19px; padding: 2px 0; background: var(--surfaceAlt);"><div style="flex: 1; padding: 8px;">13</div><div style="flex: 1.4; padding: 8px;">21,07</div><div style="flex: 1.3; padding: 8px;">11,88</div><div style="flex: 1; padding: 8px;">0,31</div></div>
        <div style="display: flex; font-size: 19px; padding: 2px 0; background: var(--surface);"><div style="flex: 1; padding: 8px;">14</div><div style="flex: 1.4; padding: 8px;">24,90</div><div style="flex: 1.3; padding: 8px;">14,02</div><div style="flex: 1; padding: 8px;">0,44</div></div>
        <div style="display: flex; font-size: 19px; padding: 2px 0; background: var(--surfaceAlt);"><div style="flex: 1; padding: 8px;">15</div><div style="flex: 1.4; padding: 8px;">27,63</div><div style="flex: 1.3; padding: 8px;">16,71</div><div style="flex: 1; padding: 8px;">0,56</div></div>

        <div style="flex-grow: 1;"></div>
        <div style="display: flex; align-items: center; gap: 16px;">
          <div style="font-size: 19px; color: var(--dim);">Génération 15 / 300</div>
          <div style="flex-grow: 1;"></div>
          <div style="padding: 12px 28px; font-size: 20px; color: #12161f; background: var(--accent); box-shadow: 0 -4px 0 var(--edge), 0 4px 0 var(--edge), -4px 0 0 var(--edge), 4px 0 0 var(--edge);">Voir en jeu</div>
        </div>
      </div>

    </div>
  </div>
</div>
