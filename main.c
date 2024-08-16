#include "ldata.h"
#include <math.h>
#include <raylib.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define WIDTH 16
#define HEIGHT 16

#define TARGET_FPS 60

#define S_TOP 100
#define SWIDTH 800
#define SHEIGHT 800

#define MINE_CHANCE 0.20

typedef struct {
    bool opened;
    bool flagged;
    i32 mines; // -1 = mine, 0 = no mine, 1+ = no mine but x mines nearby
} Square;

f64 random_f64(void) { return (f64)rand() / (f64)RAND_MAX; }

void init_square(Square* square) {
    *square = (Square){
        .opened = false,
        // .opened = true,
        .flagged = false,
        .mines = random_f64() < MINE_CHANCE ? -1 : 0
    };
}

void add_mine_nums(Square** squares, ci64 x, ci64 y) {
    Square *square = &squares[y][x];
        if (square->mines == -1)
        return;
    i64 count = 0;
    const bool check_left = x > 0;
    const bool check_right = x < WIDTH - 1;
    const bool check_up = y > 0;
    const bool check_down = y < HEIGHT - 1;
    if (check_left  && check_up   && squares[y - 1][x - 1].mines == -1)
        count++;
    if (check_right && check_up   && squares[y - 1][x + 1].mines == -1)
        count++;
    if (check_left  && check_down && squares[y + 1][x - 1].mines == -1)
        count++;
    if (check_right && check_down && squares[y + 1][x + 1].mines == -1)
        count++;
    if (check_left  && squares[y][x - 1].mines == -1)
        count++;
    if (check_right && squares[y][x + 1].mines == -1)
        count++;
    if (check_up    && squares[y - 1][x].mines == -1)
        count++;
    if (check_down  && squares[y + 1][x].mines == -1)
        count++;
    square->mines = count;
}

Square** init_squares(void) {
    Square** new_squares = malloc(HEIGHT * sizeof(Square*));

  for (i64 y = 0; y < HEIGHT; ++y)
      new_squares[y] = malloc(WIDTH * sizeof(Square));

  for (i64 y = 0; y < HEIGHT; ++y) {
      for (i64 x = 0; x < WIDTH; ++x)
          init_square(&new_squares[y][x]);
  }

  for (i64 y = 0; y < HEIGHT; ++y) {
      for (i64 x = 0; x < WIDTH; ++x)
          add_mine_nums(new_squares, x, y);
  }

  return new_squares;
}

void free_squares(Square** squares) {
    for (i64 y = 0; y < HEIGHT; ++y)
        free(squares[y]);
    free(squares);
}

void show_square(const Square** squares, ci64 x, ci64 y) {
    cu64 square_width = SWIDTH / WIDTH;
    cu64 square_height = SHEIGHT / HEIGHT;

    if (!squares[y][x].opened) {
        DrawRectangle(x * square_width, y * square_height + S_TOP, square_width, square_height, LIGHTGRAY);
        DrawRectangleLines(x * square_width, y * square_height + S_TOP, square_width, square_height, BLACK);
        if (squares[y][x].flagged) {
            DrawText("F", x * square_width + 5, y * square_height + 5 + S_TOP, 20, GREEN);
        }
        return;
      }
    DrawRectangleLines(x * square_width, y * square_height + S_TOP, square_width, square_height, BLACK);
    if (squares[y][x].mines != -1) {
        char text[20];
        sprintf(text, "%d", squares[y][x].mines);
        DrawText(text, x * square_width + 5, y * square_height + 5 + S_TOP, 20, BLACK);
        return;
    }
    DrawText("X", x * square_width + 5, y * square_height + 5 + S_TOP, 20, RED);
}

void show_squares(const Square** squares) {
    for (i64 y = 0; y < HEIGHT; ++y) {
        for (i64 x = 0; x < WIDTH; ++x)
            show_square(squares, x, y);
    }
}

void reveal_squares(Square** squares, ci64 x, ci64 y) {
    if (x < 0 || x >= WIDTH || y < 0 || y >= HEIGHT)
        return;

    if (squares[y][x].opened || squares[y][x].flagged)
        return;

    if (squares[y][x].mines == -1 || squares[y][x].mines > 0) {
        (&squares[y][x])->opened = true;
        return;
    };

    if (squares[y][x].mines == 0) {
        (&squares[y][x])->opened = true;
        if (x > 0)
            reveal_squares(squares, x - 1, y);
        if (x < WIDTH - 1)
            reveal_squares(squares, x + 1, y);
        if (y > 0)
            reveal_squares(squares, x, y - 1);
        if (y < HEIGHT - 1)
            reveal_squares(squares, x, y + 1);
        if (x > 0 && y > 0)
            reveal_squares(squares, x - 1, y - 1);
        if (x < WIDTH - 1 && y > 0)
            reveal_squares(squares, x + 1, y - 1);
        if (x > 0 && y < HEIGHT - 1)
            reveal_squares(squares, x - 1, y + 1);
        if (x < WIDTH && y < HEIGHT - 1)
            reveal_squares(squares, x + 1, y + 1);
    }
}

void flag_square(Square** squares, ci64 x, ci64 y) {
    if (x < 0 || x >= WIDTH || y < 0 || y >= HEIGHT)
        return;
    bool *flagged = &(&squares[y][x])->flagged;
    *flagged = !*flagged;
}

void mouse_coords(i64 *coords) {
    f64 mouse_x = (f64)GetMouseX();
    f64 mouse_y = (f64)GetMouseY() - S_TOP;
    coords[0] = (i64)floor(WIDTH * mouse_x / SWIDTH);
    coords[1] = (i64)floor(HEIGHT * mouse_y / SHEIGHT);
}

void show_clock(cf64 time_offset) {
    i64 t = (i64)floor(GetTime() - time_offset);
    i64 m = t / 60;
    i64 s = t % 60;
    char string[20];
    sprintf(string, "%li : %li", m, s);
    DrawText(string, 100, 50, 30, BLUE);
}

bool game_lost(const Square** squares) {
    for (i64 y = 0; y < HEIGHT; ++y) {
        for (i64 x = 0; x < WIDTH; ++x) {
            Square s = squares[y][x];
            if (s.mines == -1 && s.opened)
                return true;
        }
    }
    return false;
}

bool game_won(const Square** squares) {
    for (i64 y = 0; y < HEIGHT; ++y) {
        for (i64 x = 0; x < WIDTH; ++x) {
            Square s = squares[y][x];
            if (s.flagged && s.mines == -1)
                continue;
            if (s.opened && s.mines >= 0)
                continue;
            return false;
        }
    }
    return true;
}

bool show_message(void) {
    return !IsKeyReleased(KEY_ENTER) && !WindowShouldClose();
}

i32 main(void) {
    srand(time(NULL));
    // srand(42); // for testing

    InitWindow(SWIDTH, SHEIGHT + S_TOP, "minesweeper");
    SetTargetFPS(TARGET_FPS);

    f64 time_offset = 0.0f;

    bool is_game_won = false;
    while (!WindowShouldClose() && !is_game_won) {
        time_offset = GetTime();

        Square** squares = init_squares();
        i64 coords[2];

        while (!game_lost((const Square**)squares) && !WindowShouldClose()) {
            mouse_coords(coords);

            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
                reveal_squares(squares, coords[0], coords[1]);

            if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT))
                flag_square(squares, coords[0], coords[1]);

            BeginDrawing();
            ClearBackground(WHITE);

            show_clock(time_offset);
            show_squares((const Square**)squares);

            EndDrawing();

            if (game_won((const Square**)squares))
                is_game_won = true;
        }

        free_squares(squares);

        if (!is_game_won) {
            while (show_message()) {
                BeginDrawing();

                ClearBackground(WHITE);
                DrawText("You lose. Press ENTER to continue", 100, 50, 30, RED);

                EndDrawing();
            }
            continue;
        }

        while (show_message()) {
            BeginDrawing();

            ClearBackground(WHITE);
            DrawText("You win! Press ENTER to continue", 100, 50, 30, GREEN);

            EndDrawing();
        }
    }

    return EXIT_SUCCESS;
}
