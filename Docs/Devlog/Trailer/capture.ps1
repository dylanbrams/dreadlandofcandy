<#
  Records one gameplay clip from the packaged build: only the game window's client area (ddagrab cropped to it),
  never the rest of the desktop. gdigrab can't see the game's D3D frames (it records one stale frame), hence ddagrab.
  The game must stay focused and on top: it throttles to a few fps when unfocused, and anything covering the
  window would be recorded. Video only; music and voice go on in the edit.
  .\Docs\Devlog\Trailer\capture.ps1 -Name knight-fight -Seconds 40 -Skip 12 -Extra "-autoplay","-class=knight"
  Clips land in Docs\Devlog\GameFootage\ (git-ignored, shared with the devlog edit), at the window's size (1600x900).
#>
param(
  [Parameter(Mandatory)] [string]$Name,
  [int]$Seconds = 30,          # length recorded
  [int]$Skip = 10,             # seconds after the window appears before recording starts (loading, title)
  [string[]]$Extra = @(),
  [int]$Fps = 30,
  [string]$Map = "",           # e.g. '/Game/World/L_Sugarborough?game=/Script/MegaJam.JamTourGameMode' (tour camera)
  [switch]$Online             # leave the note backend on (improvise, proposals, director's notes call the server)
)
$ErrorActionPreference = "Stop"
$Root = (Resolve-Path "$PSScriptRoot\..\..\..").Path
$Exe = Join-Path $Root "Builds\Win64-Development\TheDreadLandOfCandy.exe"
$OutDir = Join-Path $Root "Docs\Devlog\GameFootage"
New-Item -ItemType Directory -Force $OutDir | Out-Null
$Out = Join-Path $OutDir "$Name.mp4"

Add-Type -Namespace W -Name U -MemberDefinition @'
[DllImport("user32.dll")] public static extern bool SetProcessDPIAware();
[DllImport("user32.dll")] public static extern bool SetForegroundWindow(IntPtr h);
[DllImport("user32.dll")] public static extern bool ShowWindow(IntPtr h, int c);
[DllImport("user32.dll")] public static extern void keybd_event(byte k, byte s, uint f, UIntPtr e);
[DllImport("user32.dll")] public static extern bool GetClientRect(IntPtr h, out RECT r);
[DllImport("user32.dll")] public static extern bool ClientToScreen(IntPtr h, ref POINT p);
public struct RECT { public int L, T, R, B; }
public struct POINT { public int X, Y; }
'@
[W.U]::SetProcessDPIAware() | Out-Null

$GameArgs = @($Map | Where-Object { $_ }) + @("-windowed", "-ResX=1600", "-ResY=900", "-WinX=100", "-WinY=60", "-nomusic") + $(if ($Online) { @() } else { @("-nonotes") }) + $Extra
Write-Host "Launching: $($GameArgs -join ' ')"
Start-Process -FilePath $Exe -ArgumentList $GameArgs | Out-Null

# The root exe is a shim; the real game process is MegaJam.
$Win = $null
for ($i = 0; $i -lt 90 -and -not $Win; $i++) {
  Start-Sleep -Seconds 1
  $Win = Get-Process | Where-Object { ($_.ProcessName -eq "MegaJam" -or $_.ProcessName -like "TheDreadLandOfCandy*") -and $_.MainWindowTitle -like "*PCD3D*" } | Select-Object -First 1
}
if (-not $Win) { throw "No game window after 90 s" }
$H = $Win.MainWindowHandle
[W.U]::keybd_event(0x12, 0, 0, [UIntPtr]::Zero); [W.U]::keybd_event(0x12, 0, 2, [UIntPtr]::Zero)
[W.U]::ShowWindow($H, 9) | Out-Null
$Focused = [W.U]::SetForegroundWindow($H)
Start-Sleep -Seconds $Skip
[W.U]::SetForegroundWindow($H) | Out-Null

$R = New-Object W.U+RECT; [W.U]::GetClientRect($H, [ref]$R) | Out-Null
$P = New-Object W.U+POINT; [W.U]::ClientToScreen($H, [ref]$P) | Out-Null
$Wd = ($R.R - $R.L) - (($R.R - $R.L) % 2); $Ht = ($R.B - $R.T) - (($R.B - $R.T) % 2)
if ($P.X -lt 0 -or $P.Y -lt 0) { throw "Game window is not on the primary monitor ($($P.X),$($P.Y))" }
Write-Host "Client area ${Wd}x${Ht} at $($P.X),$($P.Y), focused=$Focused; recording $Seconds s. Don't click away."

# Frames stay on the GPU (ddagrab -> nvenc): a CPU hwdownload stalls behind the game and drops to ~8 fps.
# Recorded at the window's own size; build_demo.py scales to 1080p.
& ffmpeg -hide_banner -loglevel warning -y -init_hw_device d3d11va `
  -filter_complex "ddagrab=output_idx=0:framerate=${Fps}:draw_mouse=0:offset_x=$($P.X):offset_y=$($P.Y):video_size=${Wd}x${Ht}" `
  -t $Seconds -c:v h264_nvenc -preset p4 -cq 19 -fps_mode cfr -r $Fps $Out

Get-Process | Where-Object { $_.ProcessName -eq "MegaJam" -or $_.ProcessName -like "TheDreadLandOfCandy*" } | Stop-Process -Force -Confirm:$false
Write-Host "Saved $Out"
