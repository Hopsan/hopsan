#/bin/bash
file $1 | grep ISO-8859
if [ $? -eq 0 ]; then
  echo -n "File: $1 has ISO-8859 encoding,  re-encoding to UTF8 .... "
  iconv --from-code=ISO-8859-1 --to-code=UTF-8 $1 > $1.utf8
  mv $1 $1.iso88591
  mv $1.utf8 $1
  echo "Done!"
fi