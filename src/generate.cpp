#include <fstream>
#include <vector>
#include <stack>
#include <iostream>

using namespace std;

struct point {
    int x, y;
    point (int i, int j) : x(i), y(j) {}
};

enum colour {white = false, black = true}; // enum that matches the .pbm format

struct point_data {
    bool unvisited = true, colour = black;
    int neighbours = 0;
};

enum dir {left = 0, up = 1, right = 2, down = 3};

int main(int argc, char** argv) {
    const int width = atoi(argv[1]), height = atoi(argv[2]);

    if (width <= 0 || height <= 0) {
        cerr << "Error: dimensions entered are non-positive.\n";
        return 0;
    }

    srand(time(0)); // since we'll only require integer random values and we're not concerned with slightly imperfect distributions, it suffices to use the rand() function from the C standard library instead of anything from the C++11 <random> header.

    vector<vector<point_data>> lookup (height, vector<point_data>(width));

    {
        stack<point> s;
        s.push({0, 0});

        bool backtracking = false; // in backtracking mode, our requirements for moving to the next pixel change. we'll permit moving to previously visited pixels in backtracking mode, assuming there aren't any neighbours to it excluding the current neighbour to it which is in our stack. backtracking mode allows us to go back and add more paths to our maze, which is essential.

        while (!s.empty()) {
            auto& x = s.top().x,
                y = s.top().y;

            {
                auto& p_data = lookup[y][x];
                p_data.unvisited = false,
                p_data.colour = white;
            }

            vector<int> possible_directions {};

            if (x != 0 && lookup[y][x-1].neighbours++ == (backtracking ? 1 : 0) && (backtracking || lookup[y][x-1].unvisited)) // the order here matters. first of all, we need to make sure x != 0 so that the program doesn't segfault when we access the left neighbour pixel. second of all we need to increment the neighbours count of the current pixel, assuming it exists, then check if we're either backtracking or that the left neighbour pixel is unvisited. if we do the unvisited check before the neighbours check we won't increment the neighbours counter, which leads to invalid mazes.
                possible_directions.push_back(dir::left);

            if (y != 0 && lookup[y-1][x].neighbours++ == (backtracking ? 1 : 0) && (backtracking || lookup[y-1][x].unvisited))
                possible_directions.push_back(up);

            if (x != width - 1 && lookup[y][x+1].neighbours++ == (backtracking ? 1 : 0) && (backtracking || lookup[y][x+1].unvisited))
                possible_directions.push_back(dir::right);

            if (y != height - 1 && lookup[y+1][x].neighbours++ == (backtracking ? 1 : 0) && (backtracking || lookup[y+1][x].unvisited))
                possible_directions.push_back(down);

            if (possible_directions.size()) { // if there's directions we can continue in...
                backtracking = false;
                switch (possible_directions[rand() % possible_directions.size()]) { // select a random direction available to us, push it to the top of the stack and mark it as visited
                    case dir::left:
                        s.push({x - 1, y}),
                        lookup[y][x-1].unvisited = false;
                        break;
                    case up:
                        s.push({x, y - 1}),
                        lookup[y-1][x].unvisited = false;
                        break;
                    case dir::right:
                        s.push({x + 1, y}),
                        lookup[y][x+1].unvisited = false;
                        break;
                    case down:
                        s.push({x, y + 1}),
                        lookup[y+1][x].unvisited = false;
                }
            } else // if we're at a dead end (i.e. there's no pixels we can move to) ...
                backtracking = true,
                s.pop();
        }
    }

    if (lookup[height - 1][width - 1].colour == black) { // if the path generated has arranged the bottom-right pixel as
        bool move_y = rand() % 2;
        for (size_t y = height - 1, x = width - 1; x && y && lookup[y][x].colour == black; move_y ? --y : --x)
            lookup[y][x].colour = white;
    }

    ofstream img ("img.pbm");

    img << "P1\n" << width + 2 << ' ' << height + 2 << '\n'; // we're adding 2 to each of the dimensions as part of our black border

    img << black << ' ' << white << ' '; // set up the black border on top, ensuring that the top-left pixel has an entrance above it
    for (int i = 0; i < width; ++i)
        img << black << ' ';
    img << '\n';

    for (auto& r : lookup) {
        img << black << ' ';
        for (auto& p : r)
            img << p.colour << ' ';
        img << black << '\n';
    }

    for (int i = 0; i < width; ++i) // set up the black border on the bottom, ensuring that the bottom-right pixel has an exit below it
        img << black << ' ';
    img << white << ' ' << black;
}
