# app.py
# 俄罗斯方块游戏 Flask 后端 - pybind11版本
# 本文件是一个 Flask Web 应用，充当 pybind11 游戏引擎与 JavaScript 前端之间的桥梁

from flask import Flask, jsonify, request, render_template
import sys
import os

# 添加build目录到Python路径，以便导入编译后的模块
sys.path.insert(0, os.path.join(os.path.dirname(__file__), 'build'))

# 创建Flask应用实例
app = Flask(__name__)

# 尝试导入pybind11编译的tetris_core模块
tetris_core = None
try:
    import tetris_core
    print("✓ 成功导入tetris_core pybind11模块")
except ImportError as e:
    print(f"✗ 导入tetris_core模块失败: {e}")
    print("请确保已编译pybind11模块：")
    print("  ./build.sh")
    tetris_core = None

# 全局游戏实例
# 在多用户场景下，你需要会话管理或一个游戏实例的字典。
# 对于这个单人游戏示例，一个全局游戏实例就足够了。
game_instance = None

def get_game():
    """
    获取或创建游戏实例。
    如果游戏实例不存在，则创建一个新的并开始游戏。
    返回游戏实例。
    """
    global game_instance
    if tetris_core is None:
        raise RuntimeError("tetris_core 模块未加载。无法创建或管理游戏。")
    if game_instance is None:
        # 如果没有现有的游戏实例，创建一个并开始新游戏
        game_instance = tetris_core.TetrisGame()
        game_instance.start_new_game()
    return game_instance

def get_board_state_from_game(game):
    """
    从游戏实例中获取棋盘状态。
    
    参数:
        game: TetrisGame实例
    
    返回:
        board_list: 二维列表，表示棋盘状态
        width: 棋盘宽度
        height: 棋盘高度
    """
    if not game or tetris_core is None:
        return None, 0, 0
        
    # 使用pybind11绑定的方法获取棋盘数据
    board_list = game.get_board_list()  # 直接获取二维列表
    width = tetris_core.BOARD_WIDTH
    height = tetris_core.BOARD_HEIGHT
    
    return board_list, width, height
# API路由：开始新游戏
@app.route('/api/start', methods=['POST'])
def start_game():
    """
    处理开始新游戏的API请求。
    创建新的游戏实例，并返回初始游戏状态。
    """
    global game_instance
    if tetris_core is None:
        return jsonify({"error": "tetris_core 模块未加载"}), 500

    # 创建新的游戏实例
    game_instance = tetris_core.TetrisGame()
    game_instance.start_new_game()
    
    # 获取并返回初始游戏状态
    board_data, _, _ = get_board_state_from_game(game_instance)
    return jsonify({
        "message": "新游戏已开始",
        "board": board_data,
        "score": game_instance.get_score(),
        "gameOver": game_instance.is_game_over()
    })

# API路由：获取当前游戏状态
@app.route('/api/state', methods=['GET'])
def get_state():
    """
    获取当前游戏状态的API。
    返回棋盘布局、分数和游戏结束状态。
    """
    if tetris_core is None:
        return jsonify({"error": "tetris_core 模块未加载"}), 500
    
    game = get_game()  # 获取当前或新的游戏实例
    if not game:
        return jsonify({"error": "游戏未初始化。请调用 /api/start"}), 400

    # 获取并返回当前游戏状态
    board_data, _, _ = get_board_state_from_game(game)
    return jsonify({
        "board": board_data,
        "score": game.get_score(),
        "gameOver": game.is_game_over()
    })

# API路由：处理游戏动作
@app.route('/api/action', methods=['POST'])
def handle_action():
    """
    处理游戏动作的API。
    接收客户端发送的动作（如移动、旋转等），执行动作，并返回更新后的游戏状态。
    """
    if tetris_core is None:
        return jsonify({"error": "tetris_core 模块未加载"}), 500
    
    game = get_game()
    if not game:
        return jsonify({"error": "游戏未初始化。请调用 /api/start"}), 400
    
    if game.is_game_over():
        return jsonify({"message": "游戏已结束。请开始新游戏。", "gameOver": True}), 400

    # 从请求中获取动作类型
    action = request.json.get('action')  # 从请求的JSON体中获取动作
    action_taken = False  # 标记动作是否实际执行（例如，移动是否成功）

    # 根据动作类型执行相应的操作
    if action == 'left':
        action_taken = game.move_left()
    elif action == 'right':
        action_taken = game.move_right()
    elif action == 'rotate':
        action_taken = game.rotate_piece()
    elif action == 'drop':
        game.drop_piece()  # 硬降总是会改变状态
        action_taken = True  # 假设 drop 改变了状态，即使只是固化方块
    elif action == 'tick':  # 游戏时钟滴答，由前端计时器触发
        action_taken = game.game_tick()  # game_tick 推进游戏状态
    else:
        return jsonify({"error": "无效的动作"}), 400

    # 获取并返回更新后的游戏状态
    board_data, _, _ = get_board_state_from_game(game)
    return jsonify({
        "action": action,
        "success": action_taken,  # 对于移动操作，指示移动是否有效
        "board": board_data,
        "score": game.get_score(),
        "gameOver": game.is_game_over()
    })

# 网站主页路由
@app.route('/')
def index():
    """
    渲染游戏的主页HTML。
    这是用户访问网站时看到的页面，包含游戏界面。
    """
    return render_template('index.html')  # 渲染HTML模板

# API路由：获取游戏信息
@app.route('/api/info', methods=['GET'])
def get_info():
    """
    获取游戏基本信息的API。
    返回棋盘尺寸等常量信息。
    """
    if tetris_core is None:
        return jsonify({"error": "tetris_core 模块未加载"}), 500
    
    return jsonify({
        "boardWidth": tetris_core.BOARD_WIDTH,
        "boardHeight": tetris_core.BOARD_HEIGHT,
        "version": "pybind11"
    })

# 主程序入口点
if __name__ == '__main__':
    # 警告用户，如果模块加载失败
    if tetris_core is None:
        print("严重错误: tetris_core pybind11模块加载失败。应用程序将无法正常工作。")
        print("请检查编译和模块路径。")
        print("运行以下命令编译模块：")
        print("  ./build.sh")
        # 如果模块是绝对必需的，可以选择在此处退出:
        # import sys
        # sys.exit(1)
    
    # 启动Flask应用
    # debug=True 启用开发模式，端口5001，自动重新加载代码改动
    app.run(debug=True, port=5001, use_reloader=False, reloader_type='stat') 
