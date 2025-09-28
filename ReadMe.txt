Game Control:
	Keyboard Support:
		-F1 Draw debug rings and lines
		-F8 Force restart whole game
		-T Slow down whole game (0.1 delta time)
		-O Run one step of the game
		-P Pause whole game
		-I Generate new asteroids (12 maximum)
		-SPACEE Generate new bullets (20 maximum) or start game at attract mode
		-N Respawn player (only if player is dead)
		-E Accelerate players spaceship (only if player is alive)
		-S Rotate players spaceship counterclockwise (only if player is alive)
		-F Rotate players spaceship clockwise (only if player is alive)
		-ESC Switch from game mode to attract mode or shut down application in attract mode
	Xbox Controller Support:
		-RIGHT_STICK Control player spaceship orientation and accelerate 
		(0~1 -> 0%~100% MAX_ACCELERATION)
		-A Generate new bullets (20 maximum) or start game at attract mode
		-START Respawn player (only if player is dead)
Known bugs:
	-If player hold T slowdown the game and then fire bullet until the error message window show up, close the window and even release T, the game will still in a slowdown mode.
