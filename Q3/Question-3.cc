#include <iostream>
#include <fstream>
#include <string>

// Please also try document your thought process and your code as you go!

// Currently looking to figure out how the csv is arranged

struct Point {
    double x, y;
};

int main () {
    //Will start just making the readfiles
    std::ifstream track("../Q3/track1.log");
    std::ofstream waypoints("../Q3/waypoints.txt");

    std::string line;

    getline(track, line); // reads the line saying "x,y" to flush out

    while (getline(track, line)) { // decided to read the hints after setting this up
        
    }

    track.close();
    waypoints.close();
    return 0;
}