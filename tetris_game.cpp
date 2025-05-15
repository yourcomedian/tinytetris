// tetris_game.cpp
// 俄罗斯方块游戏的C++核心实现
// 本文件包含TetrisGame类的所有实现，负责游戏的核心逻辑
#include "tetris_game.h"
#include <stdexcept> // 用于抛出std::out_of_range异常
#include <algorithm> // 用于std::fill, std::copy等算法函数

// 俄罗斯方块形状的原始整数编码
// 这些整数是从原始tinytetris.cpp中获取的，每个整数包含了一个方块的所有信息
// 包括4个组成块的位置和方块的边界框尺寸
const int val_x_shape = 431424;   // 实际解码为Z形 {(0,0),(1,0),(1,1),(2,1)}, 宽3, 高2 (原注释称I形)
const int val_y_shape = 598356;   // 实际解码为垂直S/Z形 {(1,0),(1,1),(0,1),(0,2)}, 宽2, 高3 (原注释称I形旋转)
const int val_r_shape = 427089;   // L形方块的编码
const int val_p_shape_orig = 615696; // L形方块旋转90度的编码
const int val_c_shape = 348480;   // O形方块的编码（四个旋转态相同）
const int val_px_shape = 247872;  // 实际解码为{(0,0),(0,0),(1,0),(2,0)} (注意:block[0]和block[1]坐标相同), 宽4, 高1 (原注释称J形)
const int val_py_shape = 799248;  // 实际解码为{(0,0),(0,1),(0,2),(3,0)}, 编码宽1, 高4 (注意:点(3,0)超出声明宽度1). (原注释称J形旋转)

// initial_block_data存储每种方块类型所有旋转状态的原始编码
// 7种基本方块类型，每种有4种旋转状态
const int TetrisGame::initial_block_data[7][4] = {
    {val_x_shape, val_y_shape, val_x_shape, val_y_shape}, // 方块0 (原注释称I形; val_x_shape实际为Z形, val_y_shape实际为垂直S/Z形): 长条，只有两种有效旋转状态
    {val_r_shape, val_p_shape_orig, val_r_shape, val_p_shape_orig}, // 方块1 (L形): L型，只有两种有效旋转状态
    {val_c_shape, val_c_shape, val_c_shape, val_c_shape}, // 方块2 (O形): 方块，所有旋转状态相同
    {599636, 431376, 598336, 432192},                     // 方块3 (S形; 注意: 部分旋转解码为非标准/问题形状): S型，有四种旋转状态
    {411985, 610832, 415808, 595540},                     // 方块4 (Z形; 注意: 部分旋转解码为非标准/问题形状或S形): Z型，有四种旋转状态
    {val_px_shape, val_py_shape, val_px_shape, val_py_shape}, // 方块5 (原注释称J形; val_px_shape解码为{(0,0),(0,0),(1,0),(2,0)}, val_py_shape解码为{(0,0),(0,1),(0,2),(3,0)} (编码宽1,高4;点(3,0)超出声明宽度)): 反L型，只有两种有效旋转状态
    {614928, 399424, 615744, 428369}                      // 方块6 (T形): T型，有四种旋转状态
};

// 构造函数：初始化游戏对象
TetrisGame::TetrisGame() : score_(0), game_over_(false), current_piece_type_(0), current_rotation_(0), tick_speed_control_(0) {
    srand(static_cast<unsigned int>(time(nullptr))); // 初始化随机数生成器的种子
    initialize_piece_definitions(); // 解码方块定义
    memset(board_, 0, sizeof(board_)); // 将整个棋盘初始化为0（空格）
}

// 析构函数：清理资源
TetrisGame::~TetrisGame() {
    // 在这个版本中，没有需要在析构函数中清理的动态资源
}

// 从方块形状的整数编码中提取2位的值
// piece_raw_data: 包含方块信息的整数
// bit_offset: 要提取的2位值在整数中的位置
// 返回值: 提取的2位值（0-3的整数）
int TetrisGame::get_two_bit_value(int piece_raw_data, int bit_offset) const {
    // 右移bit_offset位，然后与3（二进制：11）进行按位与操作，提取最低2位
    return (piece_raw_data >> bit_offset) & 3;
}

// 初始化所有方块形状定义
// 这个函数解码initial_block_data中的整数，将其转换为更容易使用的TetrominoShape结构
void TetrisGame::initialize_piece_definitions() {
    piece_definitions_.resize(7); // 为7种方块类型分配空间
    for (int i = 0; i < 7; ++i) {
        piece_definitions_[i].resize(4); // 每种方块有4种旋转状态
        for (int j = 0; j < 4; ++j) {
            int raw_data = initial_block_data[i][j]; // 获取整数编码
            TetrominoShape current_shape;

            // 解码4个组成块的相对坐标
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

// 开始新游戏
void TetrisGame::start_new_game() {
    memset(board_, 0, sizeof(board_)); // 清空棋盘，所有格子设为0（空）
    score_ = 0;        // 重置分数
    game_over_ = false; // 重置游戏状态
    tick_speed_control_ = 0; // 重置下落速度控制器
    spawn_new_piece();  // 生成第一个方块
}

// 获取特定类型和旋转状态的方块形状数据
const TetrominoShape& TetrisGame::get_shape_data(int piece_type, int rotation) const {
    return piece_definitions_[piece_type][rotation];
}

// 获取当前方块的形状数据
const TetrominoShape& TetrisGame::get_current_shape_data() const {
    return get_shape_data(current_piece_type_, current_rotation_);
}

// 在棋盘上放置或移除方块
// pos: 方块在棋盘上的位置
// piece_type: 方块类型
// rotation: 旋转状态
// value: 0表示移除，>0表示放置（值表示方块颜色）
void TetrisGame::place_or_remove_piece(Point pos, int piece_type, int rotation, int value) {
    const TetrominoShape& shape = get_shape_data(piece_type, rotation);
    for (int i = 0; i < 4; ++i) { // 遍历方块的4个组成块
        int board_x = pos.x + shape.blocks[i].x; // 计算在棋盘上的x坐标
        int board_y = pos.y + shape.blocks[i].y; // 计算在棋盘上的y坐标
        // 只有在棋盘范围内时才修改棋盘
        if (board_x >= 0 && board_x < BOARD_WIDTH && board_y >= 0 && board_y < BOARD_HEIGHT) {
            board_[board_y][board_x] = value; // 设置棋盘格子的值
        }
    }
}

// 检查方块在给定位置和旋转状态下是否会发生碰撞
// pos: 方块在棋盘上的位置
// piece_type: 方块类型
// rotation: 旋转状态
// 返回值: true表示会发生碰撞，false表示不会发生碰撞
bool TetrisGame::check_collision(Point pos, int piece_type, int rotation) const {
    const TetrominoShape& shape = get_shape_data(piece_type, rotation);
    for (int i = 0; i < 4; ++i) { // 遍历方块的4个组成块
        int board_x = pos.x + shape.blocks[i].x; // 计算在棋盘上的x坐标
        int board_y = pos.y + shape.blocks[i].y; // 计算在棋盘上的y坐标

        // 检查是否超出棋盘边界
        if (board_x < 0 || board_x >= BOARD_WIDTH || board_y < 0 || board_y >= BOARD_HEIGHT) {
            return true; // 与棋盘边界碰撞
        }
        // 检查是否与已有方块碰撞
        if (board_[board_y][board_x] != 0) {
            return true; // 与棋盘上已有的方块碰撞
        }
    }
    return false; // 没有发生碰撞
}

// 生成新的方块
void TetrisGame::spawn_new_piece() {
    current_piece_type_ = rand() % 7; // 随机选择一种方块类型 (0-6)
    current_rotation_ = rand() % 4;   // 随机选择一个旋转状态 (0-3)

    const TetrominoShape& shape = get_current_shape_data();
    
    // 随机选择一个水平位置，确保方块完全在棋盘内
    current_piece_pos_.x = rand() % (BOARD_WIDTH - shape.width + 1);
    current_piece_pos_.y = 0; // 方块总是从棋盘顶部开始下落

    // 检查新生成的方块是否与棋盘上已有方块发生碰撞
    if (check_collision(current_piece_pos_, current_piece_type_, current_rotation_)) {
        game_over_ = true; // 如果一开始就碰撞，说明游戏结束
    } else {
        // 将新方块放置在棋盘上
        place_or_remove_piece(current_piece_pos_, current_piece_type_, current_rotation_, current_piece_type_ + 1);
    }
}

// 将当前方块向左移动一格
bool TetrisGame::move_left() {
    if (game_over_) return false; // 如果游戏已结束，不执行任何操作

    Point new_pos = current_piece_pos_;
    new_pos.x--; // 尝试向左移动一格

    // 从棋盘上移除当前方块（先擦除再检查碰撞）
    place_or_remove_piece(current_piece_pos_, current_piece_type_, current_rotation_, 0);
    // 检查新位置是否会发生碰撞
    bool collision = check_collision(new_pos, current_piece_type_, current_rotation_);
    
    if (!collision) {
        // 如果没有碰撞，更新方块位置
        current_piece_pos_ = new_pos;
    }
    // 在棋盘上重新绘制方块（无论是否移动成功）
    place_or_remove_piece(current_piece_pos_, current_piece_type_, current_rotation_, current_piece_type_ + 1);
    return !collision; // 返回移动是否成功
}

// 将当前方块向右移动一格
bool TetrisGame::move_right() {
    if (game_over_) return false; // 如果游戏已结束，不执行任何操作

    Point new_pos = current_piece_pos_;
    new_pos.x++; // 尝试向右移动一格

    // 从棋盘上移除当前方块
    place_or_remove_piece(current_piece_pos_, current_piece_type_, current_rotation_, 0);
    // 检查新位置是否会发生碰撞
    bool collision = check_collision(new_pos, current_piece_type_, current_rotation_);

    if (!collision) {
        // 如果没有碰撞，更新方块位置
        current_piece_pos_ = new_pos;
    }
    // 在棋盘上重新绘制方块
    place_or_remove_piece(current_piece_pos_, current_piece_type_, current_rotation_, current_piece_type_ + 1);
    return !collision; // 返回移动是否成功
}

// 旋转当前方块
bool TetrisGame::rotate_piece() {
    if (game_over_) return false; // 如果游戏已结束，不执行任何操作

    // 计算下一个旋转状态（顺时针旋转）
    int next_rotation = (current_rotation_ + 1) % piece_definitions_[current_piece_type_].size();

    // 从棋盘上移除当前方块
    place_or_remove_piece(current_piece_pos_, current_piece_type_, current_rotation_, 0);
    
    // 检查旋转后是否会发生碰撞
    bool collision = check_collision(current_piece_pos_, current_piece_type_, next_rotation);
    Point test_pos = current_piece_pos_;

    // 墙壁反弹：如果旋转后与墙壁碰撞，尝试调整位置
    if (collision) {
        // 尝试向左移动一格
        test_pos.x--; 
        if (!check_collision(test_pos, current_piece_type_, next_rotation)) {
            current_piece_pos_ = test_pos;
            collision = false; // 旋转成功
        } else {
            // 尝试向右移动两格（从原始位置）
            test_pos.x += 2; 
             if (!check_collision(test_pos, current_piece_type_, next_rotation)) {
                current_piece_pos_ = test_pos;
                collision = false; // 旋转成功
            } else {
                // 如果向左向右都不行，可以尝试其他位置调整（如向上）
                // 或者更复杂的超级旋转系统（SRS）
                test_pos = current_piece_pos_; // 重置位置
            }
        }
    }

    if (!collision) {
        // 如果旋转不会发生碰撞，更新旋转状态
        current_rotation_ = next_rotation;
    }
    // 在棋盘上重新绘制方块
    place_or_remove_piece(current_piece_pos_, current_piece_type_, current_rotation_, current_piece_type_ + 1);
    return !collision; // 返回旋转是否成功
}

// 将当前方块固定在棋盘上
void TetrisGame::solidify_current_piece() {
    // 尝试清除满行并增加分数
    score_ += clear_full_lines(); 
    // 生成下一个方块
    spawn_new_piece(); 
}

// 游戏时钟：推进游戏一个时间单位
bool TetrisGame::game_tick() {
    if (game_over_) return false; // 如果游戏已结束，不执行任何操作

    Point new_pos = current_piece_pos_;
    new_pos.y++; // 尝试向下移动一格

    // 从棋盘上移除当前方块
    place_or_remove_piece(current_piece_pos_, current_piece_type_, current_rotation_, 0); 
    // 检查下方是否有碰撞
    bool collision_below = check_collision(new_pos, current_piece_type_, current_rotation_);

    if (!collision_below) {
        // 如果下方没有碰撞，更新方块位置
        current_piece_pos_ = new_pos;
        // 在新位置重新绘制方块
        place_or_remove_piece(current_piece_pos_, current_piece_type_, current_rotation_, current_piece_type_ + 1); 
    } else {
        // 如果下方有碰撞，将方块固定在当前位置
        place_or_remove_piece(current_piece_pos_, current_piece_type_, current_rotation_, current_piece_type_ + 1);
        solidify_current_piece(); // 固定方块并生成新方块
    }
    return !game_over_; // 返回游戏是否继续
}

// 硬降：将方块直接下落到底部
void TetrisGame::drop_piece() {
    if (game_over_) return; // 如果游戏已结束，不执行任何操作
    
    // 从棋盘上移除当前方块
    place_or_remove_piece(current_piece_pos_, current_piece_type_, current_rotation_, 0); 
    
    // 不断向下移动，直到发生碰撞
    while (!check_collision(current_piece_pos_, current_piece_type_, current_rotation_)) {
        current_piece_pos_.y++;
    }
    current_piece_pos_.y--; // 回退一步，找到最后一个不发生碰撞的位置
 
    // 在最终位置重新绘制方块
    place_or_remove_piece(current_piece_pos_, current_piece_type_, current_rotation_, current_piece_type_ + 1); 
    // 固定方块并生成新方块
    solidify_current_piece();
}

// 清除满行并计算分数
int TetrisGame::clear_full_lines() {
    int lines_cleared = 0; // 记录清除的行数
    
    // 从底部向上检查每一行
    for (int row = BOARD_HEIGHT - 1; row >= 0; --row) {
        bool line_full = true; // 假设当前行是满的
        
        // 检查当前行的每一列
        for (int col = 0; col < BOARD_WIDTH; ++col) {
            if (board_[row][col] == 0) { // 如果找到空格
                line_full = false; // 行不满
                break;
            }
        }

        if (line_full) { // 如果行满了
            lines_cleared++; // 增加已清除行数
            
            // 将当前行以上的所有行下移一行
            for (int k = row; k > 0; --k) {
                memcpy(board_[k], board_[k - 1], BOARD_WIDTH * sizeof(int));
            }
            // 清空最顶行
            memset(board_[0], 0, BOARD_WIDTH * sizeof(int));
            
            row++; // 因为当前行已被上方的行替换，需要重新检查当前行
        }
    }
    
    // 根据清除的行数增加分数
    if (lines_cleared > 0) {
        // 经典俄罗斯方块的计分规则
        if (lines_cleared == 1) score_ += 40;       // 消除1行：40分
        else if (lines_cleared == 2) score_ += 100; // 消除2行：100分
        else if (lines_cleared == 3) score_ += 300; // 消除3行：300分
        else if (lines_cleared >= 4) score_ += 1200; // 消除4行：1200分（俄罗斯方块中的"Tetris"）
    }
    return lines_cleared; // 返回清除的行数
}

// 获取当前棋盘状态
const int* TetrisGame::get_board() const {
    return &board_[0][0]; // 返回棋盘数组的指针
}

// 获取当前得分
int TetrisGame::get_score() const {
    return score_;
}

// 检查游戏是否结束
bool TetrisGame::is_game_over() const {
    return game_over_;
}

//------------------------------------------------------------------------------
// C语言风格的API函数实现
// 这些函数为外部语言（如Python）提供了调用C++代码的接口
//------------------------------------------------------------------------------

// 创建游戏实例
API_EXPORT TetrisGame* create_game() {
    return new TetrisGame(); // 创建一个新的TetrisGame对象
}

// 销毁游戏实例
API_EXPORT void destroy_game(TetrisGame* game) {
    delete game; // 释放TetrisGame对象的内存
}

// 开始新游戏
API_EXPORT void start_new_game_api(TetrisGame* game) {
    if (game) game->start_new_game(); // 如果game不为空，调用start_new_game方法
}

// 向左移动
API_EXPORT bool move_left_api(TetrisGame* game) {
    return game ? game->move_left() : false; // 如果game不为空，调用move_left方法
}

// 向右移动
API_EXPORT bool move_right_api(TetrisGame* game) {
    return game ? game->move_right() : false; // 如果game不为空，调用move_right方法
}

// 旋转方块
API_EXPORT bool rotate_piece_api(TetrisGame* game) {
    return game ? game->rotate_piece() : false; // 如果game不为空，调用rotate_piece方法
}

// 直接下落
API_EXPORT void drop_piece_api(TetrisGame* game) {
    if (game) game->drop_piece(); // 如果game不为空，调用drop_piece方法
}

// 游戏时钟
API_EXPORT bool game_tick_api(TetrisGame* game) {
    return game ? game->game_tick() : false; // 如果game不为空，调用game_tick方法
}

// 获取棋盘状态
API_EXPORT const int* get_board_api(TetrisGame* game) {
    return game ? game->get_board() : nullptr; // 如果game不为空，调用get_board方法
}

// 获取得分
API_EXPORT int get_score_api(TetrisGame* game) {
    return game ? game->get_score() : 0; // 如果game不为空，调用get_score方法
}

// 检查游戏是否结束
API_EXPORT bool is_game_over_api(TetrisGame* game) {
    return game ? game->is_game_over() : true; // 如果game不为空，调用is_game_over方法
}

// 获取棋盘宽度
API_EXPORT int get_board_width_api() {
    return BOARD_WIDTH; // 返回棋盘宽度常量
}

// 获取棋盘高度
API_EXPORT int get_board_height_api() {
    return BOARD_HEIGHT; // 返回棋盘高度常量
} 