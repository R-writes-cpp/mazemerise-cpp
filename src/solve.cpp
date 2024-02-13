#include <fstream>
#include <vector>
#include <stack>
#include <string>
#include <iostream>

using namespace std;

struct point {
    int x, y;
    point (int i, int j) : x(i), y(j) {}
};

enum colour {black = 0, grey = 1, white = 2}; // we'll be using the .pgm format here - black pixels represent maze walls, white pixels represent potential paths, and grey pixels represent the path from the start to the end of the maze

struct point_data {
    bool leads_to_dead_end = false;
    int colour = black;
};

int main(int argc, char** argv) { // input order: input file directory, output file directory
    vector<vector<point_data>> lookup;
    int width, height; // these represent the width and height of the maze, excluding its border

    {
        ifstream img (argv[1]); // input .pgm image
        string str;
        getline (img, str), getline (img, str); // skips the first line "P2" and moves onto the second line with the dimensions of the image

        try {
            int img_width; // represents the width of the original image, including its border. a variable different from "width" is used for clarity.
            size_t i = str.find_first_of(' ');
            img_width = stoi (str, &i);
            str.reserve (2 * img_width); // max length of each row from now on will be 2 times the width (including the spacing between each pixel and the line break)

            width = img_width - 2,
            height = stoi (string (str.begin() + i + 1, str.end())) - 2;
        }

        catch (...) { // if there's any errors with stoi...
            cerr << "Error: invalid input.\n";
            return 0;
        }

        getline (img, str), getline (img, str), getline (img, str); // skips the number of shades (which is already known assuming a valid input has been provided) and the border, then moves onto the first row with unique data

        lookup = vector<vector<point_data>> (height, vector<point_data> (width));
        for (int y = 0; y < height; ++y, getline (img, str))
            for (int x = 0; x < width; ++x)
                if (auto current = str[2 * (x + 1)] - '0'; current != 0 && current != 2) { // input images must only consist of 0s and 2s (and spaces, but we're skipping over those with 2 * (x + 1))
                    cerr << "Error: invalid input.\n";
                    return 0;
                } else
                    lookup[y][x].colour = current;
    }

    if (lookup[0][0].colour == black || lookup[height - 1][width - 1].colour == black) {
        cerr << "Error: invalid maze provided.\n";
        return 0;
    }

    {
        bool path_found = false;

        {
            stack<point> s; // stack which uses a standard depth-first search algorithm for maze solving
            s.push({0, 0});

            while (s.size()) {
                auto& x = s.top().x,
                    y = s.top().y;
                auto& current = lookup[y][x];

                if (x == width - 1 && y == height - 1) { // i.e. "have we reached the end of the maze?"
                    current.colour = grey,
                    path_found = true;
                    break;
                }

                // NOTE: on average, it is significantly faster to prioritise going right and going down instead of going left and going up - this is because these directions "bring us closer" to the end point (i.e. they decrease the Euclidean distance between the next point and the end point), and when we're closer to the end, we're generally more likely to be on an end-reaching path
                auto next_point = (x != width - 1 && lookup[y][x+1].colour == white && !lookup[y][x+1].leads_to_dead_end)  ? point {x+1, y} :
                                (y != height - 1 && lookup[y+1][x].colour == white && !lookup[y+1][x].leads_to_dead_end) ? point {x, y+1} :
                                (x != 0 && lookup[y][x-1].colour == white && !lookup[y][x-1].leads_to_dead_end)          ? point {x-1, y} :
                                (y != 0 && lookup[y-1][x].colour == white && !lookup[y-1][x].leads_to_dead_end)          ? point {x, y-1} :
                                point {-1, -1}; // final value assuming no there are no available neighbours

                if (next_point.x == -1 && next_point.y == -1) // if there are no available neighbours...
                    current.colour = white, // then the current point isn't part of our path to the end and should be set to white
                    current.leads_to_dead_end = true,
                    s.pop();
                else // if there are available neighbours...
                    current.colour = grey, // then the current point is part of our path to the end (for now)
                    s.push(next_point);
            }
        }

        if (!path_found) {
            cout << "No path found.\n";
            return 0;
        }
    }

    ofstream img (argv[2]); // output image

    img << "P2\n" << width + 2 << ' ' << height + 2 << "\n2\n"; // 3 different greyscale values means we have black, grey and white (represented by 0, 1 and 2 respectively). this is all we need.

    img << black << ' ' << grey << ' '; // set up the black border on top, ensuring that the top-left pixel has an entrance above it
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
    img << grey << ' ' << black;
}
