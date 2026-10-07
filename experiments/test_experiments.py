"""Integration checks against the real compiled worker (about 400 short runs)."""
import argparse
from concurrent.futures import ThreadPoolExecutor
import itertools
import json
from pathlib import Path
import subprocess
import tempfile

from run_sweep import execute, PROBLEMS
from analyze import summarize


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--exe', type=Path, required=True)
    a = parser.parse_args()
    exe = a.exe.resolve()
    jobs = [('rastrigin', 128, m, s, f, c, 0, 12345)
            for m, s, f, c in itertools.product(range(2), range(8), range(6), range(4))]
    jobs += [(p, n, 1, 0, 5, 2, 0, 12345) for p, n in itertools.product(PROBLEMS, [128, 256, 512])]
    with ThreadPoolExecutor(max_workers=32) as pool:
        runs = list(pool.map(lambda job: execute(exe, job, 12, 5, 60), jobs))
    for r in runs:
        assert [t[0] for t in r['trace']] == [1, 5, 10, 12]
        assert all(b[2] <= a[2] + 1e-10 for a, b in zip(r['trace'], r['trace'][1:])), r
    lookup = {(r['mutation'], r['selection'], r['flatten'], r['crossover']): r for r in runs[:384]}
    for m, s, c in itertools.product(range(2), range(8), range(4)):
        # none and normalized must be behaviorally equivalent under identical seeds.
        assert lookup[m, s, 3, c]['solution'] == lookup[m, s, 5, c]['solution']
        if s != 0:
            for f in range(6):
                assert lookup[m, s, f, c]['solution'] == lookup[m, s, 5, c]['solution']
    first = execute(exe, jobs[0], 200, 20, 60)
    second = execute(exe, jobs[0], 200, 20, 60)
    assert first['objective'] == second['objective'] and first['solution'] == second['solution']
    assert [(t[0], t[2]) for t in first['trace']] == [(t[0], t[2]) for t in second['trace']]
    # Known sample SD: sqrt(2), never population SD (1).
    synthetic = [dict(first, objective=x, target_generation=0) for x in [1., 3.]]
    row = summarize({('rastrigin', 128, 0, 0, 0, 0): synthetic}, 2)[0]
    assert abs(row['objective_sd'] - 2**.5) < 1e-12 and row['target_success_rate'] == 0
    assert row['successful_target_seconds_median'] is None
    for arguments in [[], ['langermann', '128', '12', '12345', '0', '0', '0', '0', '5'],
                      ['rastrigin', '129', '12', '12345', '0', '0', '0', '0', '5'],
                      ['rastrigin', '128', '12', '0', '0', '0', '0', '0', '5']]:
        assert subprocess.run([str(exe), *arguments], capture_output=True).returncode != 0
    # A real interrupted/resumed output must skip successes and reject a changed design.
    with tempfile.TemporaryDirectory() as folder:
        command = ['python', str(Path(__file__).with_name('run_sweep.py')), '--exe', str(exe),
                   '--output', folder, '--problems', 'rastrigin', '--populations', '128',
                   '--mutation', '1', '--selection', '0', '--flatten', '5', '--crossover', '2',
                   '--repeats', '2', '--generations', '12', '--workers', '2']
        subprocess.run(command, check=True, capture_output=True)
        paths = sorted((Path(folder) / 'runs').glob('*.json'))
        original = paths[0].read_bytes()
        paths[1].write_text('interrupted output')
        subprocess.run(command, check=True, capture_output=True)
        assert paths[0].read_bytes() == original
        json.loads(paths[1].read_text())
        resumed = subprocess.run(command, check=True, capture_output=True, text=True)
        assert '2 valid runs already saved; 0 pending' in resumed.stdout
        assert subprocess.run(command + ['--generations', '13'], capture_output=True).returncode != 0
    print(f'PASS: {len(runs)} method/domain/population runs; exact budgets; monotonic traces; '
          'repeatability; flatten equivalence; sample SD; argument rejection; corrupt-run repair and safe resume.')


if __name__ == '__main__':
    main()
