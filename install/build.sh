#! /bin/sh
mkdir -p ~/.config/cursorEffect
if [ ! -f ~/.config/cursorEffect/config.h ]; then
    wget -P ~/.config/cursorEffect https://raw.githubusercontent.com/Edvin408/cursorEffect/refs/heads/main/config.h
fi
gcc /opt/cursorEffect/main.c -o /tmp/cursorEffects -lX11 -lXrender -lXcomposite -lXext -lXi -lm
/tmp/cursorEffects &
