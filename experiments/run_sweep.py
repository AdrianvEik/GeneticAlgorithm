"""Bounded process sweep; Python standard library only. See README.md."""
import argparse
import concurrent.futures as cf
import hashlib
import itertools
import json
import math
import os
from pathlib import Path
import platform
import random
import signal
import subprocess
import sys
import time
from datetime import datetime, timezone

PROBLEMS = ['styblinski_tang', 'ackley', 'griewank', 'levy', 'rastrigin', 'schwefel']
METHODS = {
    'mutation': ['bit', 'gene'],
    'selection': ['roulette', 'tournament', 'rank', 'rank_space', 'boltzmann', 'pairwise', 'boltzmann_strict', 'boltzmann_relaxed'],
    'flatten': ['linear', 'exponential', 'logarithmic', 'normalized', 'sigmoid', 'none'],
    'crossover': ['single_point', 'two_point', 'uniform', 'complete'],
}


def atomic_json(path, data):
    temp = path.with_suffix('.tmp')
    temp.write_text(json.dumps(data, allow_nan=False, separators=(',', ':')), encoding='utf-8')
    temp.replace(path)


def validate_result(d, job, generations):
    problem, population, mutation, selection, flatten, crossover, repeat, seed = job
    expected = dict(problem=problem, population=population, mutation=mutation, selection=selection,
                    flatten=flatten, crossover=crossover, seed=seed, genes=32, generations=generations)
    if any(d.get(k) != v for k, v in expected.items()):
        raise ValueError('Worker metadata differs from scheduled run')
    for k in ['objective', 'reference', 'gap_per_gene', 'compute_seconds', 'wall_seconds', 'generations_per_second']:
        if not math.isfinite(d[k]):
            raise ValueError(f'Non-finite {k}')
    if d['compute_seconds'] <= 0 or d['wall_seconds'] < d['compute_seconds'] or len(d['solution']) != 32:
        raise ValueError('Invalid timing or solution')
    if not all(math.isfinite(x) for x in d['solution']):
        raise ValueError('Non-finite solution')
    if d['evaluations'] != generations * (population - 3):
        raise ValueError('Unexpected evaluation count')
    if not d['trace'] or d['trace'][-1][0] != generations:
        raise ValueError('Missing final trace sample')
    d['repeat'] = repeat
    return d


def run_id(job):
    p, n, m, s, f, c, rep, seed = job
    return f'{p}_p{n}_m{m}_s{s}_f{f}_c{c}_r{rep:03d}_seed{seed}'


def execute(exe, job, generations, interval, timeout):
    p, n, m, s, f, c, rep, seed = job
    command = [str(exe), p, str(n), str(generations), str(seed), str(m), str(s), str(f), str(c), str(interval)]
    start = time.perf_counter()
    result = subprocess.run(command, capture_output=True, text=True, timeout=timeout)
    if result.returncode:
        raise RuntimeError(f'exit={result.returncode}\n{result.stderr[-4000:]}\n{result.stdout[-1000:]}')
    d = validate_result(json.loads(result.stdout), job, generations)
    d['process_seconds'] = time.perf_counter() - start
    return d


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--exe', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--profile', choices=['full', 'smoke'], default='full')
    parser.add_argument('--workers', type=int, default=32)
    parser.add_argument('--repeats', type=int, default=16)
    parser.add_argument('--generations', type=int)
    parser.add_argument('--seed', type=int, default=20261007)
    parser.add_argument('--trace-interval', type=int, default=100)
    parser.add_argument('--hours', type=float, default=0, help='Stop launching new runs after this many hours; 0 completes matrix')
    parser.add_argument('--timeout', type=float, default=900, help='Per-run timeout in seconds')
    parser.add_argument('--problems', nargs='+', choices=PROBLEMS)
    parser.add_argument('--populations', nargs='+', type=int, choices=[128, 256, 512])
    for k, names in METHODS.items():
        parser.add_argument('--' + k, nargs='+', type=int, choices=range(len(names)))
    parser.add_argument('--dry-run', action='store_true')
    a = parser.parse_args()
    if not 1 <= a.workers <= 32 or a.repeats < 2 or a.timeout <= 0 or a.hours < 0:
        parser.error('workers must be 1..32, repeats >=2, timeout >0, hours >=0')
    generations = a.generations if a.generations is not None else (500 if a.profile == 'smoke' else 10000)
    if not 2 <= generations <= 100000000 or a.trace_interval < 1 or not 1 <= a.seed <= 2**32 - a.repeats:
        parser.error('Invalid generation budget, trace interval, or seed range')
    smoke = a.profile == 'smoke'
    axes = [a.problems or (['styblinski_tang', 'rastrigin'] if smoke else PROBLEMS),
            a.populations or ([128] if smoke else [128, 256, 512]),
            a.mutation or [0, 1], a.selection or ([0, 2] if smoke else list(range(8))),
            a.flatten or ([1, 5] if smoke else list(range(6))),
            a.crossover or ([2] if smoke else list(range(4)))]
    axes = [list(dict.fromkeys(x)) for x in axes]
    jobs = [(*combo, rep, a.seed + rep) for combo in itertools.product(*axes) for rep in range(a.repeats)]
    random.Random(a.seed).shuffle(jobs)
    print(f'{len(jobs) // a.repeats:,} combinations x {a.repeats} repeats = {len(jobs):,} runs; '
          f'{generations:,} generations/run; {a.workers} concurrent processes', flush=True)
    print('Fixed 32 genes; 1 solver/run; inline fitness; explicit seeds; all objectives minimized.', flush=True)
    if a.dry_run:
        return 0
    exe = a.exe.resolve(strict=True)
    output = a.output.resolve()
    output.mkdir(parents=True, exist_ok=True)
    # Windows releases this file lock even after a crash. The file may remain.
    import msvcrt
    lock = (output / 'sweep.lock').open('a+b')
    if lock.tell() == 0:
        lock.write(b'0'); lock.flush()
    lock.seek(0)
    try:
        msvcrt.locking(lock.fileno(), msvcrt.LK_NBLCK, 1)
    except OSError:
        parser.error('Another sweep is using this output directory')
    manifest_path = output / 'manifest.json'
    design = dict(schema=1, axes=axes, repeats=a.repeats, generations=generations,
                  seed=a.seed, trace_interval=a.trace_interval, workers=a.workers,
                  exe_sha256=hashlib.sha256(exe.read_bytes()).hexdigest(),
                  controller_sha256=hashlib.sha256(Path(__file__).read_bytes()).hexdigest())
    if manifest_path.exists():
        manifest = json.loads(manifest_path.read_text())
        if manifest['design'] != design:
            parser.error('Resume design/build mismatch: use a new output directory or restore the original settings/binary')
    else:
        root = Path(__file__).resolve().parents[1]
        def git(*args):
            return subprocess.run(['git', '-C', str(root), *args], capture_output=True, text=True).stdout
        (output / 'source.diff').write_text(git('diff', 'HEAD'), encoding='utf-8')
        import zipfile
        with zipfile.ZipFile(output / 'source.zip', 'w', zipfile.ZIP_DEFLATED) as archive:
            for folder in ['src', 'experiments']:
                for path in sorted((root / folder).rglob('*')):
                    if path.is_file() and path.suffix in ['.c', '.h', '.py', '.ps1', '.md', '.txt']:
                        archive.write(path, path.relative_to(root))
            archive.write(root / 'CMakeLists.txt', 'CMakeLists.txt')
        manifest = dict(design=design, created_utc=datetime.now(timezone.utc).isoformat(),
                        platform=platform.platform(), processor=platform.processor(),
                        logical_cpus=os.cpu_count(), python=sys.version, executable=str(exe),
                        git_commit=git('rev-parse', 'HEAD').strip(), git_status=git('status', '--short'),
                        method_names=METHODS)
        atomic_json(manifest_path, manifest)
    runs = output / 'runs'; runs.mkdir(exist_ok=True)
    errors = output / 'errors'; errors.mkdir(exist_ok=True)
    pending = []
    skipped = 0
    for job in jobs:
        path = runs / (run_id(job) + '.json')
        try:
            validate_result(json.loads(path.read_text()), job, generations)
            skipped += 1
        except (OSError, ValueError, KeyError, TypeError):
            pending.append(job)
    print(f'Resume: {skipped:,} valid runs already saved; {len(pending):,} pending. Output: {output}', flush=True)
    stop = False
    def request_stop(*_):
        nonlocal stop
        stop = True
        print('Stopping new launches; saving active runs before exit (bounded by --timeout).', flush=True)
    signal.signal(signal.SIGINT, request_stop)
    started = time.perf_counter()
    done = failed = 0
    iterator = iter(pending)
    exhausted = False
    last_print = started
    with cf.ThreadPoolExecutor(max_workers=a.workers) as pool:
        active = {}
        while active or not exhausted:
            if a.hours and time.perf_counter() - started >= a.hours * 3600:
                stop = True
            while not stop and not exhausted and len(active) < a.workers:
                job = next(iterator, None)
                if job is None:
                    exhausted = True
                    break
                active[pool.submit(execute, exe, job, generations, a.trace_interval, a.timeout)] = job
            if stop:
                exhausted = True
            completed, _ = cf.wait(active, timeout=1, return_when=cf.FIRST_COMPLETED)
            for future in completed:
                job = active.pop(future)
                try:
                    atomic_json(runs / (run_id(job) + '.json'), future.result())
                    (errors / (run_id(job) + '.txt')).unlink(missing_ok=True)
                    done += 1
                except Exception as e:
                    failed += 1
                    (errors / (run_id(job) + '.txt')).write_text(str(e), encoding='utf-8')
                    print(f'FAILED {run_id(job)}: {e}', flush=True)
            now = time.perf_counter()
            if now - last_print >= 10 or (exhausted and not active):
                elapsed = now - started
                rate = (done + failed) / elapsed if elapsed else 0
                remaining = len(pending) - done - failed
                eta = remaining / rate / 3600 if rate else float('nan')
                print(f'{skipped + done:,}/{len(jobs):,} saved | {failed} failed | {len(active)} active | '
                      f'{rate * 60:.1f} runs/min | remaining ETA {eta:.2f} h', flush=True)
                last_print = now
    elapsed = time.perf_counter() - started
    session = dict(started_utc=datetime.now(timezone.utc).isoformat(), elapsed_seconds=elapsed,
                   completed=done, failed=failed, previous=skipped, expected=len(jobs), stopped=stop,
                   workers=a.workers)
    with (output / 'sessions.jsonl').open('a', encoding='utf-8') as stream:
        stream.write(json.dumps(session) + '\n')
    lock.close()
    print('Sweep complete.' if skipped + done == len(jobs) else 'Partial sweep saved; rerun the same command to resume.')
    return 1 if failed else 0


if __name__ == '__main__':
    raise SystemExit(main())
