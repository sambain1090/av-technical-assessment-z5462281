#include <gtest/gtest.h>
#include <vector>
#include <memory>
#include <sstream>
#include <iomanip>
#include <cstring>

// We include the implementation file to get access to the classes.
// In a real project, you would have separate header files.
#include "Question-2.cc"

// Sample test case for SimpleTask processing
TEST(SampleTest, SimpleTaskProcessing) {
    float initial_value = 12.5f;
    SimpleTask task(initial_value);

    task.process();

    float expected_value = 25.0f;
    EXPECT_FLOAT_EQ(task.getProcessedValue(), expected_value);
    EXPECT_EQ(task.getTaskType(), 0x00);
}

// Simple Task Tests, add tuples for the input value, expected output value, and expected hexadecimal representation
class SimpleTaskTest : public testing::TestWithParam<std::tuple<float, float, std::string>> {};

TEST_P(SimpleTaskTest, ProcessingTests) {
    std::tuple input = GetParam();
    SimpleTask task(std::get<0>(input));
    task.process();
    EXPECT_FLOAT_EQ(task.getProcessedValue(), std::get<1>(input));

    std::ostringstream output;
    ThreadSafeQueue<std::unique_ptr<ITask>> queue;
    std::atomic<bool> shutdownFlag{false};

    const std::unique_ptr<ITask>& taskPtr(std::make_unique<SimpleTask>(task));

    PacketTransmitter transmitter(queue, shutdownFlag);
    transmitter.transmit(taskPtr, output);

    EXPECT_EQ(output.str().substr(8, 24), std::get<2>(input));
}

INSTANTIATE_TEST_SUITE_P(SimpleTaskValues, SimpleTaskTest, 
    testing::Values(
        std::make_tuple(0.0f, 0.0f, "0x00 0x00 0x00 0x00 0x00"),                       // Float of 0
        std::make_tuple(10000000000.0f, 20000000000.0f, "0x00 0x50 0x95 0x02 0xf9"),   // Large value
        std::make_tuple(1.8193498f, 3.6386996f, "0x00 0x40 0x68 0xe0 0x74"),           // Large decimal value
        std::make_tuple(-10000000000.0f, -20000000000.0f, "0x00 0xd0 0x95 0x02 0xf9"), // Large negative value
        std::make_tuple(-1.8193498f, -3.6386996, "0x00 0xc0 0x68 0xe0 0x74")           // Large negative decimal value
    ));

// Complex Task Tests, add tuples for the input value, expected output value, and expected hexadecimal representation
class ComplexTaskTest : public testing::TestWithParam<std::tuple<std::vector<int>, float, std::string>> {};

TEST_P(ComplexTaskTest, Processing) {
    std::tuple input = GetParam();
    ComplexTask task(std::get<0>(input));
    task.process();
    EXPECT_FLOAT_EQ(task.getProcessedValue(), std::get<1>(input));
    EXPECT_EQ(task.getTaskType(), 0x01);

    std::ostringstream output;
    ThreadSafeQueue<std::unique_ptr<ITask>> queue;
    std::atomic<bool> shutdownFlag{false};

    const std::unique_ptr<ITask>& taskPtr(std::make_unique<ComplexTask>(task));

    PacketTransmitter transmitter(queue, shutdownFlag);
    transmitter.transmit(taskPtr, output);

    EXPECT_EQ(output.str().substr(8, 24), std::get<2>(input));
};


INSTANTIATE_TEST_SUITE_P(ComplexTaskValues, ComplexTaskTest, 
    testing::Values(
        std::make_tuple(std::vector<int>({ }), 0, "0x01 0x00 0x00 0x00 0x00"),                                           // Empty vector
        std::make_tuple(std::vector<int>({0}), 0, "0x01 0x00 0x00 0x00 0x00"),                                           // Vector with only a zero
        std::make_tuple(std::vector<int>({1, 2, 3, 4, 5, 6, 7, 8, 9, 10}), 55, "0x01 0x42 0x5c 0x00 0x00"),              // Vector with all positive numbers
        std::make_tuple(std::vector<int>({-1, -2, -3, -4, -5, -6, -7, -8, -9, -10}), -55, "0x01 0xc2 0x5c 0x00 0x00"),   // Vector with all negative numbers
        std::make_tuple(std::vector<int>({3, -3, 1, -5, -8}), -12, "0x01 0xc1 0x40 0x00 0x00"),                          // Vector with a mix of positive and negative numbers
        std::make_tuple(std::vector<int>({333333, 333333}), 666666, "0x01 0x49 0x22 0xc2 0xa0")                          // Large processed value         
));