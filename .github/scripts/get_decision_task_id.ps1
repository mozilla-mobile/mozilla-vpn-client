while (1){
    $(gh pr checks $env:PR_NUMBER).Split([Environment]::NewLine) | ForEach-Object{ 
        if($_.contains("Decision Task (pull-request)")){
            if($_.contains("pass")){
#                Write-Output "Task done!"
                $url = $_.Split([char]9)[-1]
                $task_ID += $url.Replace("https://firefox-ci-tc.services.mozilla.com/tasks/","")
#                Write-Output $task_ID
                Write-Output "taskID=$task_ID"
                break;
            }else{
#              Write-Output "Found task, but not done."
            }
        } else {
#         Write-Output "Not found task."
        }
        
    }
 #   Write-Output "Sleeping 30s before retry"
    Start-Sleep -Seconds 30 
}
