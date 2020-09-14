#pragma once

#include <optional>
#include <string>
#include <unordered_map>
#include <unordered_set>

#include "taichi/ir/ir.h"
#include "taichi/ir/statements.h"
#include "taichi/lang_util.h"
#include "taichi/program/async_utils.h"
#include "taichi/program/program.h"

TLANG_NAMESPACE_BEGIN

class IRBank;
class StateFlowGraph {
 public:
  struct Node;
  using StateToNodeMapping =
      std::unordered_map<AsyncState, Node *, AsyncStateHash>;

  // Each node is a task
  // Note: after SFG is done, each node here should hold a TaskLaunchRecord.
  // Optimization should happen fully on the SFG, instead of the queue in
  // AsyncEngine.
  // Before we migrate, the SFG is only used for visualization. Therefore we
  // only store a kernel_name, which is used as the label of the GraphViz node.
  struct Node {
    TaskLaunchRecord rec;
    std::string task_name;
    OffloadedStmt::TaskType task_type;
    // Incremental ID to identify the i-th launch of the task.
    int launch_id;
    bool is_initial_node{false};

    // TODO: use a reference to the corresponding TaskMeta
    std::unordered_set<AsyncState, AsyncStateHash> input_states, output_states;

    // Returns the position in nodes_. Invoke StateFlowGraph::reid_nodes() to keep it up-to-date.
    int node_id;

    // Profiling showed horrible performance using std::unordered_multimap (at
    // least on Mac with clang-1103.0.32.62)...
    std::unordered_map<AsyncState, std::unordered_set<Node *>, AsyncStateHash>
        output_edges, input_edges;

    std::string string() const;

    // Note: there are two types of edges A->B:
    //   ------
    //   Dependency edge: A must execute before B
    //
    //   Flow edge: A generates some data that is consumed by B. This also
    //   implies that A must execute before B
    //
    //   Therefore Flow edge = Dependency edge + possible state flow

    bool has_state_flow(AsyncState state, const Node *destination) const {
      // True: (Flow edge) the state generated by this node is used by the
      // destination node.

      // False: (Dependency edge) the destination node does not
      // use the generated state, but its execution must happen after this case.
      // This usually means a write-after-read (WAR) dependency on state.

      // Note:
      // Read-after-write leads to flow edges
      // Write-after-write leads to flow edges
      // Write-after-read leads to dependency edges
      //
      // So an edge is a data flow edge iff the starting node writes to the
      // state.
      //

      if (is_initial_node) {
        // The initial node is special.
        return destination->input_states.find(state) !=
               destination->input_states.end();
      } else {
        return output_states.find(state) != output_states.end();
      }
    }
  };

  StateFlowGraph(IRBank *ir_bank);

  void clear();

  void print();

  // Returns a string representing a DOT graph
  // TODO: In case we add more and more DOT configs, create a struct?
  std::string dump_dot(const std::optional<std::string> &rankdir);

  void insert_task(const TaskLaunchRecord &rec, const TaskMeta &task_meta);

  void insert_state_flow(Node *from, Node *to, AsyncState state);

  bool fuse();

  bool optimize_listgen();

  void reid_nodes();

  void topo_sort_nodes();

  // Extract all tasks to execute.
  std::vector<TaskLaunchRecord> extract();

 private:
  std::vector<std::unique_ptr<Node>> nodes_;
  Node *initial_node_;  // The initial node holds all the initial states.
  StateToNodeMapping latest_state_owner_;
  std::unordered_map<AsyncState, std::unordered_set<Node *>, AsyncStateHash>
      latest_state_readers_;
  std::unordered_map<std::string, int> task_name_to_launch_ids_;
  IRBank *ir_bank_;
};

TLANG_NAMESPACE_END
