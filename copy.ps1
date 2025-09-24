cp Snake.g3a D:/Snake.g3a
$driveEject = New-Object -comObject Shell.Application
$driveEject.Namespace(17).ParseName("D:\").InvokeVerb("Eject")
wsl