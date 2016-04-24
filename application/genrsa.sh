openssl genrsa -out savefiles/my.key 1024 >/dev/null 2>&1
openssl rsa -in savefiles/my.key -text -noout >/dev/null 2>&1

openssl rsa -in savefiles/my.key -pubout -out savefiles/my.key.pub >/dev/null 2>&1
openssl rsa -in savefiles/my.key.pub -pubin -text -noout >/dev/null 2>&1