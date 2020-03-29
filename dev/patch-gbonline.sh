#!/bin/sh

echo "function autorun(){
mainCanvas = document.getElementById('mainCanvas');
initPlayer();
start(mainCanvas, base64_decode('$(base64 $1 | tr -d '\n')'));
}" > $2/js/autorun.js

sed 's|<script type="text/javascript" src="js/GameBoyIO.js"/>|&\n<script type="text/javascript" src="js/autorun.js"/>|g' -i $2/index.xhtml
sed 's/windowingInitialize();/&\nautorun();/g' -i $2/index.xhtml

#remove duplicate lines
sed '$!N; /^\(autorun();\|<script type="text\/javascript" src="js\/autorun.js"\/>\)\n\1$/!P; D'  -i $2/index.xhtml