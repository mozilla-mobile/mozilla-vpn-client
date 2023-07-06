#
# This script fetches all remote branches
# And filteres out branches that have a github release connected to that. 
# Draft releases will count too. 
# It Writes a key=value expression
# Where value: JSONArray<version> 
# to STDOUT, for easy use in Github workflows
# use: ./.github/scripts/get_unreleased_branches.ps1  >> $env:GITHUB_OUTPUT

git fetch
$REMOTE_RELEASE_BRANCHES = $(git branch -r).Split([Environment]::NewLine) `
                | ForEach-Object{ $_.Trim() } `
                | Where-Object {$_.startsWith('origin/releases/')} `
                | ForEach-Object{ $_.Replace("origin/releases/","") } `
                    

# Output of gh releases
# v2.15.1 Latest  v2.15.1 2023-06-28T21:38:04Z
# v2.15.1 Latest  v2.15.1 2023-06-28T21:38:04Z
# v2.15.0         v2.15.0 2023-05-30T16:15:58Z
# v2.14.1         v2.14.1 2023-03-30T16:09:25Z
$releases_on_github  = @()
$(gh release list).Split([Environment]::NewLine) | ForEach-Object{ 
    # $_ is now one line "v2.15.1 Latest  v2.15.1 2023-06-28T21:38:04Z"
    $tag = $_.Split([char]9)[0]
    $releases_on_github += $tag.Replace("v","")
} `
# Now we have 2 Lists. 

$UNRELEASED_BRANCHES = $REMOTE_RELEASE_BRANCHES | Where-Object { !($releases_on_github -contains $_) } 

# If we only have one Branch this will be a string, so let's json pack it. 
# If not this is a list and we can just use ConvertTo-Json
if ( $UNRELEASED_BRANCHES.GetType() -Eq [string] ){
    $json_value = "[ $UNRELEASED_BRANCHES ]"
}else{
    $json_value = ConvertTo-Json $UNRELEASED_BRANCHES -Compress 
}



Write-Output "branches=$json_value"
