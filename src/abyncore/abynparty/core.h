#pragma once

#include <atomic>
#include <memory>
#include <queue>

#include "communication/communication_handler.h"

#include "utility/configuration.h"
#include "utility/logger.h"

namespace ABYN {

namespace Gates::Interfaces {
class Gate;

using GatePtr = std::shared_ptr<Gate>;
}  // namespace Gates::Interfaces

namespace Wires {
class Wire;

using WirePtr = std::shared_ptr<Wire>;
}  // namespace Wires

class Core {
 public:
  Core(ConfigurationPtr &config) : config_(config) {
    logger_ =
        std::make_shared<ABYN::Logger>(config_->GetMyId(), config_->GetLoggingSeverityLevel());
  }

  std::size_t NextGateId() { return global_gate_id_++; }

  std::size_t NextWireId() { return global_wire_id_++; }

  std::size_t NextArithmeticSharingId(std::size_t num_of_parallel_values) {
    assert(num_of_parallel_values != 0);
    auto old_id = global_arithmetic_sharing_id_;
    global_arithmetic_sharing_id_ += num_of_parallel_values;
    return old_id;
  }

  std::size_t NextBooleanGMWSharingId(std::size_t num_of_parallel_values) {
    assert(num_of_parallel_values != 0);
    auto old_id = global_gmw_sharing_id_;
    global_gmw_sharing_id_ += num_of_parallel_values;
    return old_id;
  }

  const LoggerPtr &GetLogger() { return logger_; }

  const ConfigurationPtr &GetConfig() { return config_; }

  void RegisterCommunicationHandlers(
      std::vector<ABYN::Communication::CommunicationHandlerPtr> &communication_handlers) {
    communication_handlers_ = communication_handlers;
  }

  void Send(std::size_t party_id, flatbuffers::FlatBufferBuilder &message) {
    if (party_id == config_->GetMyId()) {
      throw(std::runtime_error("Want to send message to myself"));
    }
    communication_handlers_.at(party_id)->SendMessage(message);
  }

  void RegisterNextGate(ABYN::Gates::Interfaces::GatePtr gate) {
    assert(gate != nullptr);
    gates_.push_back(gate);
  }

  void RegisterNextInputGate(ABYN::Gates::Interfaces::GatePtr gate) {
    RegisterNextGate(gate);
    assert(gate != nullptr);
    input_gates_.push_back(gate);
  }

  const ABYN::Gates::Interfaces::GatePtr &GetGate(std::size_t gate_id) const {
    return gates_.at(gate_id);
  }
  const auto &GetInputGates() const { return input_gates_; }

  void UnregisterGate(std::size_t gate_id) { gates_.at(gate_id) = nullptr; }

  void RegisterNextWire(ABYN::Wires::WirePtr wire) { wires_.push_back(wire); }

  ABYN::Wires::WirePtr GetWire(std::size_t wire_id) const { return wires_.at(wire_id); }

  void UnregisterWire(std::size_t wire_id) { wires_.at(wire_id) = nullptr; }

  void AddToActiveQueue(std::size_t gate_id) {
    std::scoped_lock lock(active_queue_mutex_);
    active_gates_.push(gate_id);
    logger_->LogTrace(fmt::format("Added gate #{} to the active queue", gate_id));
  }

  std::int64_t GetNextGateFromOnlineQueue() {
    if (active_gates_.size() == 0) {
      return -1;
    } else {
      auto gate_id = active_gates_.front();
      assert(gate_id < std::numeric_limits<std::size_t>::max());
      std::scoped_lock lock(active_queue_mutex_);
      active_gates_.pop();
      return static_cast<std::int64_t>(gate_id);
    }
  }

  void IncrementEvaluatedGatesCounter() { evaluated_gates++; }

  std::size_t GetNumOfEvaluatedGates() { return evaluated_gates; }

  std::size_t GetTotalNumOfGates() { return global_gate_id_; }

 private:
  std::size_t global_gate_id_ = 0, global_wire_id_ = 0, global_arithmetic_sharing_id_ = 0,
              global_gmw_sharing_id_ =
                  0;  // don't need atomic, since only one thread has access to these

  std::atomic<std::size_t> evaluated_gates = 0;

  ABYN::ConfigurationPtr config_;
  ABYN::LoggerPtr logger_ = nullptr;

  std::queue<std::size_t> active_gates_;
  std::mutex active_queue_mutex_;

  std::vector<ABYN::Gates::Interfaces::GatePtr> input_gates_;
  std::vector<ABYN::Gates::Interfaces::GatePtr> gates_;

  std::vector<ABYN::Wires::WirePtr> wires_;

  std::vector<ABYN::Communication::CommunicationHandlerPtr> communication_handlers_;

  Core() = delete;

  Core(Core &) = delete;

  Core(const Core &) = delete;
};

using CorePtr = std::shared_ptr<Core>;
}  // namespace ABYN
