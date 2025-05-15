# app.py
# 俄罗斯方块游戏 Flask 后端
# 本文件是一个 Flask Web 应用，充当 C++ 游戏引擎与 JavaScript 前端之间的桥梁

from flask import Flask, jsonify, request, render_template
from cffi import FFI  # CFFI库用于Python调用C/C++代码
import os
import platform

# 创建Flask应用实例
app = Flask(__name__)
# 创建FFI对象，用于和C/C++代码交互
ffi = FFI()

# 根据不同操作系统确定动态链接库的文件名
# 不同操作系统的库扩展名不同：Linux是.so，macOS是.dylib，Windows是.dll
lib_name = ""
if platform.system() == "Linux":
    lib_name = "libtetris_core.so"
elif platform.system() == "Darwin": # macOS
    lib_name = "libtetris_core.dylib"
elif platform.system() == "Windows":
    lib_name = "tetris_core.dll" # 假设在Windows上是这个名字
else:
    raise OSError("不支持的操作系统，无法加载 Tetris Core 库")

# 构建库文件的两个可能路径：项目根目录或build子目录
lib_path_root = os.path.join(os.path.dirname(os.path.abspath(__file__)), lib_name)
lib_path_build = os.path.join(os.path.dirname(os.path.abspath(__file__)), "build", lib_name)

# 初始化库路径和库对象
actual_lib_path = None
tetris_lib = None # 初始化为None，后面会尝试加载库

# 尝试在两个可能的位置加载库文件
if os.path.exists(lib_path_build):
    actual_lib_path = lib_path_build
elif os.path.exists(lib_path_root):
    actual_lib_path = lib_path_root

# 如果找到库文件，尝试加载它
if actual_lib_path:
    print(f"尝试加载库: {actual_lib_path}") # 调试日志
    # 使用CFFI定义C函数接口，这些定义需要与tetris_game.h文件中的API声明完全匹配
    # 在C语言中使用分号结束，但在CFFI的声明中不需要
    ffi.cdef("""
        typedef struct TetrisGame TetrisGame; // 不透明指针类型，我们不需要知道其内部结构

        TetrisGame* create_game();         // 创建游戏实例
        void destroy_game(TetrisGame* game); // 销毁游戏实例
        void start_new_game_api(TetrisGame* game); // 开始新游戏

        bool move_left_api(TetrisGame* game);  // 向左移动方块
        bool move_right_api(TetrisGame* game); // 向右移动方块
        bool rotate_piece_api(TetrisGame* game); // 旋转方块
        void drop_piece_api(TetrisGame* game);   // 直接下落方块
        bool game_tick_api(TetrisGame* game); // 推进游戏一个节拍，返回 !game_over

        const int* get_board_api(TetrisGame* game); // 获取棋盘数据
        int get_score_api(TetrisGame* game);        // 获取当前分数
        bool is_game_over_api(TetrisGame* game);    // 检查游戏是否结束
        int get_board_width_api();  // 获取棋盘宽度
        int get_board_height_api(); // 获取棋盘高度
    """)
    try:
        # 尝试加载动态链接库
        tetris_lib = ffi.dlopen(actual_lib_path)
        print(f"成功加载 C++ 库: {actual_lib_path}")
    except OSError as e:
        # 捕获库加载失败的错误
        print(f"加载共享库 {actual_lib_path} 时出错: {e}")
        tetris_lib = None # 确保在加载失败时也被设置为 None
    except Exception as e: # 捕获其他可能的异常
        print(f"加载共享库时发生未知错误: {e}")
        tetris_lib = None # 确保在加载失败时也被设置为 None
elif lib_name: # 只有在lib_name被设置了 (即支持的操作系统) 才打印找不到的错误
    print(f"错误: 在 {lib_path_root} 或 {lib_path_build} 中未找到共享库 {lib_name}。")
    print("请确保 C++ 库已正确编译并放置。")
    # tetris_lib 此时已经是 None

# 全局游戏实例指针
# 在多用户场景下，你需要会话管理或一个游戏实例的字典。
# 对于这个单人游戏示例，一个全局游戏实例就足够了。
game_instance = None

def get_game():
    """
    获取或创建游戏实例。
    如果游戏实例不存在，则创建一个新的并开始游戏。
    返回游戏实例指针。
    """
    global game_instance
    if tetris_lib is None:
        raise RuntimeError("Tetris 库未加载。无法创建或管理游戏。")
    if game_instance is None:
        # 如果没有现有的游戏实例，创建一个并开始新游戏
        game_instance = tetris_lib.create_game()
        tetris_lib.start_new_game_api(game_instance)
    return game_instance

def get_board_state_from_lib(game):
    """
    从C++库中获取棋盘状态并转换为Python数据结构。
    
    参数:
        game: C++游戏实例的指针
    
    返回:
        board_list: 二维列表，表示棋盘状态
        width: 棋盘宽度
        height: 棋盘高度
    """
    if not game or tetris_lib is None:
        return None, 0, 0
        
    # 获取棋盘数据和维度
    board_ptr = tetris_lib.get_board_api(game)
    width = tetris_lib.get_board_width_api()
    height = tetris_lib.get_board_height_api()
    
    if board_ptr == ffi.NULL: # 检查返回的指针是否为空
        return None, width, height

    # 将C数组（一维数组）转换为Python嵌套列表（二维数组）
    # C++中的棋盘是按行优先顺序存储的一维数组
    board_list = []
    for r in range(height):
        row = [board_ptr[r * width + c] for c in range(width)]
        board_list.append(row)
    return board_list, width, height

# API路由：开始新游戏
@app.route('/api/start', methods=['POST'])
def start_game():
    """
    处理开始新游戏的API请求。
    清理旧的游戏实例（如果有），创建新的游戏实例，并返回初始游戏状态。
    """
    global game_instance
    if tetris_lib is None:
        return jsonify({"error": "Tetris 库未加载"}), 500

    if game_instance: # 如果存在旧的游戏实例
        tetris_lib.destroy_game(game_instance) # 清理旧的游戏实例
    game_instance = tetris_lib.create_game()
    tetris_lib.start_new_game_api(game_instance)
    
    # 获取并返回初始游戏状态
    board_data, _, _ = get_board_state_from_lib(game_instance)
    return jsonify({
        "message": "新游戏已开始",
        "board": board_data,
        "score": tetris_lib.get_score_api(game_instance),
        "gameOver": tetris_lib.is_game_over_api(game_instance)
    })

# API路由：获取当前游戏状态
@app.route('/api/state', methods=['GET'])
def get_state():
    """
    获取当前游戏状态的API。
    返回棋盘布局、分数和游戏结束状态。
    """
    game = get_game() # 获取当前或新的游戏实例
    if tetris_lib is None:
        return jsonify({"error": "Tetris 库未加载"}), 500
    if not game: # 理论上 get_game() 会确保有一个实例，除非库加载失败
         return jsonify({"error": "游戏未初始化。请调用 /api/start"}), 400

    # 获取并返回当前游戏状态
    board_data, _, _ = get_board_state_from_lib(game)
    return jsonify({
        "board": board_data,
        "score": tetris_lib.get_score_api(game),
        "gameOver": tetris_lib.is_game_over_api(game)
    })

# API路由：处理游戏动作
@app.route('/api/action', methods=['POST'])
def handle_action():
    """
    处理游戏动作的API。
    接收客户端发送的动作（如移动、旋转等），执行动作，并返回更新后的游戏状态。
    """
    game = get_game()
    if tetris_lib is None:
        return jsonify({"error": "Tetris 库未加载"}), 500
    if not game:
         return jsonify({"error": "游戏未初始化。请调用 /api/start"}), 400
    if tetris_lib.is_game_over_api(game):
        return jsonify({"message": "游戏已结束。请开始新游戏。", "gameOver": True}), 400

    # 从请求中获取动作类型
    action = request.json.get('action') # 从请求的JSON体中获取动作
    action_taken = False # 标记动作是否实际执行（例如，移动是否成功）

    # 根据动作类型执行相应的操作
    if action == 'left':
        action_taken = tetris_lib.move_left_api(game)
    elif action == 'right':
        action_taken = tetris_lib.move_right_api(game)
    elif action == 'rotate':
        action_taken = tetris_lib.rotate_piece_api(game)
    elif action == 'drop':
        tetris_lib.drop_piece_api(game) # 硬直落总是会改变状态
        action_taken = True # 假设 drop 改变了状态，即使只是固化方块
    elif action == 'tick': # 游戏时钟滴答，由前端计时器触发
        action_taken = tetris_lib.game_tick_api(game) # game_tick 推进游戏状态
    else:
        return jsonify({"error": "无效的动作"}), 400

    # 获取并返回更新后的游戏状态
    board_data, _, _ = get_board_state_from_lib(game)
    return jsonify({
        "action": action,
        "success": action_taken, # 对于移动操作，指示移动是否有效
        "board": board_data,
        "score": tetris_lib.get_score_api(game),
        "gameOver": tetris_lib.is_game_over_api(game)
    })

# 网站主页路由
@app.route('/')
def index():
    """
    渲染游戏的主页HTML。
    这是用户访问网站时看到的页面，包含游戏界面。
    """
    return render_template('index.html') # 渲染HTML模板


# 主程序入口点
if __name__ == '__main__':
    # 警告用户，如果库加载失败
    if tetris_lib is None:
        print("严重错误: Tetris C++ 库加载失败。应用程序将无法正常工作。")
        print("请检查编译和库路径。")
        # 如果库是绝对必需的，可以选择在此处退出:
        # import sys
        # sys.exit(1)
    
    # 启动Flask应用
    # 使用stat类型的重载器以避免watchdog的兼容性问题
    # debug=True 启用开发模式，端口5001，自动重新加载代码改动
    app.run(debug=True, port=5001, use_reloader=True, reloader_type='stat') 