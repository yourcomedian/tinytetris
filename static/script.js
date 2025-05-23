// static/script.js
// 俄罗斯方块游戏的前端JavaScript代码
// 负责处理用户输入、绘制游戏界面和与后端API通信

// 当DOM内容加载完成后执行代码
document.addEventListener('DOMContentLoaded', () => {
    // 获取页面上的HTML元素
    const canvas = document.getElementById('tetris-board'); // 游戏棋盘的Canvas元素
    const context = canvas.getContext('2d');                // Canvas 2D绘图上下文
    const scoreElement = document.getElementById('score');  // 显示分数的元素
    const startButton = document.getElementById('start-button'); // 开始游戏按钮
    const gameOverMessage = document.getElementById('game-over-message'); // 游戏结束消息

    // 游戏棋盘的配置参数（必须与C++后端的值匹配）
    const BOARD_WIDTH_CELLS = 10;  // 棋盘宽度（格子数）
    const BOARD_HEIGHT_CELLS = 20; // 棋盘高度（格子数）
    const CELL_SIZE = 25;          // 每个格子的像素大小

    // 设置Canvas的尺寸
    canvas.width = BOARD_WIDTH_CELLS * CELL_SIZE;
    canvas.height = BOARD_HEIGHT_CELLS * CELL_SIZE;

    // 游戏状态变量
    let gameBoard = [];             // 存储从后端获取的棋盘状态
    let score = 0;                  // 当前游戏分数
    let gameOver = false;           // 游戏是否结束的标志
    let gameLoopInterval = null;    // 游戏主循环的计时器ID
    let softDropInterval = null;    // 软下落（按住S键）的计时器ID
    const GAME_TICK_MS = 500;       // 游戏自动下落的间隔时间（毫秒）
    const SOFT_DROP_MS = 60;        // 按住S键加速下落的间隔时间（毫秒）

    // 方块颜色定义（优化了颜色以避免过浅的颜色）
    // 索引对应棋盘上的值：0为空白，1-7为不同的方块颜色
    const pieceColors = [
        '#FFFFFF', // 0: 棋盘背景格颜色
        '#00BCD4', // 1: I形方块（青色）
        '#2196F3', // 2: J形方块（蓝色）
        '#FF9800', // 3: L形方块（橙色）
        '#FFEB3B', // 4: O形方块（黄色）
        '#4CAF50', // 5: S形方块（绿色）
        '#9C27B0', // 6: T形方块（紫色）
        '#F44336'  // 7: Z形方块（红色）
    ];

    /**
     * 绘制游戏棋盘
     * 根据gameBoard数组的内容绘制棋盘上的每个格子
     */
    function drawBoard() {
        // 用背景色填充整个Canvas，而不是clearRect，这样空白格也有颜色
        context.fillStyle = pieceColors[0]; 
        context.fillRect(0, 0, canvas.width, canvas.height);
        context.strokeStyle = '#444'; // 格子边框颜色（深灰色）

        // 如果棋盘数据不存在，绘制空白网格
        if (!gameBoard || gameBoard.length === 0) {
            // 绘制空棋盘网格
            for (let r = 0; r < BOARD_HEIGHT_CELLS; r++) {
                for (let c = 0; c < BOARD_WIDTH_CELLS; c++) {
                    // 只绘制网格线，背景已填充
                    context.strokeRect(c * CELL_SIZE, r * CELL_SIZE, CELL_SIZE, CELL_SIZE);
                }
            }
            return;
        }

        // 有棋盘数据时，根据数据绘制每个格子
        for (let r = 0; r < BOARD_HEIGHT_CELLS; r++) {
            for (let c = 0; c < BOARD_WIDTH_CELLS; c++) {
                // 获取当前格子的值，如果行不存在则默认为0
                const cellValue = gameBoard[r] ? gameBoard[r][c] : 0;
                // 根据格子值选择颜色，如果值无效则使用背景色
                context.fillStyle = pieceColors[cellValue] || pieceColors[0]; 
                // 填充格子
                context.fillRect(c * CELL_SIZE, r * CELL_SIZE, CELL_SIZE, CELL_SIZE);
                // 绘制格子边框
                context.strokeRect(c * CELL_SIZE, r * CELL_SIZE, CELL_SIZE, CELL_SIZE);
            }
        }
    }

    /**
     * 更新游戏状态并重绘棋盘
     * @param {Object} data - 从后端API接收的数据
     */
    async function updateGameState(data) {
        // 更新棋盘数据
        if (data.board) {
            gameBoard = data.board;
        }
        // 更新分数
        if (data.score !== undefined) {
            score = data.score;
            scoreElement.textContent = score; // 更新显示的分数
        }
        // 更新游戏结束状态
        if (data.gameOver !== undefined) {
            gameOver = data.gameOver;
            if (gameOver) {
                // 如果游戏结束，显示游戏结束消息
                gameOverMessage.style.display = 'block';
                startButton.textContent = '重新开始';
                // 清除所有游戏计时器
                if (gameLoopInterval) clearInterval(gameLoopInterval);
                if (softDropInterval) clearInterval(softDropInterval);
                gameLoopInterval = null;
                softDropInterval = null;
            } else {
                // 如果游戏未结束，隐藏游戏结束消息
                gameOverMessage.style.display = 'none';
            }
        }
        // 重绘棋盘
        drawBoard();
    }

    /**
     * 向后端发送游戏动作请求
     * @param {string} action - 要执行的动作（left/right/rotate/drop/tick）
     * @returns {boolean} - 动作是否成功执行
     */
    async function sendAction(action) {
        // 如果游戏已结束且动作不是开始/重启，则不执行任何操作
        if (gameOver && action !== 'start' && action !== 'restart') return false;
        try {
            // 发送POST请求到后端API
            const response = await fetch('/api/action', {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify({ action: action }),
            });
            
            // 如果请求失败
            if (!response.ok) {
                const errorData = await response.json();
                console.error('Action failed:', errorData.error || response.statusText);
                // 如果游戏已结束，更新游戏状态
                if (errorData.gameOver) {
                    updateGameState({ gameOver: true, score: errorData.score, board: errorData.board });
                }
                return false; // 动作失败
            }
            
            // 请求成功，解析响应数据并更新游戏状态
            const data = await response.json();
            updateGameState(data);
            return true; // 动作成功
        } catch (error) {
            // 捕获网络错误或其他异常
            console.error('Error sending action:', error);
            return false;
        }
    }

    /**
     * 开始新游戏
     * 向后端API发送请求，重置游戏状态
     */
    async function startGame() {
        // 清除所有游戏计时器
        if (gameLoopInterval) clearInterval(gameLoopInterval);
        if (softDropInterval) clearInterval(softDropInterval);
        gameLoopInterval = null;
        softDropInterval = null;

        try {
            // 调用后端开始新游戏的API
            const response = await fetch('/api/start', { method: 'POST' });
            if (!response.ok) {   // 检查请求是否成功（response.ok）
                console.error('Failed to start game:', response.statusText);
                return;
            }
            
            // 解析响应数据并更新游戏状态
            const data = await response.json();
            gameOver = data.gameOver; // 更新游戏结束状态
            updateGameState(data);
            startButton.textContent = '游戏中...';
            
            // 如果游戏未结束，启动游戏循环
            if (!gameOver) {
                // 设置游戏主循环，定期调用game_tick
                gameLoopInterval = setInterval(() => {
                    if (!gameOver) sendAction('tick');
                }, GAME_TICK_MS);
            }
        } catch (error) {
            console.error('Error starting game:', error);
        }
    }

    // 软下落状态标志
    let isSoftDropping = false;

    // 键盘按下事件处理
    document.addEventListener('keydown', (event) => {
        // 防止S键重复触发（长按）
        if (event.repeat && (event.key === 's' || event.key === 'S')) return;
        
        // 如果游戏已结束，只允许Q键重启游戏
        if (gameOver && event.key !== 'q' && event.key !== 'Q') return; 

        let action = null;
        // 根据按键确定要执行的动作
        switch (event.key) {
            case 'a': // 左移
            case 'A':
                action = 'left'; 
                break;
            case 'd': // 右移
            case 'D':
                action = 'right'; 
                break;
            case 'w': // 旋转
            case 'W':
                action = 'rotate'; 
                break;
            case ' ': // 空格键，直接下落（硬降）
                event.preventDefault(); // 防止页面滚动
                action = 'drop';
                break;
            case 's': // 加速下落（软降）
            case 'S':
                event.preventDefault(); // 防止页面滚动
                // 如果已在软降或游戏已结束，不执行操作
                if (!isSoftDropping && !gameOver) {
                    isSoftDropping = true;
                    sendAction('tick'); // 立即下落一次
                    // 设置软降计时器，加速方块下落
                    if (softDropInterval) clearInterval(softDropInterval);
                    softDropInterval = setInterval(() => {
                        if (!gameOver) {
                            sendAction('tick');
                        } else {
                            // 如果游戏结束，清除软降计时器
                            clearInterval(softDropInterval);
                            isSoftDropping = false;
                        }
                    }, SOFT_DROP_MS);
                }
                return; // S键自己处理循环，不进入下面的sendAction
            case 'q': // 重新开始游戏
            case 'Q':
                startGame(); 
                return;
            default: return; // 其他按键不处理
        }
        
        // 如果有有效动作，发送到后端
        if (action) {
            sendAction(action);
        }
    });

    // 键盘松开事件处理
    document.addEventListener('keyup', (event) => {
        // 当松开S键时，停止软降
        if (event.key === 's' || event.key === 'S') {
            if (softDropInterval) {
                clearInterval(softDropInterval);
                softDropInterval = null;
            }
            isSoftDropping = false;
        }
    });

    // 为开始游戏按钮添加点击事件
    startButton.addEventListener('click', startGame);

    // 初始绘制一个空的棋盘
    drawBoard(); 
});

/**
 * 页面加载时执行：主函数、初始化dom元素参数、设置canvas尺寸、初始化状态变量、绘制空棋盘
 * 
 * 用户操作阶段： 
 * 
 * 点击开始按钮:
 * 触发startgame函数；
 * 清除所有计时器；
 * 向后端发送/api/start请求，获取初始棋盘和分数；
 * 更新游戏状态；
 * 如果游戏未结束，启动主循环（每500ms自动下落一次，调用sendAction('tick')）。
 * 按键操作：
 * 监听keydown事件
 * 判断按键类型，决定动作（左/右/旋转/硬降/软降/重启）。
 * 软降（S键）会启动一个更快的下落计时器（每60ms），并立即下落一次。
 * 其他动作（如A/D/W/空格）直接调用sendAction()发送到后端。
 * Q键可以随时重启游戏（调用startGame()）
 * 监听keyup事件：松开S键时，停止软降计时器。
 * 
 * 游戏主循环：
 * 如果游戏在进行中，主循环每500ms自动调用sendAction('tick')，让方块自动下落。
 * 
 * 交互：
 * 每次有动作（包括自动下落、用户操作），都通过sendAction()向后端发送请求。
 * 后端返回最新的棋盘、分数、游戏是否结束等信息。
 * 前端用updateGameState()更新状态和界面。
 * 
 * 如果后端返回gameOver为true，前端会：
 * 停止所有计时器。
 * 显示“游戏结束”提示。
 * 按钮变为“重新开始”。
 */