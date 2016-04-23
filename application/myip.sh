#!/bin/bash
printf  "LAN "
hostname --ip-address
printf  "WAN "
curl -s http://whatismijnip.nl |cut -d " " -f 5