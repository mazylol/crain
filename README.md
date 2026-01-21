# crain
rain in the terminal using [ncurses](https://invisible-island.net/ncurses/) (rewrite/revamp of [humidify](https://github.com/mazylol/humidify))

## Install
1. `git clone https://github.com/mazylol/crain && cd crain`
3. `meson setup buildDir && cd buildDir`
4. `sudo ninja install`

## Features
- [x] Custom Character
- [ ] Speed
- [ ] Color
- [ ] Number of drops
- [ ] Gravity
- [ ] Splash
- [ ] Direction
- [ ] Angle

## Why?
It's cool. Also humidify was written in Go, and I did it in an awful horribly unoptomized way (every drop is it's own goroutine). I feel the need to right a wrong. It's about time I align myself with the suckless folks.
