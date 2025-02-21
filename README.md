Simple Retro-Style Games for ESP32/ESP8266/Arduino Mega & SH110X/SSD1306 OLED Displays
This project features three classic games optimized for microcontrollers: Snake, Chain Reaction (Unstable), and an Artillery game inspired by Worms and Pocket Tanks.

You can simulate the project on Wokwi for Arduino Mega and SSD1306 here:
🔗 [Wokwi Simulation](https://wokwi.com/projects/423497131298398209) (May take some time to compile)

Included Games
🐍 Snake
A classic arcade game where the player controls a growing snake that moves across the screen. The goal is to eat food to grow longer while avoiding collisions with itself and the screen boundaries.

💥 Chain Reaction
A strategy-based multiplayer game where players take turns placing orbs on a grid.

Orbs "explode" when their count exceeds the number of adjacent cells, spreading to neighboring spaces.
The goal is to capture the entire board by triggering chain reactions that convert opponents’ orbs into your own.
A cell can hold up to its neighbor count minus one before exploding:
- Middle of the grid → Max 3 orbs (4 neighbors).
- Corner cells → Max 1 orb (2 neighbors).
- Edge cells → Max 2 orbs (3 neighbors).

The game continues until only one kind of orb is left on board.

🎯 Artillery Game (Inspired by Pocket Tanks & Worms)
A 2-player turn-based strategy game where players control tanks and aim to damage their opponent by adjusting angle and power while considering wind and terrain.

Gameplay
- Each player gets 9 turns to deal as much damage as possible.
- The player who accumulates the most points wins.
- Before the match, players select 9 weapons from 3 randomly presented choices per slot.
- The game randomly selects one of three maps, and Player 1 starts.
- Each round, players can move, aim, select a weapon, and fire.
- Wind strength and direction (0-15) change every round.

Weapons
- Bomb – Standard weapon (15 damage) with a small explosion.
- TNT – Stronger bomb (25 damage) with a larger blast radius.
- Stream – Multi-hit projectile that doesn't affect terrain but is more affected by wind.
- Pepper – Fires small pellets with a wide spread, lightly scratching the terrain.
- Mole – A powerful explosive that can burrow underground before detonating.
- Jetpack – Instead of firing, launches the player to a new position.
- Wall – Upon impact, the projectile creates a solid pillar, altering the battlefield.


