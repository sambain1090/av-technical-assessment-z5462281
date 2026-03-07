#include <iostream>
#include <fstream>
#include <string>

// Question 1: This is an extension task that requires you to decode sensor data from a CAN log file.
// CAN (Controller Area Network) is a communication standard used in automotive applications (including Redback cars)
// to allow communication between sensors and controllers.
//
// Your Task: Using the definition in the Sensors.dbc file, extract the "WheelSpeedRR" values
// from the candump.log file. Parse these values correctly and store them in an output.txt file with the following format:
// (<UNIX_TIME>): <DECODED_VALUE>
// eg:
// (1705638753.913408): 1234.5
// (1705638754.915609): 6789.0
// ...
// The above values are not real numbers; they are only there to show the expected data output format.
// You do not need to use any external libraries. Use the resources below to understand how to extract sensor data.
// Hint: Think about manual bit masking and shifting, data types required,
// what formats are used to represent values, etc.
// Resources:
// https://www.csselectronics.com/pages/can-bus-simple-intro-tutorial
// https://www.csselectronics.com/pages/can-dbc-file-database-intro

void writeRRSpeed(std::ifstream& candump, std::ofstream& output);

int main() {
    std::ifstream candump("../Q1/candump.log");
    std::ofstream output("../Q1/output.txt");

    writeRRSpeed(candump, output);

    candump.close();
    output.close();

    return 0;
}

// Takes in an ifstream of the CAN dump and an output stream. It decodes the 
// rear-right wheel speed and sends it pushes it to the output stream given
void writeRRSpeed(std::ifstream& candump, std::ofstream& output) {
    std::string line;
    std::string dataString;
    uint16_t RRBytes;
    double wheelSpeedRR;

    while (getline(candump, line)) {
        if (line.length() > 42) {

            RRBytes = (std::stoi(line.substr(40, 2), nullptr, 16) << 8) | 
                (std::stoi(line.substr(38, 2), nullptr, 16));
            // Gets the hexidecimal value for the rear-right wheel speed in little
            // endian
            
            wheelSpeedRR = static_cast<int>(RRBytes) * 0.1; 
            // Scalar is 0.1, offset is 0
            // Scales the wheel speed to the correct measurement

            output << line.substr(0, 19) << ": " << wheelSpeedRR << std::endl;
            // Sends a string in this form (<UNIX_TIME>): <DECODED_VALUE> to the
            // output output stream
        }
    }
}
