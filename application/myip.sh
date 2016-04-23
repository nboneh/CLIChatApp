#!/bin/bash
printf  "LAN "
if [  "$(uname)" == "Darwin"  ]
	then
    # Mac
    ifconfig en1 | awk '{ print $2}' | grep -E -o "([0-9]{1,3}[\.]){3}[0-9]{1,3}"
else  
    # Linux
    echo $(ip a s|sed -ne '/127.0.0.1/!{s/^[ \t]*inet[ \t]*\([0-9.]\+\)\/.*$/\1/p}')
fi
printf  "WAN "
curl -s http://whatismijnip.nl |cut -d " " -f 5