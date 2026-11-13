# Commandlet asset deletion can leave World package files behind without editor
# ticks. Remove only packages recorded by our cleanup, with two protection lists.
$ErrorActionPreference = 'Stop'
$terrainReviewRoot = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../../../Content/World/Review'))
$terrainCleanupPath = Join-Path $PSScriptRoot 'review_cleanup.json'
$terrainCleanup = Get-Content -LiteralPath $terrainCleanupPath -Raw | ConvertFrom-Json
$terrainTracked = Get-Content -LiteralPath (Join-Path $PSScriptRoot 'tracked_review_maps.json') -Raw | ConvertFrom-Json
$terrainCurrent = Get-Content -LiteralPath (Join-Path $PSScriptRoot 'review_maps.json') -Raw | ConvertFrom-Json
$terrainProtected = @($terrainTracked) + @($terrainCurrent.PSObject.Properties.Value)
$terrainRemovedFiles = @()
foreach ($terrainPackage in $terrainCleanup.removed) {
    if (-not $terrainPackage.StartsWith('/Game/World/Review/') -or $terrainPackage -in $terrainProtected) { throw "Protected or invalid package: $terrainPackage" }
    $terrainLeaf = ($terrainPackage -split '/')[-1]
    if ($terrainLeaf -notmatch '^L_[A-Za-z0-9]+$') { throw "Unexpected package name: $terrainLeaf" }
    $terrainFile = [IO.Path]::GetFullPath((Join-Path $terrainReviewRoot "$terrainLeaf.umap"))
    if ((Split-Path -Parent $terrainFile) -ne $terrainReviewRoot) { throw "Review package escaped workspace: $terrainFile" }
    if (Test-Path -LiteralPath $terrainFile) {
        Remove-Item -LiteralPath $terrainFile
        $terrainRemovedFiles += $terrainFile
    }
}
$terrainCleanup | Add-Member -NotePropertyName package_files_removed -NotePropertyValue $terrainRemovedFiles -Force
$terrainCleanup | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath $terrainCleanupPath
Write-Output "Removed $($terrainRemovedFiles.Count) scratch World packages."
