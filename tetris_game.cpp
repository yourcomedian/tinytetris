#include <algorithm>
#include <cstdlib>
#include <cstring>
#include "tetris_game.h"

const int TetrisGame::initial_block_data[7][4] = {
    {431424, 598356, 431424, 598356},
    {427089, 615696, 427089, 615696},
    {348480, 348480, 348480, 348480},
    {599636, 431376, 598336, 432192},
    {411985, 610832, 415808, 595540},
    {247872, 799248, 247872, 799248},
    {614928, 399424, 615744, 428369}
};

TetrisGame::TetrisGame() : score_(0), game_over_(false), current_piece_type_(0), current_rotation_(0),tick_speed_control_(0) {
    srand(static_cast<unsigned int>(time(nullptr)));
    initialize_piece_definitions();
    memset(board_, 0, sizeof(board_));
}

TetrisGame::~TetrisGame () {

}

int TetrisGame::get_two_bit_value(int piece_raw_data, int bit_offset) const{
    return (piece_raw_data >> bit_offset) & 3;
}

void TetrisGame::initialize_piece_definitions() {
    piece_definitions_.resize(7); // 为7种方块类型分配空间
    for (int i = 0; i < 7; ++i) {
        piece_definitions_[i].resize(4); // 每种方块有4种旋转状态
        for (int j = 0; j < 4; ++j) {
            int raw_data = initial_block_data[i][j]; // 获取整数编码
            TetrominoShape current_shape;

            // 解码4个组成块的相对坐
            // 在整数编码中，每2位表示一个坐标值
            // 0-1位表示第一个块的y坐标，2-3位表示第一个块的x坐标，以此类推
            current_shape.blocks[0].y = get_two_bit_value(raw_data, 0);  // 第一个块的y坐标
            current_shape.blocks[0].x = get_two_bit_value(raw_data, 2);  // 第一个块的x坐标
            current_shape.blocks[1].y = get_two_bit_value(raw_data, 4);  // 第二个块的y坐标
            current_shape.blocks[1].x = get_two_bit_value(raw_data, 6);  // 第二个块的x坐标
            current_shape.blocks[2].y = get_two_bit_value(raw_data, 8);  // 第三个块的y坐标
            current_shape.blocks[2].x = get_two_bit_value(raw_data, 10); // 第三个块的x坐标
            current_shape.blocks[3].y = get_two_bit_value(raw_data, 12); // 第四个块的y坐标
            current_shape.blocks[3].x = get_two_bit_value(raw_data, 14); // 第四个块的x坐标

            // 解码方块边界框的宽度和高度
            // 在编码中，width-1和height-1分别存储在16-17位和18-19位
            // 所以我们需要加1才能得到实际的宽度和高度
            current_shape.width  = get_two_bit_value(raw_data, 16) + 1; // 方块宽度
            current_shape.height = get_two_bit_value(raw_data, 18) + 1; // 方块高度
            
            piece_definitions_[i][j] = current_shape; // 存储解码后的方块形状
        }
    }
}

void TetrisGame::start_new_game(){
    memset(board_, 0, sizeof(board_));
    score_ = 0;
    game_over_ = false;
    tick_speed_control_ = 0;
    spawn_new_piece();
}

const TetrominoShape& TetrisGame::get_shape_data(int piece_type, int rotation) const {
    return piece_definitions_[piece_type][rotation];
}

const TetrominoShape& TetrisGame::get_current_shape_data() const {
    return get_shape_data(current_piece_type_, current_rotation_);
}

void TetrisGame::place_or_remove_piece(Point pos, int piece_type, int rotation, int value) {
    const TetrominoShape& shape = get_shape_data(piece_type, rotation);
    for (int i = 0; i < 4; ++i) {
        int board_x = pos.x + shape.blocks[i].x;
        int board_y = pos.y + shape.blocks[i].y;
        if(board_x >= 0 && board_x < BOARD_WIDTH && board_y >= 0 && board_y < BOARD_HEIGHT) {
            board_[board_y][board_x] = value;
        }
    }
}


bool TetrisGame::check_collision(Point pos, int piece_type, int rotation) const {
    const TetrominoShape& shape = get_shape_data(piece_type, rotation);
    for (int i = 0; i < 4; ++i) {
        int board_x = pos.x + shape.blocks[i].x;
        int board_y = pos.y + shape.blocks[i].y;
        if (board_x < 0 || board_x >= BOARD_WIDTH || board_y < 0 || board_y >= BOARD_HEIGHT) {
            return true;
        }
        if (board_[board_y][board_x] != 0) {
            return true;
        }
    }
    return false;
}

void TetrisGame::spawn_new_piece() {
    current_piece_type_ = rand() % 7;
    current_rotation_ = rand() % 4;
    const TetrominoShape& shape = get_current_shape_data();
    current_piece_pos_.x = rand() % (BOARD_WIDTH - shape.width + 1);
    current_piece_pos_.y = 0;

    if (check_collision(current_piece_pos_, current_piece_type_, current_rotation_)) {
        game_over_ = true;
    } else {
        place_or_remove_piece(current_piece_pos_, current_piece_type_, current_rotation_, current_piece_type_+1);
    }
}

bool TetrisGame::move_left() {
    if (game_over_) return false;
    Point new_pos = current_piece_pos_;
    new_pos.x--;
    place_or_remove_piece(current_piece_pos_, current_piece_type_, current_rotation_, 0);
    bool collision = check_collision(new_pos, current_piece_type_, current_rotation_);
    if(!collision) {
        current_piece_pos_ = new_pos;
    }
    place_or_remove_piece(current_piece_pos_, current_piece_type_, current_rotation_, current_piece_type_+1);
    return !collision;
}


bool TetrisGame::move_right() {
    if (game_over_) return false;
    Point new_pos = current_piece_pos_;
    new_pos.x++;
    place_or_remove_piece(current_piece_pos_, current_piece_type_, current_rotation_, 0);
    bool collision = check_collision(new_pos, current_piece_type_, current_rotation_);
    if(!collision) {
        current_piece_pos_ = new_pos;
    }
    place_or_remove_piece(current_piece_pos_, current_piece_type_, current_rotation_, current_piece_type_+1);
    return !collision;
}

bool TetrisGame::rotate_piece() {
    if(game_over_) return false;
    int next_rotation = (current_rotation_ + 1) % piece_definitions_[current_piece_type_].size();
    place_or_remove_piece(current_piece_pos_, current_piece_type_, current_rotation_, 0);
    
    bool collision = check_collision(current_piece_pos_, current_piece_type_, next_rotation);
    Point test_pos = current_piece_pos_;

    if (collision) {
        test_pos.x--;
        if (!check_collision(test_pos, current_piece_type_, next_rotation)) {
            current_piece_pos_ = test_pos;
            collision = false;
        } else {
            test_pos.x += 2;
            if (!check_collision(test_pos, current_piece_type_, next_rotation)) {
                current_piece_pos_ = test_pos;
                collision = false;
            } else {
                test_pos = current_piece_pos_;
            }
        }
    }

    if (!collision) {
        current_rotation_ = next_rotation;
    }
    place_or_remove_piece(current_piece_pos_, current_piece_type_, current_rotation_, current_piece_type_+1);
    return !collision;
}


void TetrisGame::solidify_current_piece() {
    score_ += clear_full_lines();
    spawn_new_piece();
}

bool TetrisGame::game_tick() {
    if (game_over_) return false;

    Point new_pos = current_piece_pos_;
    new_pos.y++;
    place_or_remove_piece(current_piece_pos_, current_piece_type_, current_rotation_, 0);
    bool collision_below = check_collision(new_pos, current_piece_type_, current_rotation_);

    if(!collision_below) {
        current_piece_pos_ = new_pos;
        place_or_remove_piece(current_piece_pos_, current_piece_type_, current_rotation_, current_piece_type_+1);
    } else {

        place_or_remove_piece(current_piece_pos_, current_piece_type_, current_rotation_, current_piece_type_+1);
        solidify_current_piece();
    }
    return !game_over_;
}

void TetrisGame::drop_piece() {
    if (game_over_) return;
    place_or_remove_piece(current_piece_pos_, current_piece_type_, current_rotation_, 0);

    while(!check_collision(current_piece_pos_, current_piece_type_, current_rotation_)) {
        current_piece_pos_.y++;
    }
    current_piece_pos_.y--;

    place_or_remove_piece(current_piece_pos_, current_piece_type_, current_rotation_, current_piece_type_+1);
    solidify_current_piece();
}

int TetrisGame::clear_full_lines() {
    int lines_cleared = 0;

    for (int row = BOARD_HEIGHT - 1; row >= 0; --row) {
        bool line_full = true;

        for (int col = 0; col < BOARD_WIDTH; ++col) {
            if (board_[row][col] == 0) {
                line_full = false;
                break;
            }
        }
        if (line_full) {
            lines_cleared++;

            for (int k = row; k > 0; --k) {
                memcpy(board_[k], board_[k-1], BOARD_WIDTH * sizeof(int));
            }
            memset(board_[0], 0, BOARD_WIDTH * sizeof(int));
            row++;
        }
    } 

    if (lines_cleared > 0){
        if (lines_cleared == 1) score_ += 40;
        else if(lines_cleared == 2) score_ += 100;
        else if(lines_cleared == 3) score_ += 300;
        else if(lines_cleared == 4) score_ += 1200;
    }
    return lines_cleared;
}

const int* TetrisGame::get_board()  const {
    return &board_[0][0];
}

int TetrisGame::get_score() const {
    return score_;
}

bool TetrisGame::is_game_over() const {
    return game_over_;
}