[CmdletBinding()]
param(
    [ValidateSet('full','smoke')][string]$Profile = 'full',
    [ValidateRange(1,32)][int]$Workers = 32,
    [ValidateRange(2,1000000)][int]$Repeats = 16,
    [int]$Generations = 0,
    [string]$Output = '',
    [double]$Hours = 0,
    [string]$Python = 'python',
    [switch]$SkipBuild,
    [switch]$DryRun,
    [switch]$Plots
)
$ErrorActionPreference = 'Stop'
$repo = Split-Path $PSScriptRoot -Parent
if (-not $Output) { $Output = Join-Path $repo "build\experiments-$Profile-results" }
if (-not $SkipBuild -and -not $DryRun) { & (Join-Path $PSScriptRoot 'Build-Benchmark.ps1') }
$worker = Join-Path $repo 'build\experiments-release\ga_bench.exe'
$arguments = @((Join-Path $PSScriptRoot 'run_sweep.py'), '--exe', $worker, '--output', $Output,
    '--profile', $Profile, '--workers', $Workers, '--repeats', $Repeats, '--hours', $Hours)
if ($Generations -gt 0) { $arguments += @('--generations', $Generations) }
if ($DryRun) { $arguments += '--dry-run' }
& $Python @arguments
$runExit = $LASTEXITCODE
if (-not $DryRun -and (Test-Path -LiteralPath (Join-Path $Output 'manifest.json'))) {
    $reportArguments = @((Join-Path $PSScriptRoot 'analyze.py'), $Output)
    if ($Plots) { $reportArguments += '--plots' }
    & $Python @reportArguments
    if ($LASTEXITCODE -ne 0) { throw 'Analysis failed.' }
}
if ($runExit -ne 0) { throw "Sweep exited with code $runExit; inspect the errors directory." }
