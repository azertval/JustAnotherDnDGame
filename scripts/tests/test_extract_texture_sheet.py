# SPDX-FileCopyrightText: 2026 Valentin Eloy
# SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

"""L'atelier des textures (LOT-92, T3) : la disposition se valide sans image, et une planche
synthétique -- chaque emprise peinte à plat, sur deux planches -- se découpe, s'installe et se revérifie.

Les tests d'image demandent Pillow et numpy, que la CI n'installe pas : ils sont sautés là, et joués
sur le poste qui produit les planches.
"""
import copy
import json

import pytest

import extract_texture_sheet as T


@pytest.fixture
def colisee():
    return T.charger('colisee')


def test_les_dispositions_de_l_atelier_sont_conformes():
    nombre, fautes = T.valider_tout()
    assert fautes == []
    assert nombre >= 36


def test_une_grille_se_deduit_sans_chevauchement(colisee):
    poses = T.grille(colisee)
    assert len(poses) == len(colisee['cells'])
    largeur, hauteur = T.taille_planche(colisee)
    for numero in range(1, T.planches(colisee) + 1):
        boites = [p['boite'] for p in poses if p['planche'] == numero]
        assert boites, f'planche {numero} vide'
        for i, (ax0, ay0, ax1, ay1) in enumerate(boites):
            assert 0 <= ax0 < ax1 <= largeur and 0 <= ay0 < ay1 <= hauteur
            for bx0, by0, bx1, by1 in boites[i + 1:]:
                assert ax1 <= bx0 or bx1 <= ax0 or ay1 <= by0 or by1 <= ay0


def test_les_cellules_debordent_sur_une_planche_de_plus(colisee):
    colisee['sheet'] = {'size': [2560, 1440], 'scale': 4}
    poses = T.grille(colisee)
    assert T.planches(colisee) == 2
    assert [p['planche'] for p in poses] == sorted(p['planche'] for p in poses)
    colisee['sheet'] = {'size': [3840, 2160], 'scale': 4}
    assert T.planches(colisee) == 1


def test_un_sol_est_le_losange_d_iso_projection(colisee):
    sol = next(c for c in colisee['cells'] if c['class'] == 'floor')
    assert T.canevas(colisee, sol) == (68, 42)
    assert T.sommets(colisee, sol) == [(34, 0), (68, 21), (34, 42), (0, 21)]


def test_une_grande_piece_a_gauche_s_allonge_sur_l_arete_haut_gauche(colisee):
    porte = next(c for c in colisee['cells'] if c['name'] == 'gate-left')
    haut, droite, bas, gauche = T.sommets(colisee, porte)
    assert (haut[0] - gauche[0], gauche[1] - haut[1]) == (68, 42)   # deux cases vers la gauche
    assert (droite[0] - haut[0], droite[1] - haut[1]) == (34, 21)   # une vers la droite


@pytest.mark.parametrize('faute, attendu', [
    (lambda d: d['cells'].append(copy.deepcopy(d['cells'][0])), 'deux fois'),
    (lambda d: d.update(keyPrefix='ui/coliseum'), 'scene/'),
    (lambda d: d.update(location='lieu-inexistant'), "absent de l'atlas"),
    (lambda d: d['cells'][0].update({'class': 'inconnue'}), 'inconnue'),
    (lambda d: d.update(sheet={'size': [2560, 1450], 'scale': 4}), 'multiples de 16'),
    (lambda d: d.update(sheet={'size': [4096, 2048], 'scale': 4}), 'au plus 3840'),
    (lambda d: d.update(sheet={'size': [3200, 800], 'scale': 2}), 'rapport'),
    (lambda d: d.update(sheet={'size': [768, 512], 'scale': 2}), 'pixels'),
    (lambda d: d.update(sheet={'size': [2560, 1440], 'scale': 12}), 'ne tient pas'),
])
def test_une_disposition_fautive_est_refusee(colisee, faute, attendu):
    faute(colisee)
    assert any(attendu in f for f in T.valider(colisee))


def test_le_bloc_c_nomme_chaque_cellule_de_sa_planche_dans_l_ordre(colisee):
    for numero in range(1, T.planches(colisee) + 1):
        texte = T.bloc_c(colisee, numero)
        cellules = [p['cellule'] for p in T.grille(colisee) if p['planche'] == numero]
        for rang, cellule in enumerate(cellules, 1):
            assert f"{rang}. {cellule['prompt']}" in texte
        assert f"{rang + 1}. " not in texte


def test_le_bloc_a_prend_le_pas_de_la_disposition(colisee):
    colisee['sheet'] = {'size': [2560, 1440], 'scale': 4}
    texte = T.bloc_a(colisee)
    assert '4 screen pixels per art pixel' in texte and '272 screen pixels wide and 168' in texte
    assert '{' not in texte and not any(l.startswith('#') for l in texte.splitlines())


# -- Avec images ---------------------------------------------------------------------------------

def _planches(disposition, trou=None):
    np = pytest.importorskip('numpy')
    Image = pytest.importorskip('PIL.Image')
    ImageDraw = pytest.importorskip('PIL.ImageDraw')
    s = T.pas(disposition)
    images = [Image.new('RGBA', T.taille_planche(disposition), (0, 0, 0, 0))
              for _ in range(T.planches(disposition))]
    for rang, pose in enumerate(T.grille(disposition)):
        cellule = pose['cellule']
        trait = ImageDraw.Draw(images[pose['planche'] - 1])
        ox, oy = pose['origine']
        couleur = (40 + 5 * rang, 120, 200 - 4 * rang, 255)
        points = [(ox + x * s, oy + y * s) for x, y in T.sommets(disposition, cellule)]
        trait.polygon(points, fill=couleur)
        hausse = disposition['classes'][cellule['class']]['rise']
        if hausse:
            w, _ = T.canevas(disposition, cellule)
            trait.rectangle([ox + 10 * s, oy, ox + (w - 10) * s, oy + hausse * s], fill=couleur)
        if cellule['name'] == trou:
            x, y = points[0]
            trait.rectangle([x - 30 * s, y, x + 30 * s, y + 30 * s], fill=(0, 0, 0, 0))
    return np, Image, images


@pytest.fixture
def depot(tmp_path, monkeypatch, colisee):
    monkeypatch.setattr(T, 'RACINE', tmp_path)
    return tmp_path


def test_des_planches_se_decoupent_s_installent_et_se_reverifient(depot, colisee, tmp_path):
    np, Image, planches = _planches(colisee)
    candidats = []
    for numero, planche in enumerate(planches, 1):
        candidats.append(tmp_path / f'candidat-{numero}.png')
        planche.save(candidats[-1])
    assert len(candidats) == 2
    assert T.decoupe('colisee', candidats) == 0

    racine = depot / colisee['installRoot']
    manifeste = json.loads((racine / 'manifest.json').read_text(encoding='utf-8'))
    assert len(manifeste['textures']) == len(colisee['cells'])
    assert [f['file'] for f in manifeste['sheets']] == ['planche-1.png', 'planche-2.png']
    sable = manifeste['textures']['scene/coliseum/sand']
    assert (sable['size'], sable['anchor'], sable['sheet']) == ([68, 42], [34, 0], 1)
    assert Image.open(racine / 'sand.png').size == (68, 42)
    assert T.verifier('colisee') == 0

    # une texture retouchée à la main se voit
    retouche = np.asarray(Image.open(racine / 'pillar.png')).copy()
    retouche[50, 30] = [255, 0, 255, 255]
    Image.fromarray(retouche).save(racine / 'pillar.png')
    assert T.verifier('colisee') == 1


def test_un_sol_troue_est_refuse(colisee):
    _, _, planches = _planches(colisee, trou='sand-blood')
    recues = [T.mettre_au_format(colisee, p) for p in planches]
    _, _, erreurs, _ = T.decouper(colisee, recues)
    assert any(e.startswith('scene/coliseum/sand-blood : le sol couvre') for e in erreurs)


def test_il_faut_toutes_les_planches(colisee):
    _, _, planches = _planches(colisee)
    with pytest.raises(T.DispositionError, match='1 planche'):
        T.decouper(colisee, [T.mettre_au_format(colisee, planches[0])])


def test_une_planche_sans_transparence_est_refusee(colisee):
    _, _, planches = _planches(colisee)
    with pytest.raises(T.DispositionError, match='fond transparent'):
        T.mettre_au_format(colisee, planches[0].convert('RGB'))
