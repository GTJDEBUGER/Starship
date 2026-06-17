# Starship

A top-down space shooter where players pilot a starship through crowded enemy swarms. Defeat foes to earn experience, unlock randomized roguelike upgrades, and survive long enough to escape the star zone!

> **Gameplay Showcase:** Watch the game in action [here](https://gaotianji.site/#portfolio/project13).

## Screenshot

<table style="width:100%">
  <tr>
    <td width="50%"><img src="https://github.com/user-attachments/assets/85a0f6b4-f30f-4553-919e-5aa25fe7a14a" alt="Gameplay Screenshot 1" width="100%"></td>
    <td width="50%"><img src="https://github.com/user-attachments/assets/dfb7a03e-0c6c-49b2-a348-0706e71196b0" alt="Gameplay Screenshot 2" width="100%"></td>
  </tr>
  <tr>
    <td width="50%"><img src="https://github.com/user-attachments/assets/926ab924-263e-41c2-99d9-7fa069921075" alt="Gameplay Screenshot 3" width="100%"></td>
    <td width="50%"><img src="https://github.com/user-attachments/assets/c30bb2ab-7790-499b-a99f-1ef91ce276fd" alt="Gameplay Screenshot 4" width="100%"></td>
  </tr>
</table>

## How to Play？

### Keyboard Controls

* <kbd>SPACE</kbd> — Fire bullets / Start game (from attract mode)
* <kbd>E</kbd> — Accelerate spaceship (only if player is alive)
* <kbd>S</kbd> — Rotate spaceship counterclockwise / Select left upgrade
* <kbd>F</kbd> — Rotate spaceship clockwise / Select right upgrade
* <kbd>ENTER</kbd> — Confirm upgrade selection
* <kbd>N</kbd> — Respawn player (only if player is dead)
* <kbd>ESC</kbd> — Toggle attract mode / Exit application
* <kbd>P</kbd> — Pause the entire game
* <kbd>T</kbd> — Slow motion mode (0.1x delta time)
* <kbd>O</kbd> — Step forward one frame (Debug)
* <kbd>I</kbd> — Force generate new asteroids (Debug)
* <kbd>K</kbd> — Kill all entities on screen (Cheat)
* <kbd>F1</kbd> — Toggle debug rendering (rings and lines)
* <kbd>F8</kbd> — Force restart the entire game

### Xbox Controller Support

* <kbd>Right Stick</kbd> — Control spaceship orientation and accelerate (0~1 input → 0%~100% max acceleration)
* <kbd>A</kbd> — Fire bullets / Start game
* <kbd>D-Pad Left</kbd> — Select left upgrade at upgrade board
* <kbd>D-Pad Right</kbd> — Select right upgrade at upgrade board
* <kbd>X</kbd> — Confirm upgrade selection
* <kbd>START</kbd> — Respawn player (only if player is dead)

## Build Instructions

> **Important Dependency Note:**
> This game is built on my [Custom Game Engine](https://github.com/GTJDEBUGER/GameEngine). You must pull the engine project first to build this game. 
> 
> Additionally, due to FMOD audio dependencies, **you must manually place `fmod64.dll` into the `Run/` directory after building**, otherwise the executable will throw a missing DLL error.

**Prerequisites:**
* Visual Studio 2022
* FMOD DLL

**Compilation Steps:**
1. Clone the [Game Engine](https://github.com/GTJDEBUGER/GameEngine) repository.
2. Ensure both the engine project and the Starship game project are located in the same parent directory.
3. Open the game project solution using **Visual Studio 2022**.
4. Set the build configuration to **Release / x64**.
5. Build the solution.
6. Download or locate `fmod64.dll` and place it directly into the game's `Run/` folder.
7. Navigate to the `Run/` directory and launch the compiled executable.
