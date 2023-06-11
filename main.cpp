#include <algorithm>
#include <fstream>
#include <iostream>
#include <queue>
#include <sstream>
#include <vector>
#include <unistd.h>

using namespace std;

struct Process {
  int id;
  int arrival;
  int burst;
  int remaining;
  int service_time;
  int arrival_time;
  int start_time;
  int end_time;
};
bool compare_arrival(const Process& p1, const Process& p2) {
  return p1.arrival < p2.arrival;
}
void fcfs_scheduling(Process processes[], int n) {
  queue<Process> ready_queue;      // queue to hold the ready processes
  int current_time = 0;            // current time
  float avg_wait_time = 0.0;       // average waiting time
  float avg_turnaround_time = 0.0; // average turnaround time

  ofstream outfile("fcfs.out"); // create an output file

  for (int i = 0; i < n; i++) {
    // add each process to the ready queue in order of arrival time
    ready_queue.push(processes[i]);
  }

  while (!ready_queue.empty()) {
    // get the next process from the front of the queue
    Process current_process = ready_queue.front();
    ready_queue.pop();

    // update the current time to the arrival time of the current process
    current_time = max(current_time, current_process.arrival);

    // calculate the waiting time for the current process
    int wait_time = current_time - current_process.arrival;
    avg_wait_time += wait_time;

    // update the current time to the end of the current process's burst time
    current_time += current_process.burst;

    // calculate the turnaround time for the current process
    int turnaround_time = current_time - current_process.arrival;
    avg_turnaround_time += turnaround_time;

    // print the completion time and waiting time for the current process
    outfile << "Process " << current_process.id
         << ": completion time = " << current_time
         << ", waiting time = " << wait_time << endl;
  }

  // calculate the average waiting and turnaround times
  avg_wait_time /= n;
  avg_turnaround_time /= n;

  // print the average waiting and turnaround times to the output file
  outfile << "Average waiting time = " << avg_wait_time << endl;
  outfile << "Average turnaround time = " << avg_turnaround_time << endl;

  // close the output file
  outfile.close();
}


void round_robin_scheduling(Process processes[], int n, int quantum, ofstream& outfile) {
  queue<Process> ready_queue;      // queue to hold the ready processes
  int current_time = 0;            // current time
  float avg_wait_time = 0.0;       // average waiting time
  float avg_turnaround_time = 0.0; // average turnaround time

  if (quantum == 10) {
    outfile.open("rr_10.out");
  } else if (quantum == 40) {
    outfile.open("rr_40.out");
  }
  
  for (int i = 0; i < n; i++) {
    processes[i].remaining = processes[i].burst;
    ready_queue.push(processes[i]);
  }

  while (!ready_queue.empty()) {
    Process current_process = ready_queue.front();
    ready_queue.pop();

    int execute_time = min(current_process.remaining, quantum);
    current_time += execute_time;
    current_process.remaining -= execute_time;

    if (current_process.remaining > 0) {
      ready_queue.push(current_process);
    } else {
      int wait_time = current_time - current_process.arrival -
                      current_process.burst + current_process.remaining;
      avg_wait_time += wait_time;
      int turnaround_time = current_time - current_process.arrival;
      avg_turnaround_time += turnaround_time;

      outfile << "Process " << current_process.id
              << ": completion time = " << current_time
              << ", waiting time = " << wait_time << endl;
    }
  }

  // calculate the average waiting and turnaround times
  avg_wait_time /= n;
  avg_turnaround_time /= n;

  // print the average waiting and turnaround times to the output file
  outfile << "Average waiting time = " << avg_wait_time << endl;
  outfile << "Average turnaround time = " << avg_turnaround_time << endl;

  outfile.close(); // closing the output file stream
}

void shortest_process_next(Process processes[], int n) {
    priority_queue<pair<int, int>, vector<pair<int, int>>, greater<pair<int, int>>> ready_queue;
    int current_time = 0;
    int *remaining_time = new int[n];
    bool *completed = new bool[n];
    int *waiting_time = new int[n];
    int total_waiting_time = 0;
    int total_turnaround_time = 0;

    for (int i = 0; i < n; i++) {
        remaining_time[i] = processes[i].burst;
        completed[i] = false;
        waiting_time[i] = 0;
    }

    while (true) {
        bool all_completed = true;

        for (int i = 0; i < n; i++) {
            if (!completed[i]) {
                all_completed = false;

                if (processes[i].arrival <= current_time) {
                    ready_queue.push(make_pair(processes[i].burst, i));
                }
            }
        }

        if (all_completed) {
            break;
        }

        if (ready_queue.empty()) {
            current_time++;
            continue;
        }

        pair<int, int> current_process = ready_queue.top();
        ready_queue.pop();

        int remaining_burst = remaining_time[current_process.second];

        if (remaining_burst == current_process.first) {
            waiting_time[current_process.second] = current_time - processes[current_process.second].arrival;
        }

        current_time += current_process.first;
        remaining_time[current_process.second] = 0;
        completed[current_process.second] = true;

        int turnaround_time = current_time - processes[current_process.second].arrival;
        total_waiting_time += waiting_time[current_process.second];
        total_turnaround_time += turnaround_time;
    }

    float avg_waiting_time = (float)total_waiting_time / n;
    float avg_turnaround_time = (float)total_turnaround_time / n;

    // write output to file
    ofstream outfile("spn.out");
    outfile << "Average Waiting Time: " << avg_waiting_time << endl;
    outfile << "Average Turnaround Time: " << avg_turnaround_time << endl;
    outfile.close();

    delete[] remaining_time;
    delete[] completed;
    delete[] waiting_time;
}



/*void HRRN(Process processes[], int n, int quantum, int output[]) {
  int current_time = 0;
  int remaining_processes = n;
  vector<int> ready_queue;
  vector<int> response_ratio;
  vector<int> service_time;

  // Initialize ready queue and response ratio
  for (int i = 0; i < n; i++) {
    ready_queue.push_back(i);
    response_ratio.push_back(0);
    service_time.push_back(processes[i].remaining);
  }

  // Sort processes by arrival time
  sort(processes, processes + n, arrivalTimeComparator);

  // Process scheduling loop
  while (remaining_processes > 0) {
    int next_process_index = -1;
    float highest_response_ratio = -1;

    // Calculate response ratio for each process in the ready queue
    for (int i = 0; i < ready_queue.size(); i++) {
      int index = ready_queue[i];
      response_ratio[index] =
          (float)(current_time - processes[index].arrival_time +
                  processes[index].service_time) /
          processes[index].service_time;
      if (response_ratio[index] > highest_response_ratio) {
        highest_response_ratio = response_ratio[index];
        next_process_index = index;
      }
    }

    // Check if there are no ready processes
    if (next_process_index == -1) {
      current_time++;
      continue;
    }

    // Run the selected process for 1 time quantum (10ms)
    int remaining_time = service_time[next_process_index];
    for (int i = 0; i < quantum && remaining_time > 0; i++) {
      output[current_time++] = processes[next_process_index].id;
      remaining_time--;
    }

    // Update remaining time and check if process is finished
    service_time[next_process_index] = remaining_time;
    if (remaining_time == 0) {
      remaining_processes--;
      ready_queue.erase(
          find(ready_queue.begin(), ready_queue.end(), next_process_index));
    }
  }
}*/

void feedback_scheduling(Process *processes, int num_processes, int quantum) {
  vector<int> remaining_time(num_processes, 0);
  vector<int> priority(num_processes, 0);
  vector<int> next_time(num_processes, 0);
  vector<int> active_queue;
  vector<int> service_time(num_processes);
  float avg_wait_time = 0.0;       // average waiting time
  float avg_turnaround_time = 0.0; // average turnaround time


  vector<vector<int>> feedback_queues(5, vector<int>());
  int current_time = 0;

  // Initialize remaining time and priority
  for (int i = 0; i < num_processes; i++) {
    service_time[i] = processes[i].service_time;
    remaining_time[i] = processes[i].service_time;
    priority[i] = num_processes - i;
  }

  // Open output file
  ofstream output_file("feedback.out");

  while (true) {
    // Check if all processes have completed
    bool all_done = true;
    for (int i = 0; i < num_processes; i++) {
      if (remaining_time[i] > 0) {
        all_done = false;
        break;
      }
    }
    if (all_done) {
      break;
    }

    // Move processes to the appropriate feedback queue
    for (int i = 0; i < num_processes; i++) {
      if (remaining_time[i] > 0 && next_time[i] <= current_time) {
        feedback_queues[priority[i] - 1].push_back(i);
        next_time[i] = current_time + 10;
      }
    }

    // Find the next process to execute
    int next_process = -1;
    for (int i = 0; i < 5; i++) {
      if (!feedback_queues[i].empty()) {
        next_process = feedback_queues[i][0];
        feedback_queues[i].erase(feedback_queues[i].begin());
        break;
      }
    }

    if (next_process == -1) {
      // No process to execute, so just advance time
      current_time += 10;
      continue;
    }

    // Execute the process for the quantum
    int time_slice = min(10, remaining_time[next_process]);
    for (int i = 0; i < time_slice; i++) {
      active_queue.push_back(next_process);
    }
    remaining_time[next_process] -= time_slice;

    // Update priorities of other processes
    for (int i = 0; i < num_processes; i++) {
      if (i != next_process && remaining_time[i] > 0 &&
          next_time[i] <= current_time) {
        priority[i]++;
      }
    }

    // Advance time
    current_time += time_slice;

    // Move processes from active queue back to feedback queue
    while (!active_queue.empty()) {
      int process = active_queue[0];
      active_queue.erase(active_queue.begin());
      if (remaining_time[process] > 0 && next_time[process] <= current_time) {
        feedback_queues[priority[process] - 1].push_back(process);
        next_time[process] = current_time + 10;
      }
    }
  }

  // Write output to file
  output_file << "Average Waiting Time: " << avg_wait_time << endl;
  output_file << "Average Turnaround Time: " << avg_turnaround_time << endl;

  // Close output file
  output_file.close();
}


int main() {
  int n, quantum;

  // read process data from input file
  ifstream infile("process_data.txt");
  infile >> n >> quantum;

  // create array to hold processes
  Process processes[n];

  // read process data from input file
  for (int i = 0; i < n; i++) {
    infile >> processes[i].id >> processes[i].arrival >> processes[i].burst;
  }

  // sort processes in order of arrival time
  sort(processes, processes + n, compare_arrival);

  // run FCFS scheduling and output results
  ofstream fcfs_outfile("fcfs.out");
  fcfs_scheduling(processes, n);
  fcfs_outfile.close();

  // run Round Robin scheduling with quantum = 10 and output results
  ofstream rr10_outfile("rr_10.out");
  round_robin_scheduling(processes, n, 10, rr10_outfile);
  rr10_outfile.close();

  // run Round Robin scheduling with quantum = 40 and output results
  ofstream rr40_outfile("rr_40.out");
  round_robin_scheduling(processes, n, 40, rr40_outfile);
  rr40_outfile.close();

   // run SPN scheduling algorithm
  ofstream spn_outfile("spn.out");
  shortest_process_next(processes, n);
  spn_outfile.close();

     // run feedback scheduling algorithm with quantum = 10
  ofstream feedback_outfile("feedback.out");
  feedback_scheduling(processes, n, 10);
  feedback_outfile.close();

  return 0;
}
/*int main(int argc, char *argv[]) {
  if (argc < 2) {
    cout << "Usage: " << argv[0] << " <process_data_file>" << endl;
    return 1;
  }

  ifstream input_file(argv[1]);

  if (!input_file) {
    cout << "Error: could not open file " << argv[1] << endl;
    return 1;
  }

  vector<Process> processes;
  string line;
  while (getline(input_file, line)) {
    stringstream ss(line);
    int id, arrival, burst;
    ss >> id >> arrival >> burst;
    processes.push_back(Process{id, arrival, burst, burst});
  }

  input_file.close();

  int n = processes.size();

  cout << "First Come First Serve (FCFS) Scheduling:" << endl;

  fcfs_scheduling(processes.data(), n);

  cout << endl;

  cout << "Round Robin Scheduling with Quantum = 10:" << endl;

  round_robin_scheduling(processes.data(), n, 10);

  cout << endl;

  cout << "Round Robin Scheduling with Quantum = 40:" << endl;

  round_robin_scheduling(processes.data(), n, 40);

  cout << endl;

  cout << "Shortest Process Next (SPN) Scheduling:" << endl;

  shortest_process_next(processes.data(), n);

  cout << endl;

  cout << "Highest Response Ratio Next (HRRN) Scheduling:" << endl;

  //HRRN(processes.data(), n, 10);

  cout << endl;

  cout << "Feedback Scheduling with Quantum = 10:" << endl;

  feedback_scheduling(processes.data(), n, 10);

  return 0;
}
*/