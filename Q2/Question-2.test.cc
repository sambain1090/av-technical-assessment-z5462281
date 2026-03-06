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

    float expected_value = 26.0f;
    EXPECT_FLOAT_EQ(task.getProcessedValue(), expected_value);
    EXPECT_EQ(task.getTaskType(), 0);
}
