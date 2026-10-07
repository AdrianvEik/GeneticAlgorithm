"""Summarize saved runs; optionally create publication-friendly PNG/PDF figures."""
import argparse
from collections import defaultdict
import csv
import json
import math
from pathlib import Path
import statistics as stats

from run_sweep import METHODS, run_id, validate_result

KEYS = ['problem', 'population', 'mutation', 'selection', 'flatten', 'crossover']


def label(row):
    return '/'.join(METHODS[k][int(row[k])] for k in KEYS[2:])


def write_csv(path, rows):
    if not rows:
        return
    with path.open('w', newline='', encoding='utf-8') as stream:
        writer = csv.DictWriter(stream, fieldnames=list(rows[0]))
        writer.writeheader(); writer.writerows(rows)


def summarize(groups, repeats):
    summary = []
    for key, runs in sorted(groups.items()):
        row = dict(zip(KEYS, key))
        row.update(n=len(runs), complete=len(runs) == repeats, methods=label(row))
        for metric in ['objective', 'gap_per_gene', 'compute_seconds', 'wall_seconds', 'generations_per_second']:
            values = [r[metric] for r in runs]
            row[metric + '_mean'] = stats.mean(values)
            row[metric + '_sd'] = stats.stdev(values) if len(values) > 1 else None
            row[metric + '_median'] = stats.median(values)
        successes = [r for r in runs if r['target_generation'] > 0]
        row['target_successes'] = len(successes)
        row['target_success_rate'] = len(successes) / len(runs)
        # Unsuccessful runs are censored, never treated as zero-time successes.
        row['successful_target_seconds_median'] = stats.median(r['target_compute_seconds'] for r in successes) if successes else None
        row['objective_min'] = min(r['objective'] for r in runs)
        row['objective_max'] = max(r['objective'] for r in runs)
        summary.append(row)
    return summary


def plots(output, summary, groups, manifest):
    import numpy as np
    import matplotlib
    matplotlib.use('Agg')
    import matplotlib.pyplot as plt
    plt.rcParams.update({'font.size': 10, 'axes.spines.top': False, 'axes.spines.right': False,
                         'figure.facecolor': '#f5f7fb', 'axes.facecolor': 'white',
                         'savefig.facecolor': '#f5f7fb', 'axes.titleweight': 'bold'})
    folder = output / 'figures'; folder.mkdir(exist_ok=True)
    complete = [s for s in summary if s['complete']]
    if not complete:
        print('No complete combinations yet; plots wait for all repeats in at least one combination.')
        return
    for problem, population in sorted({(r['problem'], r['population']) for r in complete}):
        rows = [r for r in complete if (r['problem'], r['population']) == (problem, population)]
        ranked = sorted(rows, key=lambda r: r['gap_per_gene_mean'])
        top = ranked[:8]
        fig, axes = plt.subplots(2, 2, figsize=(16, 11), layout='constrained')
        ax = axes[0, 0]
        y = np.arange(len(top))
        ax.errorbar([r['gap_per_gene_mean'] for r in top], y,
                    xerr=[r['gap_per_gene_sd'] for r in top], fmt='o', color='#2059a6', capsize=4)
        ax.set_yticks(y, [r['methods'].replace('/', ' / ') for r in top], fontsize=8)
        ax.invert_yaxis(); ax.set_xlabel('Objective gap per gene (lower is better)')
        ax.set_title('Best combinations: mean and sample SD')
        ax.grid(axis='x', alpha=.2)

        ax = axes[0, 1]
        for mutation in sorted({r['mutation'] for r in rows}):
            selected = [r for r in rows if r['mutation'] == mutation]
            ax.scatter([r['generations_per_second_mean'] for r in selected],
                       [max(r['gap_per_gene_mean'], 1e-12) for r in selected],
                       label=METHODS['mutation'][mutation], alpha=.7, s=35)
        ax.set_yscale('log'); ax.set_xlabel('Mean generations / compute second (higher is better)')
        ax.set_ylabel('Mean objective gap / gene, log scale'); ax.legend(title='Mutation')
        ax.set_title('Speed versus solution quality'); ax.grid(alpha=.2)

        # Distinct curves; equivalent flattening controls need not dominate this panel.
        chosen = []
        signatures = set()
        for row in ranked:
            signature = (row['mutation'], row['selection'], row['crossover'],
                         row['flatten'] if row['selection'] == 0 else -1)
            if signature not in signatures:
                chosen.append(row); signatures.add(signature)
            if len(chosen) == 4:
                break
        ax = axes[1, 0]
        for index, row in enumerate(chosen):
            records = groups[tuple(row[k] for k in KEYS)]
            generations = np.array([t[0] for t in records[0]['trace']])
            values = np.array([[(t[2] - r['reference']) / r['genes'] for t in r['trace']] for r in records])
            mean = values.mean(axis=0); sd = values.std(axis=0, ddof=1)
            color = f'C{index}'
            ax.plot(generations, np.maximum(mean, 1e-12), label=f'{index + 1}: {row["methods"]}', color=color)
            ax.fill_between(generations, np.maximum(mean - sd, 1e-12), np.maximum(mean + sd, 1e-12), color=color, alpha=.13)
        ax.set_yscale('log'); ax.set_xlabel('Completed generations'); ax.set_ylabel('Objective gap / gene, log scale')
        ax.set_title('Convergence: mean and sample SD across seeds')
        ax.legend(fontsize=7, loc='upper right'); ax.grid(alpha=.2)
        upper = ax.secondary_xaxis('top', functions=(lambda x: x * (population - 3), lambda x: x / (population - 3)))
        upper.set_xlabel('Objective evaluations')

        ax = axes[1, 1]
        names, active, wall = [], [], []
        for i, row in enumerate(chosen):
            names.append(str(i + 1)); active.append(row['compute_seconds_mean']); wall.append(row['wall_seconds_mean'])
        positions = np.arange(len(chosen))
        ax.bar(positions - .18, active, width=.36, label='Generation compute', color='#2059a6')
        ax.bar(positions + .18, wall, width=.36, label='Full GA call', color='#df8b37')
        ax.set_xticks(positions, names); ax.set_xlabel('Combination number from convergence legend')
        ax.set_ylabel('Mean seconds / run'); ax.set_title('Compute versus end-to-end latency'); ax.legend()
        ax.grid(axis='y', alpha=.2)
        n = manifest['design']['repeats']; budget = manifest['design']['generations']
        fig.suptitle(f'{problem.replace("_", " ").title()} | 32 genes | population {population}\n'
                     f'{budget:,} generations, {n} seeds per combination, {manifest["design"]["workers"]} concurrent processes', fontsize=17)
        fig.supxlabel('Descriptive results at fixed settings. SD describes seed variability, not a confidence interval. '
                      'Generation timing excludes observation, logging and startup.', fontsize=9)
        for extension in ['png', 'pdf']:
            fig.savefig(folder / f'{problem}_p{population}.{extension}', dpi=170)
        plt.close(fig)

    # Factor summaries average complete cells equally, with between-cell variability.
    # Flattening is shown only for roulette, because other selectors ignore it.
    for problem in sorted({r['problem'] for r in complete}):
        fig, axes = plt.subplots(2, 2, figsize=(14, 9), layout='constrained')
        for ax, factor in zip(axes.flat, KEYS[2:]):
            for pop in sorted({r['population'] for r in complete}):
                rows = [r for r in complete if r['problem'] == problem and r['population'] == pop
                        and (factor != 'flatten' or r['selection'] == 0)]
                ids = sorted({r[factor] for r in rows})
                means = [stats.mean(r['gap_per_gene_mean'] for r in rows if r[factor] == i) for i in ids]
                ax.plot(ids, means, 'o-', label=f'Population {pop}')
                ax.set_xticks(ids, [METHODS[factor][i] for i in ids], rotation=25, ha='right')
            ax.set_title(f'{factor.title()}' + (' (roulette only)' if factor == 'flatten' else ''))
            ax.set_ylabel('Mean objective gap / gene'); ax.grid(alpha=.2); ax.legend(fontsize=8)
        fig.suptitle(f'{problem.replace("_", " ").title()}: descriptive factor averages', fontsize=17)
        fig.supxlabel('Averages over the other tested factors; interactions and incomplete sweeps can confound these comparisons. '
                      'These settings are not tuned separately for each method.', fontsize=9)
        for extension in ['png', 'pdf']:
            fig.savefig(folder / f'{problem}_factors.{extension}', dpi=170)
        plt.close(fig)
    print(f'Figures: {folder}')


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('output', type=Path)
    parser.add_argument('--plots', action='store_true')
    a = parser.parse_args()
    manifest = json.loads((a.output / 'manifest.json').read_text())
    design = manifest['design']
    groups = defaultdict(list)
    raw = []
    for path in sorted((a.output / 'runs').glob('*.json')):
        r = json.loads(path.read_text())
        job = tuple(r[k] for k in KEYS) + (r['repeat'], r['seed'])
        validate_result(r, job, design['generations'])
        if path.stem != run_id(job) or not 0 <= r['repeat'] < design['repeats'] or r['seed'] != design['seed'] + r['repeat']:
            raise ValueError(f'Unexpected run identity: {path}')
        if any(value not in axis for value, axis in zip(job[:6], design['axes'])):
            raise ValueError(f'Run is outside manifest design: {path}')
        groups[tuple(r[k] for k in KEYS)].append(r)
        raw.append({k: v for k, v in r.items() if k not in ['solution', 'trace']})
    summary = summarize(groups, design['repeats'])
    report = a.output / 'report'; report.mkdir(exist_ok=True)
    write_csv(report / 'runs.csv', raw)
    write_csv(report / 'summary.csv', summary)
    expected = math.prod(len(axis) for axis in design['axes']) * design['repeats']
    complete = [r for r in summary if r['complete']]
    print(f'\nSaved {len(raw):,}/{expected:,} runs; {len(complete):,} complete combinations; '
          f'{len(summary) - len(complete)} partial combinations.')
    print('Best complete combination per problem/population (lower objective is better):')
    print(f'{"Problem":17s} {"Pop":>4s} {"n":>3s} {"Mean objective":>15s} {"Sample SD":>12s} {"Gen/s":>11s} {"Target hits":>12s}')
    for problem, pop in sorted({(r['problem'], r['population']) for r in complete}):
        best = min((r for r in complete if (r['problem'], r['population']) == (problem, pop)), key=lambda r: r['objective_mean'])
        print(f'{problem:17s} {pop:4d} {best["n"]:3d} {best["objective_mean"]:15.7g} '
              f'{best["objective_sd"]:12.5g} {best["generations_per_second_mean"]:11.1f} '
              f'{best["target_successes"]:>8d}/{best["n"]}')
        print(f'  {best["methods"]}')
    sessions_path = a.output / 'sessions.jsonl'
    if sessions_path.exists():
        sessions = [json.loads(line) for line in sessions_path.read_text().splitlines()]
        elapsed = sum(s['elapsed_seconds'] for s in sessions)
        successful = sum(s['completed'] for s in sessions)
        if elapsed > 0:
            print(f'Whole-sweep throughput: {successful / elapsed:.3f} saved runs/s across recorded sessions '
                  '(includes failures, startup and scheduling).')
    print(f'CSV tables: {report}')
    if a.plots:
        plots(a.output, summary, groups, manifest)


if __name__ == '__main__':
    main()
