#include <iostream>
#include <vector>
#include <string>
#include <mutex>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <ctime>
#include <cstdlib>

#include "httplib.h"
#include "json.hpp"

using json = nlohmann::json;

std::vector<int> g_game_board(9, 2);  // 2: empty, 1: player (X), 0: bot (O)
std::mutex g_game_mutex;              // Mutex to protect global state
int g_player_starts_next = 1;         // 1 for Player, 0 for Bot. Player starts first game.

int game_draw(const std::vector<int>& values) {
    for (int val : values) {
        if (val == 2) return 0;
    }
    return 1;
}

int check_grid(const std::vector<int>& values) {
    for (int i = 0; i < 3; i++) { // row check
        if (values[i * 3] != 2 && values[i * 3] == values[i * 3 + 1] && values[i * 3 + 2] == values[i * 3 + 1]) {
            return values[i * 3];
        }
    }
    for (int i = 0; i < 3; i++) { // column check
        if (values[i] != 2 && values[i] == values[i + 3] && values[i] == values[i + 6]) {
            return values[i];
        }
    }
    if (values[0] != 2 && values[0] == values[4] && values[0] == values[8]) return values[0];
    if (values[2] != 2 && values[2] == values[4] && values[4] == values[6]) return values[2];
    if (game_draw(values) == 1) return 3; // 3 for draw
    return 2; // 2 for ongoing
}

int to_win(const std::vector<int>& values, int n) {
    for (int i = 0; i < 9; ++i) {
        if (values[i] == 2) {
            std::vector<int> values_copy = values;
            values_copy[i] = n;
            if (check_grid(values_copy) == n) return i;
        }
    }
    return 100;
}

int randint(int j, int k) {
    if (j >= k) return j;
    return (rand() % (k - j)) + j;
}

void weigh(const std::vector<int>& values, std::vector<int>& weights, int n) {
    for (int i = 0; i < 9; i++) {
        if (values[i] == n) {
            int x = i % 3; int y = i / 3;
            // Simplified logic for checking lines and adding weights
            const int other_player = (n == 0) ? 1 : 0;
            // Column
            if (values[x + ((y + 1) % 3) * 3] != other_player && values[x + ((y + 2) % 3) * 3] != other_player) {
                weights[x + ((y + 1) % 3) * 3]++;
                weights[x + ((y + 2) % 3) * 3]++;
            }
            // Row
            if (values[(x + 1) % 3 + y * 3] != other_player && values[(x + 2) % 3 + y * 3] != other_player) {
                weights[(x + 1) % 3 + y * 3]++;
                weights[(x + 2) % 3 + y * 3]++;
            }
            // Diagonals
            if (x == y) {
                if (values[((x + 1) % 3) * 4] != other_player && values[((x + 2) % 3) * 4] != other_player) {
                    weights[((x + 1) % 3) * 4]++;
                    weights[((x + 2) % 3) * 4]++;
                }
            }
            if (x + y == 2) {
                if (values[2] != other_player && values[6] != other_player && i==4) weights[2]++, weights[6]++;
                if (values[2] != other_player && values[4] != other_player && i==6) weights[2]++, weights[4]++;
                if (values[4] != other_player && values[6] != other_player && i==2) weights[4]++, weights[6]++;
            }
        }
    }
}

int return_normal_move(std::vector<int> values, int n); // Forward declaration

int final_d(std::vector<int> values, int n) {
    int t = check_grid(values);
    if (t != 2) {
        return t;
    }
    t = return_normal_move(values, n);
    values[t] = n;
    return final_d(values, (n + 1) % 2);
}

int return_normal_move(std::vector<int> values, int n) {
    int t = to_win(values, n);
    if (t != 100) return t;
    t = to_win(values, (n + 1) % 2);
    if (t != 100) return t;

    std::vector<int> weights(9, 0);
    weigh(values, weights, n);
    weigh(values, weights, (n + 1) % 2);

    int max_weight = -1;
    for(int i = 0; i < 9; ++i) {
        if(values[i] == 2) max_weight = std::max(max_weight, weights[i]);
    }

    std::vector<int> best_moves;
    for (int i = 0; i < 9; i++) {
        if (values[i] == 2 && weights[i] == max_weight) {
            best_moves.push_back(i);
        }
    }
    if (best_moves.empty()) { // Should not happen in a valid game state
        for(int i=0; i<9; ++i) if(values[i]==2) return i;
    }
    return best_moves[randint(0, best_moves.size())];
}

void optimal_move(std::vector<int>& values, int n) {
    // 1. If bot can win, make the winning move.
    int t = to_win(values, n);
    if (t != 100) {
        if(values[t] == 2) { values[t] = n; return; }
    }
    // 2. If player is about to win, block them.
    t = to_win(values, (n + 1) % 2);
    if (t != 100) {
        if(values[t] == 2) { values[t] = n; return; }
    }
    
    // 3. Find all "best" moves based on weight heuristic
    std::vector<int> weights(9, 0);
    weigh(values, weights, n);
    weigh(values, weights, (n+1)%2);

    int max_weight = -1;
    for(int i = 0; i < 9; ++i) if(values[i] == 2) max_weight = std::max(max_weight, weights[i]);

    std::vector<int> best_moves;
    for(int i=0; i<9; ++i) if(values[i] == 2 && weights[i] == max_weight) best_moves.push_back(i);

    // 4. From those best moves, find one that doesn't lead to a loss
    std::vector<int> optimal_moves;
    for (int move : best_moves) {
        std::vector<int> values_copy = values;
        values_copy[move] = n;
        if (final_d(values_copy, (n + 1) % 2) != (n + 1) % 2) {
            optimal_moves.push_back(move);
        }
    }

    if (!optimal_moves.empty()) {
        values[optimal_moves[randint(0, optimal_moves.size())]] = n;
    } else if (!best_moves.empty()) { // Fallback to any best move if all lead to a loss (i.e. forced loss)
        values[best_moves[randint(0, best_moves.size())]] = n;
    } else { // Ultimate fallback if no moves are available
        for(int i=0; i<9; ++i) if(values[i]==2) {values[i]=n; return;}
    }
}

json get_game_state_json() {
    return {
        {"board", g_game_board},
        {"status", check_grid(g_game_board)}
    };
}

int main(void) {
    srand((unsigned)time(0));
    httplib::Server svr;

    svr.Get("/", [](const httplib::Request&, httplib::Response& res) {
        std::ifstream file("./www/index.html");
        if (!file.is_open()) {
            res.status = 404;
            res.set_content("Could not find index.html in ./www/ directory.", "text/plain");
            return;
        }
        std::stringstream buffer;
        buffer << file.rdbuf();
        res.set_content(buffer.str(), "text/html");
    });
    
    svr.Post("/api/play", [](const httplib::Request& req, httplib::Response& res) {
        std::lock_guard<std::mutex> guard(g_game_mutex);

        try {
            int move = json::parse(req.body)["move"];

            if (move >= 0 && move < 9 && g_game_board[move] == 2 && check_grid(g_game_board) == 2) {
                g_game_board[move] = 1; // Player is 'X' (1)
            } else {
                 res.status = 400;
                 res.set_content("Invalid move.", "text/plain");
                 return;
            }

            if (check_grid(g_game_board) == 2) {
                optimal_move(g_game_board, 0); // Bot is 'O' (0)
            }
            res.set_content(get_game_state_json().dump(), "application/json");
        } catch (...) {
            res.status = 400;
            res.set_content("Invalid request", "text/plain");
        }
    });

    svr.Post("/api/reset", [](const httplib::Request&, httplib::Response& res) {
        std::lock_guard<std::mutex> guard(g_game_mutex);
        
        g_game_board.assign(9, 2); // Reset board

        if (g_player_starts_next == 0) { // If it's the bot's turn to start
            optimal_move(g_game_board, 0);
        }
        
        // Alternate for the *next* game
        g_player_starts_next = (g_player_starts_next + 1) % 2; 

        res.set_content(get_game_state_json().dump(), "application/json");
    });

    std::cout << "Server starting on http://localhost:8080" << std::endl;
    svr.listen("localhost", 8080);
    return 0;
}