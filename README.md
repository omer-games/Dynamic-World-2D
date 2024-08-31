<p align="center">
  <img src="http://ForTheBadge.com/images/badges/made-with-c++.svg">
</p>

# Dynamic World 2D

Dynamic World 2D is a C++ game that creates an ever-changing environment. As the player moves in different directions, the world dynamically generates new layers. Players can interact with the world by placing and breaking blocks, and even create moving spikes.

## How it Works

In Dynamic World 2D, the game world changes based on the player's movement. Each time the player moves left or right, a new layer is generated, and it's different each time even if the player retraces their steps. The game emphasizes minimal library usage, relying on just three essential libraries to deliver its dynamic environment and interactive features.

## Features

- Dynamic world generation based on player movement.
- Interaction capabilities to place and break blocks.
- Ability to create moving spikes.

## How to Use

1. **Clone the Repository:**

    ```bash
    git clone https://github.com/omer-games/Dynamic-World-2D.git
    ```

2. **Build the Game:**

    Navigate to the project directory and run:

    ```bash
    make
    ```

3. **Run the Game:**

    ```bash
    ./DynamicWorld2D
    ```

## Keys

- **W**: Jump
- **A**: Move left
- **D**: Move right
- **X**: Place block
- **P**: Create moving spike
- **M**: Break block

## Libraries

The game uses the following built-in libraries:

- `<iostream>`
- `<thread>`
- `<Windows.h>`

## Configuration

The game's configuration options are available in the `config.txt` file. Here you can adjust various gameplay settings to customize your experience.

## License

See the `LICENSE` file for licensing information.

## Developer

Omer-Games
