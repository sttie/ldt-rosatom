#include <memory>
#include <vector>
#include "algos.h"

#include "ortools/base/logging.h"
#include "absl/strings/str_format.h"
#include "absl/types/span.h"
#include "ortools/sat/cp_model.h"
#include "ortools/sat/cp_model.pb.h"
#include "ortools/sat/cp_model_solver.h"

#include <cstdlib>

using namespace operations_research;
using namespace sat;

std::vector<std::pair<size_t, size_t>> AssignmentCP(const std::vector<std::vector<float>> &costs,
                                            const std::vector<std::pair<size_t, size_t>> &banned_pairs) {
    const size_t num_workers = static_cast<size_t>(costs.size());
    std::vector<size_t> all_workers(num_workers);
    std::iota(all_workers.begin(), all_workers.end(), 0);

    const size_t num_tasks = static_cast<size_t>(costs[0].size());
    std::vector<size_t> all_tasks(num_tasks);
    std::iota(all_tasks.begin(), all_tasks.end(), 0);

    // Model
    CpModelBuilder cp_model;

    // Variables
    // x[i][j] is an array of Boolean variables. x[i][j] is true
    // if worker i is assigned to task j.
    std::vector<std::vector<BoolVar>> x(num_workers,
                                      std::vector<BoolVar>(num_tasks));
    for (size_t worker : all_workers) {
        for (size_t task : all_tasks) {
            x[worker][task] = cp_model.NewBoolVar().WithName(
                absl::StrFormat("x[%d,%d]", worker, task));
        }
    }

    // Constraints
    // Each worker is assigned to at most one task.
    for (size_t worker : all_workers) {
        cp_model.AddExactlyOne(x[worker]);
    }
    // Each task is assigned to exactly one worker.
    for (size_t task : all_tasks) {
        std::vector<BoolVar> tasks;
        for (size_t worker : all_workers) {
            tasks.push_back(x[worker][task]);
        }
        cp_model.AddAtMostOne(tasks);
    }

    for (const auto &task_pair: banned_pairs) {
        for (size_t i = 0; i < num_workers; i++) {
            for (size_t j = i + 1; j < num_workers; j++) {
                cp_model.AddLessThan(LinearExpr(x[i][task_pair.first] + x[j][task_pair.second]), 2);
                cp_model.AddLessThan(LinearExpr(x[i][task_pair.second] + x[j][task_pair.first]), 2);
            }
        }
    }

    // Objective
    LinearExpr total_cost;
    for (size_t worker : all_workers) {
        for (size_t task : all_tasks) {
            total_cost += x[worker][task] * costs[worker][task];
        }
    } 
    cp_model.Minimize(total_cost);

    // Solve
    const CpSolverResponse response = Solve(cp_model.Build());

    std::vector<std::pair<size_t, size_t>> result;

    // Print solution.
    if (response.status() == CpSolverStatus::INFEASIBLE) {
        LOG(FATAL) << "No solution found.";
    }
    LOG(INFO) << "Total cost: " << response.objective_value();
    LOG(INFO);
    for (size_t worker : all_workers) {
        for (size_t task : all_tasks) {
            if (SolutionBooleanValue(response, x[worker][task])) {
                result.push_back({worker, task});
                LOG(INFO) << "Worker " << worker << " assigned to task " << task
                  << ".  Cost: " << costs[worker][task];
            }
        }
    }

    return result;
}