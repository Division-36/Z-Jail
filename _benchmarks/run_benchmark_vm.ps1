#Requires -RunAsAdministrator
<#
  run_benchmark_vm.ps1  --  one-shot, hands-off Z-Jail benchmark.

  WHAT IT DOES (all by itself, once you start it in an ADMIN PowerShell):
    1. installs Multipass (a Hyper-V-backed Linux VM manager) if it is missing;
    2. launches a small Ubuntu LTS VM;
    3. runs the full benchmark inside it (builds z_jail, bwrap, nsjail, measures
       latency + peak RSS, runs the 18-scenario test suite);
    4. copies the results back to Windows next to this script;
    5. tells you exactly where REPORT.md is.

  HOW TO RUN:
    - Right-click "Windows PowerShell" -> "Run as administrator"
    - Then paste this ONE line:
        powershell -ExecutionPolicy Bypass -File ".\_benchmarks\run_benchmark_vm.ps1"

  To reclaim the ~3 GB afterwards:  multipass delete zjail-bench; multipass purge
#>

$ErrorActionPreference = 'Stop'
$VM         = 'zjail-bench'
$LocalScript= Join-Path $PSScriptRoot 'full_native_benchmark.sh'
$OutDir     = Join-Path $PSScriptRoot 'vm_results'
$VmOut      = '/home/ubuntu/zjail-bench-out'

function Say($m){ Write-Host "==> $m" -ForegroundColor Cyan }
function Refresh-Path {
    $env:Path = [Environment]::GetEnvironmentVariable('Path','Machine') + ';' +
                [Environment]::GetEnvironmentVariable('Path','User')
}

if (-not (Test-Path $LocalScript)) {
    throw "Cannot find $LocalScript -- run this from the repo's _benchmarks folder."
}

# ---- 1. Multipass -----------------------------------------------------------
if (-not (Get-Command multipass -ErrorAction SilentlyContinue)) {
    Say 'installing Multipass via winget (first time only)...'
    winget install -e --id Canonical.Multipass `
        --accept-package-agreements --accept-source-agreements
    Refresh-Path
}
if (-not (Get-Command multipass -ErrorAction SilentlyContinue)) {
    Write-Host ''
    Write-Host 'Multipass was installed but is not on PATH yet.' -ForegroundColor Yellow
    Write-Host 'Close this window, open a NEW admin PowerShell, and re-run the same command.' -ForegroundColor Yellow
    Write-Host '(If it still fails, a one-time reboot finalizes the Hyper-V networking.)' -ForegroundColor Yellow
    exit 1
}
Say ("multipass ready: " + (multipass version | Select-Object -First 1))

# ---- 2. VM ------------------------------------------------------------------
$exists = (multipass list --format csv 2>$null | Select-String "^$VM,")
if (-not $exists) {
    Say 'launching Ubuntu LTS VM (downloads ~400 MB the first time)...'
    try {
        multipass launch --name $VM --cpus 4 --memory 4G --disk 12G
    } catch {
        Write-Host ''
        Write-Host 'VM launch failed. This is almost always because Multipass needs a' -ForegroundColor Yellow
        Write-Host 'one-time reboot right after install. Please REBOOT, then re-run.'      -ForegroundColor Yellow
        exit 1
    }
} else {
    Say "VM '$VM' already exists; reusing it."
    multipass start $VM 2>$null | Out-Null
}

# ---- 3. run the benchmark inside the VM -------------------------------------
Say 'copying benchmark script into the VM...'
multipass transfer $LocalScript "${VM}:/tmp/bench.sh"

Say 'running the benchmark inside the VM (several minutes; installs deps, builds, measures)...'
multipass exec $VM -- sudo bash -c "cd /home/ubuntu && OUTDIR=$VmOut bash /tmp/bench.sh; chmod -R a+rX $VmOut"

# ---- 4. pull results back to Windows ----------------------------------------
Say 'copying results back to Windows...'
if (Test-Path $OutDir) { Remove-Item -Recurse -Force $OutDir }
New-Item -ItemType Directory -Force -Path $OutDir | Out-Null
multipass transfer -r "${VM}:$VmOut" $OutDir

# ---- 5. done ----------------------------------------------------------------
$report = Get-ChildItem -Recurse -Path $OutDir -Filter 'REPORT.md' -ErrorAction SilentlyContinue |
          Select-Object -First 1
Write-Host ''
Write-Host '==================================================================' -ForegroundColor Green
Write-Host ' DONE.' -ForegroundColor Green
if ($report) { Write-Host (" Report : " + $report.FullName) -ForegroundColor Green }
Write-Host  (" Folder : " + $OutDir) -ForegroundColor Green
Write-Host '==================================================================' -ForegroundColor Green
Write-Host ''
Write-Host 'Environment recorded inside REPORT.md (kernel, CPU, virt=hyperv).'
Write-Host 'To reclaim the ~3 GB now:  multipass delete zjail-bench; multipass purge'
