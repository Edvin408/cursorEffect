#! /bin/sh
gcc main.c -o cursorEffects -lX11 -lXrender -lXcomposite -lXext -lXi -lm
./cursorEffects
