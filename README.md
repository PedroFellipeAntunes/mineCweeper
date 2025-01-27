# MineCweeper

MineCweeper is a terminal-based Minesweeper game implemented in C, utilizing the `ncurses` library for an interactive gaming experience. Players navigate a grid, avoid hidden bombs, and aim to uncover all non-bomb spaces. The game also tracks and scores performance based on speed, accuracy, and difficulty.

<p align="center">
  <img src="IMAGE mineCweeper.png" width="450" alt="Application Interface">
</p>

## How to Play

- Start the game by running the compiled binary: `./minecweeper`
- Use arrow keys to navigate the grid.
- Press Enter to reveal a cell.
- If the cell contains a bomb, the game ends.
- If the cell is safe, it reveals the number of surrounding bombs.
- Flag potential bombs to mark them as dangerous.
- Win the game by uncovering all non-bomb cells.

## Compilation
To compile the game, ensure you have `gcc` and the `ncurses` library installed. Use the following command:
```bash
gcc -o minecweeper minecweeper.c -lncurses
