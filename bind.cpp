// bind.cpp
// pybind11绑定文件，为TetrisGame类提供Python接口
#include <cstddef>
#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <string>
#include <vector>
#include "tetris_game.h"

namespace py = pybind11;

PYBIND11_MODULE(tetris_core, m) {
    m.doc() = "TinyTetris游戏核心模块 - 使用pybind11绑定";

    // // 绑定Point结构体
    // py::class_<Point>(m, "Point")
    //     .def(py::init<>())
    //     .def_readwrite("x", &Point::x)
    //     .def_readwrite("y", &Point::y)
    //     .def("__repr__", [](const Point &p) {
    //         return "Point(x=" + std::to_string(p.x) + ", y=" + std::to_string(p.y) + ")";
    //     });

    // py::class_<TetrominoShape>(m, "TetrominoShape")
    //     .def(py::init<>())
    //     .def_readwrite("width", &TetrominoShape::width)
    //     .def_readwrite("height", &TetrominoShape::height)
    //     .def_property("bolcks",
    //         [](const TetrominoShape &shape) {
    //             std::vector<Point> blocks_vec;
    //             for (int i = 0; i < 4; ++i) {
    //                 blocks_vec.push_back(shape.blocks[i]);
    //             }
    //             return blocks_vec;
    //         },
    //         [](TetrominoShape &shape, const std::vector<Point> &blocks_vec) {
    //             for (size_t i = 0; i < 4 && i < blocks_vec.size(); ++i){
    //                 shape.blocks[i] = blocks_vec[i];
    //             }
    //         })
    //     .def("__repr__", [](const TetrominoShape &shape){
    //         return "TetrominoShape(width=" + std::to_string(shape.width) + 
    //             ",height="+ std::to_string(shape.height) + ")";
    //     });

    py::class_<TetrisGame>(m,"TetrisGame")
        .def(py::init<>(), "创建新的游戏实例")
        .def("start_new_game", &TetrisGame::start_new_game,"开始新游戏")
        .def("move_left",&TetrisGame::move_left,"向左移动方块")
        .def("move_right",&TetrisGame::move_right,"向右移动方块")
        .def("rotate_piece",&TetrisGame::rotate_piece,"旋转方块")
        .def("drop_piece",&TetrisGame::drop_piece,"硬降当前方块")
        .def("game_tick", &TetrisGame::game_tick,"游戏时钟推进一步")

        .def("get_score", &TetrisGame::get_score,"获取分数")
        .def("is_game_over", &TetrisGame::is_game_over,"检查游戏是否结束")

        .def("get_board_list",[](const TetrisGame &game){
            const int* board_data = game.get_board();
            std::vector<std::vector<int>> board_2d;
            for (int row = 0; row < BOARD_HEIGHT; ++row) {
                std::vector<int> row_data;
                for (int col = 0; col < BOARD_WIDTH; ++col) {
                    row_data.push_back(board_data[row * BOARD_WIDTH + col]);
                }
                board_2d.push_back(row_data);
            }
            return board_2d;
        },"获取棋盘状态为二维列表");
    m.attr("BOARD_WIDTH") = BOARD_WIDTH;
    m.attr("BOARD_HEIGHT") = BOARD_HEIGHT;
        
        
    m.def("get_board_width", [](){return BOARD_WIDTH;}, "获取棋盘高度");
    m.def("get_board_height", [](){return BOARD_HEIGHT;}, "获取棋盘高度");
} 