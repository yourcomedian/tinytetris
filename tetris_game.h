// tetris_game.h
// 这个头文件定义了俄罗斯方块游戏的C++核心逻辑
// 它包含了游戏的基本数据结构、状态管理和操作接口
#ifndef TETRIS_GAME_H // 防止头文件被重复包含的保护宏
#define TETRIS_GAME_H

// 包含必要的标准库
#include <vector>       // 用于存储可变大小的数组（方块定义）
#include <string>       // 字符串操作（可能用于扩展功能）
#include <cstdlib>      // 包含rand(), srand()等随机数生成函数
#include <ctime>        // 用于time()函数，为随机数生成器提供种子
#include <cstring>      // 用于内存操作函数如memcpy(), memset()

// 定义棋盘维度（常量）
const int BOARD_WIDTH = 10;   // 棋盘宽度，即列数
const int BOARD_HEIGHT = 20;  // 棋盘高度，即行数

// 定义一个二维点或坐标的结构体
// 在游戏中用于表示方块位置和相对位置
struct Point {
    int x;  // 水平坐标（列）
    int y;  // 垂直坐标（行）
};

// 定义一个俄罗斯方块形状的结构体
// 包含方块的组成部分和尺寸信息
struct TetrominoShape {
    Point blocks[4];  // 存储4个组成块相对于锚点的坐标
    int width;        // 方块边界框的宽度
    int height;       // 方块边界框的高度
};

// 俄罗斯方块游戏核心逻辑的主类
// 管理游戏状态、方块移动和游戏规则
class TetrisGame {
public:
    // 构造函数：初始化游戏对象
    TetrisGame();
    
    // 析构函数：清理游戏资源
    ~TetrisGame();

    // 开始新游戏：重置棋盘、分数和游戏状态
    void start_new_game();

    // 游戏控制函数 - 返回true表示操作成功执行
    
    // 将当前方块向左移动一格
    // 如果移动成功（没有碰撞），返回true
    bool move_left();
    
    // 将当前方块向右移动一格
    // 如果移动成功（没有碰撞），返回true
    bool move_right();
    
    // 顺时针旋转当前方块
    // 如果旋转成功（没有碰撞），返回true
    bool rotate_piece();
    
    // 硬降：立即将方块下落到底部
    void drop_piece();

    // 游戏时钟：推进游戏一个时间单位（方块下落一格）
    // 如果游戏仍在继续，返回true；如果游戏结束，返回false
    bool game_tick();

    // 获取游戏状态函数
    
    // 获取当前棋盘状态的指针
    // 返回一维数组指针，按行优先顺序存储棋盘
    const int* get_board() const;
    
    // 获取当前得分
    int get_score() const;
    
    // 检查游戏是否结束
    bool is_game_over() const;

private:
    // 原始方块形状的整数编码数据
    // 来自原始tinytetris的数据，通过位操作解码使用
    static const int initial_block_data[7][4];

    // 解码后的方块形状数据：第一维是方块类型，第二维是旋转状态
    std::vector<std::vector<TetrominoShape>> piece_definitions_;

    // 游戏棋盘：0表示空格，1-7表示不同颜色的方块
    int board_[BOARD_HEIGHT][BOARD_WIDTH];
    
    int score_;       // 当前游戏得分
    bool game_over_;  // 游戏是否结束的标志

    int current_piece_type_;   // 当前方块的类型（0-6）
    int current_rotation_;     // 当前方块的旋转状态（0-3）
    Point current_piece_pos_;  // 当前方块在棋盘上的位置（左上角锚点）

    int tick_speed_control_;   // 控制自动下落速度的计数器
    static const int FALL_SPEED_THRESHOLD = 30;  // 下落速度阈值

    // 辅助函数
    
    // 初始化所有方块形状的定义
    void initialize_piece_definitions();
    
    // 从整数编码中提取2位值（位操作辅助函数）
    int get_two_bit_value(int piece_raw_data, int bit_offset) const;

    // 在棋盘上生成新的方块
    void spawn_new_piece();
    
    // 检查方块在给定位置和旋转状态下是否会发生碰撞
    bool check_collision(Point pos, int piece_type, int rotation) const;

    // 在棋盘上放置或移除方块
    // value为0表示移除，value>0表示放置（value对应方块颜色）
    void place_or_remove_piece(Point pos, int piece_type, int rotation, int value);

    // 将当前方块固定在棋盘上，并检查行消除和游戏状态
    void solidify_current_piece();
    
    // 清除已满的行并返回清除的行数
    int clear_full_lines();

    // 获取特定类型和旋转状态的方块形状数据
    const TetrominoShape& get_shape_data(int piece_type, int rotation) const;
    
    // 获取当前方块的形状数据
    const TetrominoShape& get_current_shape_data() const;
};

// 为不同操作系统定义导出宏，用于创建动态链接库
#ifdef _WIN32
    #define API_EXPORT __declspec(dllexport)  // Windows导出符号
#else // 适用于Linux和macOS
    #define API_EXPORT __attribute__((visibility("default")))  // Linux/Mac导出符号
#endif

// 定义C风格的API接口
// extern "C" 确保函数名不被C++编译器修饰，使得其他语言（如Python）可以调用这些函数
extern "C" {
    // 创建游戏实例
    API_EXPORT TetrisGame* create_game();
    
    // 销毁游戏实例
    API_EXPORT void destroy_game(TetrisGame* game);
    
    // 开始新游戏
    API_EXPORT void start_new_game_api(TetrisGame* game);

    // 游戏控制函数
    API_EXPORT bool move_left_api(TetrisGame* game);
    API_EXPORT bool move_right_api(TetrisGame* game);
    API_EXPORT bool rotate_piece_api(TetrisGame* game);
    API_EXPORT void drop_piece_api(TetrisGame* game);
    API_EXPORT bool game_tick_api(TetrisGame* game); // 推进游戏一个节拍，返回!game_over

    // 获取游戏状态函数
    API_EXPORT const int* get_board_api(TetrisGame* game); // 获取棋盘数据
    API_EXPORT int get_score_api(TetrisGame* game);        // 获取得分
    API_EXPORT bool is_game_over_api(TetrisGame* game);    // 检查游戏是否结束
    
    // 获取棋盘尺寸的工具函数
    API_EXPORT int get_board_width_api();   // 返回棋盘宽度
    API_EXPORT int get_board_height_api();  // 返回棋盘高度
}

#endif // TETRIS_GAME_H 