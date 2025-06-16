#ifndef TETRIS_GAME_H
#define TETRIS_GAME_H

#include <vector>
#include <string>
#include <cstdlib>
#include <ctime>
#include <cstring>

const int BOARD_WIDTH = 10;
const int BOARD_HEIGHT = 20;

struct Point{
    int x;
    int y;
};

struct TetrominoShape{
    Point blocks[4];
    int width;
    int height;
};

class TetrisGame {
public:
    TetrisGame();
    ~TetrisGame();

    void start_new_game();
    bool move_left();
    bool move_right();
    bool rotate_piece();
    void drop_piece();
    bool game_tick();
    const int* get_board() const;
    int get_score() const;
    bool is_game_over() const;
private:
    static const int initial_block_data[7][4];
    std::vector<std::vector<TetrominoShape>> piece_definitions_;
    int board_[BOARD_HEIGHT][BOARD_WIDTH];
    int score_;
    bool game_over_;
    int current_piece_type_;
    int current_rotation_;
    Point current_piece_pos_;
    int tick_speed_control_;
    static const int FALL_SPEED_THERSHOLD = 30;
    void initialize_piece_definitions();
    int get_two_bit_value(int piece_raw_data, int bit_offset) const;
    void spawn_new_piece();
    bool check_collision(Point pos, int piece_type, int rotation) const;
    void place_or_remove_piece(Point pos, int piece_type, int rotation, int value);
    void solidify_current_piece();
    int clear_full_lines();
    const TetrominoShape& get_shape_data(int piece_type, int rotation) const;
    const TetrominoShape& get_current_shape_data() const;
};

#endif //TETRIS_GAME_H
