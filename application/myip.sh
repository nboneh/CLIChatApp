#!/bin/bash
printf  "LAN "
echo $(ip a s|sed -ne '/127.0.0.1/!{s/^[ \t]*inet[ \t]*\([0-9.]\+\)\/.*$/\1/p}')
printf  "WAN "
curl -s http://whatismijnip.nl |cut -d " " -f 5