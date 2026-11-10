import csv
import io
from collections import defaultdict
from pathlib import Path
from statistics import median

import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
import numpy as np
from matplotlib import font_manager
from matplotlib.transforms import Bbox
from PIL import Image

INK = '#000000'
SERIES = ['#2a78d6', '#1baf7a', '#eb6834']

for d in ['/usr/share/texmf/fonts/opentype/public/libertine',
          '/usr/share/texlive/texmf-dist/fonts/opentype/public/libertine',
          'D:/Program Files/texlive/2024/texmf-dist/fonts/opentype/public/libertine']:
    for f in Path(d).glob('LinLibertine_R*.otf') if Path(d).is_dir() else []:
        font_manager.fontManager.addfont(str(f))

plt.rcParams.update({
    'font.family': 'serif', 'font.serif': ['Linux Libertine O', 'Linux Libertine', 'STIXGeneral'],
    'font.size': 8, 'pdf.fonttype': 42,
    'axes.edgecolor': INK, 'axes.labelcolor': INK, 'text.color': INK,
    'xtick.color': INK, 'ytick.color': INK,
    'axes.linewidth': 0.4, 'xtick.major.width': 0.4, 'ytick.major.width': 0.4,
    'axes.spines.top': False, 'axes.spines.right': False,
    'axes.grid': True, 'axes.axisbelow': True, 'axes.grid.axis': 'y',
    'grid.color': '#e4e3df', 'grid.linewidth': 0.5,
})

DATA = Path('plots/data')
OUT = Path('plots/figures')

CHANGES = {
    'base': 'course code', 'opt1': 'row bounds', 'opt2': 'flat grid', 'opt3': 'multi-topple',
    'opt4': 'bytes, SIMD', 'opt5': 'symmetry', 'opt6': 'AVX2', 'opt7': 'OpenMP',
}


def read(name):
    return list(csv.DictReader((DATA / name).read_text().splitlines()))


def save(fig, name):
    # crop the white space above and below the ink, keep the width
    w, h = fig.get_size_inches()
    buf = io.BytesIO()
    fig.savefig(buf, format='png', dpi=600)
    rows = np.where((np.asarray(Image.open(buf).convert('L')) < 250).any(axis=1))[0]
    top, bottom = h - rows[0] / 600, h - (rows[-1] + 1) / 600
    fig.savefig(OUT / name, bbox_inches=Bbox([[0, max(bottom - 0.01, 0)], [w, min(top + 0.01, h)]]))


def times():
    runs = defaultdict(list)
    for row in read('times.csv'):
        if row['n'] == '1001':
            runs[row['commit'], int(row['threads'])].append(float(row['seconds']))
    return {key: median(vals) for key, vals in runs.items()}


def commits_figure(secs):
    names = [c for c in CHANGES if (c, 1) in secs]
    ys = [secs[c, 1] for c in names]
    fig, ax = plt.subplots(figsize=(3.33, 1.9), layout='constrained')
    pos = list(range(len(names)))[::-1]
    shades = [plt.get_cmap('Blues')(0.3 + 0.65 * k / (len(ys) - 1)) for k in range(len(ys))]
    ax.barh(pos, ys, 0.68, color=shades, edgecolor='white', linewidth=0.5)
    for y, t in zip(pos, ys):
        ax.annotate(f'{t:.3g} s  ({ys[0] / t:.3g}×)', (t, y), xytext=(3, 0),
                    textcoords='offset points', ha='left', va='center', fontsize=7)
    ax.set_xscale('log')
    ax.set_xlim(4, 1300)
    ax.set_yticks(pos, [f'{c}  {CHANGES[c]}' for c in names])
    ax.tick_params(axis='y', length=0)
    ax.grid(axis='y', visible=False)
    ax.grid(axis='x', visible=True)
    ax.minorticks_off()
    ax.set_xticks([10, 100, 1000], ['10', '100', '1000'])
    ax.set_xlabel('Seconds on one thread (log scale)')
    save(fig, 'commits.pdf')


def steps_threads_figure(secs):
    fig, (a, b) = plt.subplots(1, 2, figsize=(7, 1.85), layout='constrained',
                               gridspec_kw={'width_ratios': [1, 1.1]})

    rows = read('steps.csv')
    kinds = list(dict.fromkeys(r['stabilizer'] for r in rows))
    width = 0.8 / len(kinds)
    for i, kind in enumerate(kinds):
        ys = [int(r['steps']) / 1000 for r in rows if r['stabilizer'] == kind]
        xs = [p + (i - (len(kinds) - 1) / 2) * width for p in range(len(ys))]
        bars = a.bar(xs, ys, width, color=SERIES[i], edgecolor='white', linewidth=0.5, label=kind)
        a.bar_label(bars, fmt='%.0fk', padding=1.5, fontsize=7)
    a.set_xticks([0, 1], ['Pass 1: stab(6)', 'Pass 2: stab(6 − stab(6))'])
    a.tick_params(axis='x', length=0)
    a.set_ylabel('Sweeps or steps (thousands)')
    a.set_ylim(0, 225)
    a.legend(loc='upper right', frameon=False, handlelength=1, borderaxespad=0)

    ts = sorted(t for c, t in secs if c == 'opt7')
    total = [secs['opt7', t] for t in ts]
    b.plot(ts, [total[0] / t for t in ts], color=INK, linewidth=0.6, linestyle=(0, (3, 2)), zorder=1)
    b.plot(ts, total, color=SERIES[0], marker='o', markersize=3, linewidth=1.1)
    b.annotate('ideal', (32, total[0] / 32), xytext=(-6, -1), textcoords='offset points',
               ha='right', va='center', fontsize=7)
    best = ts[total.index(min(total))]
    for t, y in zip(ts, total):
        if t in (1, best, 32):
            b.annotate(f'{y:.2f} s', (t, y), xytext=(0, 4), textcoords='offset points',
                       ha='center', va='bottom', fontsize=7)
    b.set_xscale('log', base=2)
    b.set_yscale('log')
    b.set_xticks(ts, [str(t) for t in ts])
    b.set_yticks([0.25, 0.5, 1, 2, 4, 8], ['0.25', '0.5', '1', '2', '4', '8'])
    b.minorticks_off()
    b.set_ylim(0.18, 16)
    b.grid(axis='x', visible=False)
    b.set_xlabel('Threads')
    b.set_ylabel('Seconds, opt7 (log scale)')

    for k, ax in enumerate((a, b)):
        ax.annotate(f'({chr(97 + k)})', (0, 1), xycoords='axes fraction', xytext=(-2, 4),
                    textcoords='offset points', ha='right', va='bottom', weight='bold')
    save(fig, 'steps_threads.pdf')


if __name__ == '__main__':
    OUT.mkdir(parents=True, exist_ok=True)
    secs = times()
    commits_figure(secs)
    steps_threads_figure(secs)
