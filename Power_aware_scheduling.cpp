#include <iostream>
#include <vector>
#include <queue>
#include <numeric>
#include <unistd.h>
#include <chrono>
#include <iomanip>
#include <thread>
#include <random>
#include <limits> // Required for numeric_limits

using namespace std; // Added using namespace std;

enum class Priority {
    HIGH,
    LOW
};

struct Task {
    int id;
    Priority priority;
    int burst_time;
    int remaining_time;
    bool is_sleeping;
    chrono::time_point<chrono::high_resolution_clock> arrival_time;
    chrono::time_point<chrono::high_resolution_clock> start_time;
    chrono::time_point<chrono::high_resolution_clock> finish_time;

    Task(int id, Priority priority, int burst_time) :
        id(id), priority(priority), burst_time(burst_time), remaining_time(burst_time), is_sleeping(false) {
        arrival_time = chrono::high_resolution_clock::now();
    }
};

void execute_task(Task& task, int time_slice) {
    if (task.remaining_time <= 0) {
        return;
    }

    int execution_time = min(task.remaining_time, time_slice);
    task.remaining_time -= execution_time;

    this_thread::sleep_for(chrono::milliseconds(execution_time));
}

int main() {
    queue<Task> ready_queue;
    vector<Task> finished_tasks;

    int num_tasks;
    int time_slice_high;
    int time_slice_low;
    int sleep_duration_min;
    int sleep_duration_max;
    double running_power;
    double sleeping_power;
    double idle_power;


    cout << "=======================================================\n";
    cout << "      Power-Aware Scheduling Simulation - Setup      \n";
    cout << "=======================================================\n";

     // Get simulation parameters from the user
    cout << "Enter the number of tasks: ";
    cin >> num_tasks;
    while(cin.fail() || num_tasks <=0){
        cout << "Invalid input. Please enter a positive integer for the number of tasks: ";
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cin >> num_tasks;
    }


    cout << "Enter time slice for HIGH priority tasks (ms): ";
    cin >> time_slice_high;
    while(cin.fail() || time_slice_high <=0){
        cout << "Invalid input. Please enter a positive integer for time slice: ";
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cin >> time_slice_high;
    }

    cout << "Enter time slice for LOW priority tasks (ms): ";
    cin >> time_slice_low;
    while(cin.fail() || time_slice_low <=0){
        cout << "Invalid input. Please enter a positive integer for time slice: ";
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cin >> time_slice_low;
    }

    cout << "Enter minimum sleep duration (ms): ";
    cin >> sleep_duration_min;
    while(cin.fail() || sleep_duration_min <=0){
        cout << "Invalid input. Please enter a positive integer for sleep duration: ";
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cin >> sleep_duration_min;
    }


    cout << "Enter maximum sleep duration (ms): ";
    cin >> sleep_duration_max;
    while(cin.fail() || sleep_duration_max <= sleep_duration_min){
        cout << "Invalid input. Please enter an integer greater than minimum sleep duration: ";
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cin >> sleep_duration_max;
    }


    cout << "Enter running power consumption (arbitrary units): ";
    cin >> running_power;
     while(cin.fail() || running_power <= 0){
        cout << "Invalid input. Please enter a positive value for running power: ";
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cin >> running_power;
    }

    cout << "Enter sleeping power consumption (arbitrary units): ";
    cin >> sleeping_power;
    while(cin.fail() || sleeping_power <=0 || sleeping_power >= running_power){
        cout << "Invalid input. Please enter a positive value less than running power: ";
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cin >> sleeping_power;
    }

    cout << "Enter idle power consumption (arbitrary units): ";
    cin >> idle_power;
    while(cin.fail() || idle_power <=0 || idle_power >= running_power){
        cout << "Invalid input. Please enter a positive value less than running power: ";
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cin >> idle_power;
    }


    // Get task details from the user
    for (int i = 1; i <= num_tasks; ++i) {
        int burst_time;
        int priority_input;
        Priority priority;

        cout << "\nEnter details for Task " << i << ":\n";
        cout << "  Burst Time (ms): ";
        cin >> burst_time;
        while(cin.fail() || burst_time <= 0){
            cout << "    Invalid input. Please enter a positive integer for burst time: ";
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cin >> burst_time;
        }

        cout << "  Priority (0 for HIGH, 1 for LOW): ";
        cin >> priority_input;
        while(cin.fail() || (priority_input != 0 && priority_input != 1)){
            cout << "    Invalid input. Please enter 0 for HIGH or 1 for LOW priority: ";
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cin>>priority_input;
        }
        priority = (priority_input == 0) ? Priority::HIGH : Priority::LOW;

        ready_queue.push(Task(i, priority, burst_time));
    }



    double total_power_consumption = 0.0;
    bool cpu_idle = false;
    int tick = 0;

    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> sleep_distrib(sleep_duration_min, sleep_duration_max);

    cout << "\n=======================================================\n";
    cout << "      Power-Aware Scheduling Simulation - Start      \n";
    cout << "=======================================================\n";
    cout << "  Time  |  CPU State  |  Task Info (ID, Priority, Remaining) | Power Consumed\n";
    cout << "-------------------------------------------------------\n";



    while (!ready_queue.empty() || cpu_idle) {
        cpu_idle = true;
        tick++;

        bool high_priority_task_found = false;
        queue<Task> next_ready_queue;

        queue<Task> temp_queue = ready_queue;
        while (!temp_queue.empty()) {
            Task current_task = temp_queue.front();
            temp_queue.pop();
            if (current_task.priority == Priority::HIGH) {
                high_priority_task_found = true;
                break;
            }
        }

        if (high_priority_task_found) {
            cpu_idle = false;

            queue<Task> processing_queue = ready_queue;
            ready_queue = queue<Task>();

            while (!processing_queue.empty()) {
                Task current_task = processing_queue.front();
                processing_queue.pop();

                if (current_task.priority == Priority::HIGH) {
                    if (!current_task.start_time.time_since_epoch().count()) {
                        current_task.start_time = chrono::high_resolution_clock::now();
                    }

                    double power_this_tick = running_power * (time_slice_high / 1000.0);
                    total_power_consumption += power_this_tick;

                    cout << setw(6) << tick << "  |  Running    |  Task " << current_task.id << " (HIGH, "
                              << setw(4) << current_task.remaining_time << "ms)        | "
                              << fixed << setprecision(4) << power_this_tick << "\n";


                    execute_task(current_task, time_slice_high);

                    if (current_task.remaining_time > 0) {
                        ready_queue.push(current_task);
                    } else {
                        current_task.finish_time = chrono::high_resolution_clock::now();
                        finished_tasks.push_back(current_task);
                        cout << setw(6) << tick << "  |  Finished   |  Task " << current_task.id << " (HIGH)                 |  ---\n";
                    }
                } else {
                    next_ready_queue.push(current_task);
                }
            }
        } else {
            if (!ready_queue.empty()) {
                cpu_idle = false;

                Task& current_low_priority_task = ready_queue.front();
                ready_queue.pop();

                if (!current_low_priority_task.start_time.time_since_epoch().count()) {
                    current_low_priority_task.start_time = chrono::high_resolution_clock::now();
                }
                double power_this_tick;

                if (!current_low_priority_task.is_sleeping) {

                    power_this_tick = running_power * (time_slice_low / 1000.0);
                    total_power_consumption += power_this_tick;
                    cout << setw(6) << tick << "  |  Running    |  Task " << current_low_priority_task.id << " (LOW, "
                              << setw(4) << current_low_priority_task.remaining_time << "ms)        | "
                              << fixed << setprecision(4) << power_this_tick << "\n";


                    execute_task(current_low_priority_task, time_slice_low);

                } else {

                    power_this_tick = 0; // Power is accounted for when entering sleep
                     cout << setw(6) << tick << "  |  Waking Up  |  Task " << current_low_priority_task.id << " (LOW, "
                              << setw(4) << current_low_priority_task.remaining_time << "ms)        | "
                              << fixed << setprecision(4) << power_this_tick << "\n";

                    current_low_priority_task.is_sleeping = false;
                    execute_task(current_low_priority_task, time_slice_low);
                    total_power_consumption += running_power * (time_slice_low / 1000.0);

                }

                if (current_low_priority_task.remaining_time > 0) {
                    bool any_high_priority_waiting = false;
                    queue<Task> temp_check_queue = next_ready_queue;
                    while(!temp_check_queue.empty()){
                        if(temp_check_queue.front().priority == Priority::HIGH){
                            any_high_priority_waiting = true;
                            break;
                        }
                        temp_check_queue.pop();
                    }

                    if (!any_high_priority_waiting) {
                        int sleep_duration = sleep_distrib(gen);
                        power_this_tick = sleeping_power * (sleep_duration / 1000.0);
                        total_power_consumption += power_this_tick;

                        cout << setw(6) << tick << "  |  Sleeping   |  Task " << current_low_priority_task.id << " (LOW) for "
                                  << setw(4) << sleep_duration << "ms     | "
                                  << fixed << setprecision(4) << power_this_tick << "\n";


                        current_low_priority_task.is_sleeping = true;
                        this_thread::sleep_for(chrono::milliseconds(sleep_duration));
                    }
                     else{
                        //If there are high priority tasks, we don't display sleep info.
                        power_this_tick = 0; //Reset to 0, since the task didn't sleep.

                    }

                    ready_queue.push(current_low_priority_task);

                } else {
                    current_low_priority_task.finish_time = chrono::high_resolution_clock::now();
                    finished_tasks.push_back(current_low_priority_task);
                    cout << setw(6) << tick << "  |  Finished   |  Task " << current_low_priority_task.id << " (LOW)                  |  ---\n";
                }
            } else {
                cpu_idle = true;
                double power_this_tick = idle_power * (sleep_duration_min / 1000.0);
                total_power_consumption += power_this_tick;

                cout << setw(6) << tick << "  |  Idle       |  ---                             | "
                          << fixed << setprecision(4) << power_this_tick << "\n";

                this_thread::sleep_for(chrono::milliseconds(sleep_duration_min));
            }
        }
        while(!next_ready_queue.empty()){
            ready_queue.push(next_ready_queue.front());
            next_ready_queue.pop();
        }
    }

   cout << "-------------------------------------------------------\n";
    cout << "                    Simulation End                     \n";
    cout << "=======================================================\n";


    cout << "\nTask Execution Summary:\n";
    cout << "-----------------------------------------\n";
    cout << "Task ID | Priority | Burst Time | Turnaround Time | Waiting Time\n";
    cout << "-----------------------------------------\n";

    double total_turnaround_time = 0;
    double total_waiting_time = 0;

    for (const auto& task : finished_tasks) {
        auto turnaround_time = chrono::duration_cast<chrono::milliseconds>(task.finish_time - task.arrival_time).count();
        auto waiting_time = chrono::duration_cast<chrono::milliseconds>(task.start_time - task.arrival_time).count();

        total_turnaround_time += turnaround_time;
        total_waiting_time += waiting_time;

        cout << setw(7) << task.id << " | " << setw(8) << (task.priority == Priority::HIGH ? "HIGH" : "LOW")
                  << " | " << setw(10) << task.burst_time << "ms | "
                  << setw(15) << turnaround_time << "ms | " << setw(12) << waiting_time << "ms\n";
    }
     cout << "-----------------------------------------\n";


    if (!finished_tasks.empty()) {
        cout << "\nAverage Turnaround Time: " << fixed << setprecision(2) << total_turnaround_time / finished_tasks.size() << "ms\n";
        cout << "Average Waiting Time: " << fixed << setprecision(2) << total_waiting_time / finished_tasks.size() << "ms\n";
    } else {
        cout << "No tasks were executed.\n";
    }

    cout << "\nPower Consumption Summary:\n";
    cout << "--------------------------\n";
    cout << "Total Power Consumed: " << fixed << setprecision(4) << total_power_consumption << " units\n";

    return 0;
}
