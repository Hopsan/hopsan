#Get list of all png files in grahpics folder
$names = Get-ChildItem graphics -Filter "*.png" -Name | Out-String

#Remove space at end of string
$names = $names.trim()

#Replace line breaks with spaces and prepend path to each filename
$names = $names.replace("`r`n"," graphics/")

#Prepend path to first filename
$names = "graphics/$names"

#Replace string in user doxyfile
cat Doxyfile | %{$_ -replace "HTML_EXTRA_FILES       = ","HTML_EXTRA_FILES       = $names"} | Out-File -Encoding "UTF8" Doxyfile_tmp
mv -Force Doxyfile_tmp Doxyfile

#Replace string in full doxyfile
cat Doxyfile_full | %{$_ -replace "HTML_EXTRA_FILES       = ","HTML_EXTRA_FILES       = $names"} | Out-File -Encoding "UTF8" Doxyfile_tmp
mv -Force Doxyfile_tmp Doxyfile_full