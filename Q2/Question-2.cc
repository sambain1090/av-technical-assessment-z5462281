#include <iostream>
#include <vector>
#include <thread>
#include <memory>
#include <iomanip>
#include <atomic>

// Constant expressions
constexpr uint8_t SIMPLE = 0x00;
constexpr uint8_t COMPLEX = 0x01;

template<typename T>
class ThreadSafeQueue {
private:
    std::vector<T> q;
    std::mutex mtx;

public:
    void push(T value) {
        std::lock_guard<std::mutex> lock(mtx);
        q.push_back(std::move(value));
    }   

    T pop() {
        std::lock_guard<std::mutex> lock(mtx);
        if (q.empty()) {
            return nullptr;
        }

        T value = std::move(q.front());
        q.erase(q.begin());
        return value;
    }

    size_t size() {
        std::lock_guard<std::mutex> lock(mtx);
        return q.size();
    }

    // A non-blocking pop for graceful shutdown
    T pop_for_shutdown() {
        if (!mtx.try_lock()) {
            return nullptr;
        }

        std::lock_guard<std::mutex> lock(mtx, std::adopt_lock);

        if (q.empty()) {
            return nullptr;
        }

        T value = std::move(q.front());
        q.erase(q.begin());
        return value;
    }
};

// Class Representation of a Task
class ITask {
public:
    virtual ~ITask() = default;
    virtual void process() = 0;
    virtual float getProcessedValue() const = 0;
    virtual uint8_t getTaskType() const = 0;
};

// Takes a float input and if processed, doubles the value
class SimpleTask : public ITask {
private:
    float iVal;
    float pVal;

public:
    explicit SimpleTask(float val): iVal(val), pVal(0) {}

    void process() override {
        pVal = iVal * 2.0f;
    }

    float getProcessedValue() const override {
        return pVal;
    }

    uint8_t getTaskType() const override {
        return SIMPLE;
    }
};

// Takes a vector of integers as input and if processed, sums all the elements
// into a integer value
class ComplexTask : public ITask {
private:
    std::vector<int> iNums;
    float pSum;

public:
    explicit ComplexTask(std::vector<int> nums): iNums(nums), pSum(0) {}

    void process() override {
        for (unsigned long i{0}; i < iNums.size(); ++i) {
            pSum += iNums.at(i);
        }
    }

    float getProcessedValue() const override {
        return pSum;
    }

    uint8_t getTaskType() const override {
        return COMPLEX;
    }
};


class TaskGenerator {
private:
    ThreadSafeQueue<std::unique_ptr<ITask>>& task_queue_;
    std::atomic<bool>& shutdown_;

public:
    TaskGenerator(ThreadSafeQueue<std::unique_ptr<ITask>>& queue, std::atomic<bool>& shutdown)
        : task_queue_(queue), shutdown_(shutdown) {}
    void run() {
        addTask(20.0f);
        addTask(std::vector<int>( { 20 } ));
        addTask(1.234F);
        addTask(std::vector<int>( { 1, 3, 4, -1, 3, 93, 1013, 11} ));
        addTask(1.234F);
        addTask(std::vector<int>( { 1, 3, 4, -1, 3 } ));
        while (!shutdown_) { }
    }

    // Added a function to automatically create a simple or complex task based
    // on input using function overloading. If the input is a float it creates
    // a simple task and adds it to the queue, if its a vector of integers 
    // it creates a complex task and pushes it.
    void addTask(float value) {
        task_queue_.push(std::make_unique<SimpleTask>(SimpleTask(value)));
    }

    void addTask(std::vector<int> nums) {
        task_queue_.push(std::make_unique<ComplexTask>(ComplexTask(nums)));
    }
};

class TaskProcessor {
private:
    ThreadSafeQueue<std::unique_ptr<ITask>>& task_queue_;
    ThreadSafeQueue<std::unique_ptr<ITask>>& processed_queue_;
    std::atomic<bool>& shutdown_;

public:
    TaskProcessor(ThreadSafeQueue<std::unique_ptr<ITask>>& t_queue, ThreadSafeQueue<std::unique_ptr<ITask>>& p_queue, std::atomic<bool>& shutdown)
        : task_queue_(t_queue), processed_queue_(p_queue), shutdown_(shutdown) {}
    
    void run() {
        while (!shutdown_) {
            if (task_queue_.size() > 0) {
                std::unique_ptr<ITask> currentTask = task_queue_.pop();
                currentTask->process();
                processed_queue_.push(std::move(currentTask));
            }
        }   

        while (std::unique_ptr<ITask> currentTask = task_queue_.pop_for_shutdown()) {
            currentTask->process();
            processed_queue_.push(std::move(currentTask));
        }
    }
};  

class PacketTransmitter {
private:
    ThreadSafeQueue<std::unique_ptr<ITask>>& processed_queue_;
    std::atomic<bool>& shutdown_;
public:
    PacketTransmitter(ThreadSafeQueue<std::unique_ptr<ITask>>& queue, std::atomic<bool>& shutdown)
        : processed_queue_(queue), shutdown_(shutdown) {}

    void run(std::ostream& os) {
        while (!shutdown_) {
            if (processed_queue_.size() > 0) {
                std::unique_ptr<ITask> data = processed_queue_.pop();;
                transmit(data, os);
            }
        }

        while (std::unique_ptr<ITask> data = processed_queue_.pop_for_shutdown()) {
                transmit(data, os);
        }
    }

    void transmit(const std::unique_ptr<ITask>& data, std::ostream& os) {
        uint8_t buffer[8] = {0};

        // sets the first byte to the task byte representation
        buffer[0] |= data->getTaskType();
        float processedValue = data->getProcessedValue();

        // copies the data into 4 bytes of the buffer starting from the first
        // second array position
        std::memcpy(&buffer[1], &processedValue, sizeof(float));
        std::reverse(&buffer[1], &buffer[5]); // convert to big endian

        uint32_t timeStamp = static_cast<uint32_t>(std::time(nullptr));
        
        // adds the lowest 24 bits of unix time to the end of the buffer
        buffer[5] |= timeStamp >> 16 & 0xFF;
        buffer[6] |= timeStamp >> 8 & 0xFF;
        buffer[7] |= timeStamp & 0xFF;

        // Print the buffer in hex format for verification
        os << "Packet: ";
        for (int i = 0; i < 8; ++i) {
            os << "0x" << std::setw(2) << std::setfill('0') << std::hex << (int)buffer[i] << " ";
        }
        os << std::dec << std::endl;
    }
};

#ifndef TESTING

int main() {
    std::cout << "Starting the data generation pipeline" << std::endl;

    std::atomic<bool> shutdown_flag{false};

    ThreadSafeQueue<std::unique_ptr<ITask>> task_queue;
    ThreadSafeQueue<std::unique_ptr<ITask>> processed_queue;

    TaskGenerator generator(task_queue, shutdown_flag);
    TaskProcessor processor(task_queue, processed_queue, shutdown_flag);
    PacketTransmitter transmitter(processed_queue, shutdown_flag);

    std::thread generator_thread(&TaskGenerator::run, &generator);
    std::thread processor_thread(&TaskProcessor::run, &processor);
    std::thread transmitter_thread(&PacketTransmitter::run, &transmitter, std::ref(std::cout));

    std::this_thread::sleep_for(std::chrono::seconds(10));

    shutdown_flag = true;

    generator_thread.join();
    processor_thread.join();
    transmitter_thread.join();

    std::cout << "Data Gen pipeline Finished." << std::endl;

    return 0;
}

#endif