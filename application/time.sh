if [  "$(uname)" == "Darwin"  ]
	then
    # Mac
   	gdate +%s%N | cut -b1-13
else  
    # Linux
    date +%s%N | cut -b1-13
fi