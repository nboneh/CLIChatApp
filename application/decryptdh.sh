openssl enc -d -aes-128-ctr -in file2.bin -out tempouttext -kfile savefiles/dhkey.pem -iv 00000000000000000000000000000000 >/dev/null 2>&1
