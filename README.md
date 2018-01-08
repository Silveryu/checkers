# checkers

## Ruleset

Also called Spanish checkers. It is mainly played in Portugal and in some parts of South America and some Northern African countries.

- Flying kings; men cannot capture backwards
- 8x8 board size
- 12 pieces per side
- Double-corner or light square on player's near-right? Light square is on right, but double corner is on left, as play is on the light squares. (Play on the dark squares with dark square on right is Portuguese draughts.)
- White moves first
- A sequence must capture the maximum possible number of pieces, and the maximum possible number of kings from all such sequences.

## Development

```bash
$ make
$ ./bin/main
```

```bash
$ make footage
$ ./bin/main bin/footage.mp4
```

## TODO

- improve silverio's points with cornerSubPix
- inner-most loop should not set the same (x,y) pair twice
- try (1) sharpen (2) canny before findChessboardCorners
- introduce the notion of a commit when setting game state
