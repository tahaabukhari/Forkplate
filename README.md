# ForkPlate

ForkPlate is a creative desktop application that demonstrates process forking and simple graphical output using SDL2. Combining system-level process operations with a graphical user interface, ForkPlate showcases how you can visually monitor and interact with created process forks.

## Overview

ForkPlate displays a visual representation of parent and forked child processes on a custom “plate” using SDL2. In addition to spawning child processes—each represented by a movable fork image with a unique process ID—ForkPlate provides a debugging panel that logs system activities and displays a dynamically-updated process hierarchy (using the familiar pstree format). A dedicated xterm window is launched to continuously show the real-time process tree.

## Key Features

- **Graphical Interface:** Utilizes SDL2, SDL_Image, and SDL_TTF for rendering a custom window, textures, and text.
- **Process Forking:** Spawns child processes using fork() and tracks their PID along with custom names using prctl().
- **Dynamic Debug Panel:** Displays recent system messages and renders a process tree with parent and child process information.
- **External Process Monitoring:** Launches an xterm session to run `pstree` for live process hierarchy monitoring.
- **Interactive UI:** Provides a clickable button that, when pressed, creates a new forked process and visualizes it on the plate.
- **Randomized Fork Placement:** Each new fork image is positioned randomly on the main area to create an engaging visual layout.

## Getting Started

1. Ensure your system has SDL2, SDL_Image, and SDL_TTF installed.
2. Make sure you have the required assets (such as `plate_img.png` and `fork_img.png`) in the correct location.
3. Run the executable:
   ```bash
   ./forkplate
   ```

