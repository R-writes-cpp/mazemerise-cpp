#include <fstream>
#include <vector>
#include <stack>
#include <iostream>

using namespace std;

struct point {
    int x, y;
    point (int i, int j) : x(i), y(j) {}
};

enum colour {black = 0, white = 2}; // enum that matches the .pgm format - here the .pgm format is used instead of .pbm so that the maze solver program (which uses a .pgm output) may more easily read from the maze

struct point_data {
    bool unvisited = true;
    int neighbours = 0,
        colour = black;
};

void generate_maze(vector<vector<point_data>>& lookup, const size_t& width, const size_t& height) {
    {
        bool backtracking = false; // in backtracking mode, our requirements for moving to the next pixel change. we'll permit moving to previously visited pixels in backtracking mode, assuming there aren't any neighbours to it excluding the current neighbour to it which is in our stack. backtracking mode allows us to go back and add more paths to our maze, which is essential.

        auto check_point = [&](int y, int x, vector<point>& possible_points) { // checks whether the point (x, y) on the image can be moved to
            if (lookup[y][x].neighbours++ == (backtracking ? 1 : 0) && (backtracking || lookup[y][x].unvisited))
                possible_points.push_back({x, y});
        };

        stack<point> s;
        s.push({0, 0});

        while (!s.empty()) {
            auto& x = s.top().x,
                  y = s.top().y;

            {
                auto& p_data = lookup[y][x];
                p_data.unvisited = false,
                p_data.colour = white;
            }

            vector<point> possible_points {};
            possible_points.reserve(4);

            {
                if (x != 0)
                    check_point (y, x-1, possible_points);
                if (y != 0)
                    check_point (y-1, x, possible_points);
                if (x != width - 1)
                    check_point (y, x+1, possible_points);
                if (y != height - 1)
                    check_point (y+1, x, possible_points);
            }

            if (possible_points.size()) // if there's directions we can continue in...
                s.push(possible_points[rand() % possible_points.size()]), // push a random element in the possible_points vector to the stack
                backtracking = false; // disable backtracking mode
            else // if we're at a dead end (i.e. there's no pixels we can move to) ...
                s.pop(),
                backtracking = true; // enable backtracking mode to go back through the stack
        }
    }

    if (lookup[height - 1][width - 1].colour == black) { // if the path generated has arranged the bottom-right pixel as black...
        bool move_y = rand() % 2;
        for (size_t y = height - 1, x = width - 1; x && y && lookup[y][x].colour == black; move_y ? --y : --x) // we'll "tunnel through" either the pixels above it or the pixels to the left of it so that we can reach the end
            lookup[y][x].colour = white;
    }
}

void output (char*& filename, vector<vector<point_data>>& lookup, const size_t& width, const size_t& height) {
    ofstream img (filename);

    img << "P2\n" << width + 2 << ' ' << height + 2 << "\n2\n"; // we're adding 2 to each of the dimensions as part of our black border

    img << black << ' ' << white << ' '; // set up the black border on top, ensuring that the top-left pixel has an entrance above it
    auto vertical_border = [&width, &img]() { // lambda which sets a vertical border
        for (size_t i = 0; i < width; ++i)
            img << black << ' ';
    };

    vertical_border();
    img << '\n';

    for (auto& r : lookup) {
        img << black << ' ';
        for (auto& p : r)
            img << p.colour << ' ';
        img << black << '\n';
    }

    vertical_border();
    img << white << ' ' << black;
}

int main(int argc, char** argv) { // input order: output file directory, width of desired maze, height of desired maze
    if (argc != 4) {
        cerr << "Error: bad number of inputs. Please only input an output .pgm filename and the dimensions of the output maze.\n";
        return 1;
    }

    const size_t width = atoi(argv[2]), height = atoi(argv[3]);

    srand(time(0)); // since we'll only require integer random values and we're not concerned with slightly skewed distributions, it suffices to use the rand() function from the C standard library instead of anything from the C++11 <random> header.

    vector<vector<point_data>> lookup (height, vector<point_data>(width));

    generate_maze(lookup, width, height);
    output (argv[1], lookup, width, height);
}
