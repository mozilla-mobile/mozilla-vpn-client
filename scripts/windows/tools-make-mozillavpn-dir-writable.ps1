$acl = Get-Acl "C:\Program Files\Mozilla\Mozilla VPN"
$permission = "Everyone","FullControl","Allow"
$rule = New-Object System.Security.AccessControl.FileSystemAccessRule $permission
$acl.SetAccessRule($rule)
$acl | Set-Acl "C:\Program Files\Mozilla\Mozilla VPN"