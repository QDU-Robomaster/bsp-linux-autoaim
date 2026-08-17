#pragma once

#include <algorithm>
#include <array>
#include <atomic>
#include <chrono>
#include <cmath>
#include <condition_variable>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <limits>
#include <map>
#include <mutex>
#include <string>
#include <thread>
#include <utility>
#include <vector>

namespace AutoAimReplayBenchmark
{
using Clock = std::chrono::steady_clock;

inline constexpr double kNaN = std::numeric_limits<double>::quiet_NaN();

struct DetectionRecord
{
  int color{-1};
  int type{-1};
  int number{-1};
  double confidence{kNaN};
  bool pnp_valid{false};
  double pnp_error_px{kNaN};
  std::array<double, 8> corners{};
  std::array<double, 3> translation{};
};

struct FrameRecord
{
  uint64_t timestamp_us{0};
  int64_t capture_start_ns{0};
  int64_t capture_commit_ns{0};
  int64_t sync_ns{0};
  int64_t detector_start_ns{0};
  int64_t detector_end_ns{0};
  int64_t tracker_enqueue_ns{0};
  int64_t tracker_queued_ns{0};
  int64_t tracker_start_ns{0};
  int64_t tracker_end_ns{0};
  int64_t aimer_start_ns{0};
  int64_t aimer_end_ns{0};

  double capture_read_ms{kNaN};
  double capture_decode_ms{kNaN};
  double capture_commit_ms{kNaN};
  double detector_preprocess_ms{kNaN};
  double detector_infer_call_ms{kNaN};
  double detector_postprocess_ms{kNaN};
  double hailo_infer_ms{kNaN};
  double hailo_tail_ms{kNaN};
  double detector_compute_ms{kNaN};
  double detector_result_ms{kNaN};
  double tracker_copy_ms{kNaN};
  double tracker_compute_ms{kNaN};
  double aimer_callback_ms{kNaN};
  double mpc_reference_ms{kNaN};
  double mpc_yaw_ms{kNaN};
  double mpc_roll_ms{kNaN};
  double mpc_total_ms{kNaN};

  uint32_t armor_count{0};
  uint32_t pnp_count{0};
  bool tracker_output{false};
  bool tracking{false};
  int target_id{-1};
  std::array<double, 3> target_position{};
  std::array<double, 3> target_velocity{};
  double target_yaw{kNaN};
  double target_yaw_velocity{kNaN};
  double target_radius_1{kNaN};
  double target_radius_2{kNaN};
  double target_dz{kNaN};

  bool mpc_attempted{false};
  bool mpc_solver_ran{false};
  bool mpc_output_finite{false};
  bool mpc_plan_accepted{false};
  int yaw_rc{-999};
  int yaw_solved{-1};
  int yaw_status{-1};
  int yaw_iterations{-1};
  int roll_rc{-999};
  int roll_solved{-1};
  int roll_status{-1};
  int roll_iterations{-1};

  bool aimer_output{false};
  bool aimer_control{false};
  bool aimer_fire{false};
  bool aimer_used_mpc{false};
  bool aimer_output_finite{false};
  std::array<double, 8> gimbal_plan{};
  std::vector<DetectionRecord> detections{};
};

struct PipelineTimingRecord
{
  uint64_t timestamp_us{0};
  int64_t slot_acquire_ns{0};
  int64_t infer_enqueue_ns{0};
  int64_t infer_start_ns{0};
  int64_t infer_end_ns{0};
  int64_t infer_worker_period_ns{0};
  int64_t infer_worker_intercall_gap_ns{0};
  int64_t infer_worker_dispatch_gap_ns{0};
  int64_t output_enqueue_ns{0};
  int64_t output_start_ns{0};
  int64_t output_worker_period_ns{0};
  int64_t output_end_ns{0};
  int64_t post_enqueue_ns{0};
  int64_t post_start_ns{0};
  int64_t post_worker_period_ns{0};
  int64_t post_end_ns{0};
  int64_t slot_release_ns{0};
  uint32_t slots_busy_before_admission{0};
  uint32_t slot_id{0};
  uint64_t slot_generation{0};
  bool infer_backlog_after_call{false};
  uint64_t no_free_count_at_release{0};
};

struct AsyncPipelineTimingRecord
{
  uint64_t timestamp_us{0};
  uint64_t admission_seq{0};
  uint64_t request_id{0};
  int64_t infer_submit_ns{0};
  int64_t infer_complete_ns{0};
  uint64_t completion_seq{0};
  uint32_t inflight_before_submit{0};
  uint32_t inflight_after_submit{0};
  uint32_t inflight_at_complete{0};
  uint32_t inflight_high_water_after_submit{0};
  bool completion_reordered{false};
  int64_t post_enqueue_ns{0};
  int64_t post_start_ns{0};
  int64_t output_publish_ns{0};
  int64_t post_end_ns{0};
  uint64_t output_seq{0};
  int64_t slot_release_ns{0};
};

struct TrackerPipelineTimingRecord
{
  uint64_t timestamp_us{0};
  uint64_t admission_sequence{0};
  uint64_t worker_sequence{0};
  double producer_wait_ms{kNaN};
  bool waited_for_full{false};
  uint32_t ready_after_commit{0};
  uint32_t occupied_after_commit{0};
  uint32_t high_water_after_commit{0};
  double worker_service_ms{kNaN};
};

struct Config
{
  bool enabled{false};
  bool require_complete_pipeline{false};
  bool wait_for_aimer{false};
  std::filesystem::path output_dir{};
  uint64_t expected_frames{0};
  uint64_t quiet_ms{1500};
  uint64_t timeout_ms{120000};
  uint64_t frame_timeout_ms{5000};
  std::string variant{"unknown"};
  std::string run_id{"unknown"};
};

struct RunState
{
  std::mutex mutex{};
  std::mutex pipeline_timing_mutex{};
  std::condition_variable condition{};
  std::map<uint64_t, FrameRecord> frames{};
  std::map<uint64_t, PipelineTimingRecord> pipeline_timings{};
  std::map<uint64_t, AsyncPipelineTimingRecord> async_pipeline_timings{};
  std::map<uint64_t, TrackerPipelineTimingRecord> tracker_pipeline_timings{};
  bool pipeline_ready{false};
  bool source_complete{false};
  bool source_ok{false};
  uint64_t source_frames{0};
  uint64_t sync_drops{0};
  uint64_t tracker_overwrites{0};
  bool detector_pipeline_counters_recorded{false};
  std::array<uint64_t, 10> detector_pipeline_counters{};
  int64_t started_ns{0};
  int64_t source_complete_ns{0};
  int64_t last_event_ns{0};
  bool artifacts_written{false};
  int exit_code{2};
};

inline uint64_t ParseUnsignedEnv(const char* name, uint64_t fallback)
{
  const char* value = std::getenv(name);
  if (value == nullptr || value[0] == '\0')
  {
    return fallback;
  }
  char* end = nullptr;
  const unsigned long long parsed = std::strtoull(value, &end, 10);
  return end != value && end != nullptr && end[0] == '\0' ? static_cast<uint64_t>(parsed)
                                                          : fallback;
}

inline const Config& GetConfig()
{
  static const Config config = []
  {
    Config value{};
    const char* output_dir = std::getenv("AUTOAIM_REPLAY_BENCHMARK_DIR");
    value.enabled = output_dir != nullptr && output_dir[0] != '\0';
    if (value.enabled)
    {
      value.output_dir = output_dir;
    }
    value.expected_frames = ParseUnsignedEnv("AUTOAIM_REPLAY_EXPECTED_FRAMES", 0);
    value.require_complete_pipeline =
        ParseUnsignedEnv("AUTOAIM_REPLAY_REQUIRE_COMPLETE_PIPELINE", 0) != 0;
    value.wait_for_aimer = ParseUnsignedEnv("AUTOAIM_REPLAY_WAIT_FOR_AIMER", 0) != 0;
    value.quiet_ms = ParseUnsignedEnv("AUTOAIM_REPLAY_QUIET_MS", 1500);
    value.timeout_ms = ParseUnsignedEnv("AUTOAIM_REPLAY_TIMEOUT_MS", 120000);
    value.frame_timeout_ms = ParseUnsignedEnv("AUTOAIM_REPLAY_FRAME_TIMEOUT_MS", 5000);
    if (const char* variant = std::getenv("AUTOAIM_REPLAY_VARIANT"))
    {
      value.variant = variant;
    }
    if (const char* run_id = std::getenv("AUTOAIM_REPLAY_RUN_ID"))
    {
      value.run_id = run_id;
    }
    return value;
  }();
  return config;
}

inline bool Enabled() { return GetConfig().enabled; }

inline int64_t NowNs()
{
  return std::chrono::duration_cast<std::chrono::nanoseconds>(
             Clock::now().time_since_epoch())
      .count();
}

inline double DurationMs(int64_t begin_ns, int64_t end_ns)
{
  if (begin_ns == 0 || end_ns < begin_ns)
  {
    return kNaN;
  }
  return static_cast<double>(end_ns - begin_ns) / 1000000.0;
}

inline RunState& State()
{
  static RunState state{};
  return state;
}

inline void MarkPipelineReady()
{
  if (!Enabled())
  {
    return;
  }
  RunState& state = State();
  {
    std::lock_guard<std::mutex> lock(state.mutex);
    state.pipeline_ready = true;
  }
  state.condition.notify_all();
}

inline bool WaitForPipelineReady()
{
  if (!Enabled())
  {
    return true;
  }
  RunState& state = State();
  std::unique_lock<std::mutex> lock(state.mutex);
  return state.condition.wait_for(lock, std::chrono::milliseconds(GetConfig().timeout_ms),
                                  [&state] { return state.pipeline_ready; });
}

inline bool WaitForAimer(uint64_t timestamp_us)
{
  const Config& config = GetConfig();
  if (!config.wait_for_aimer)
  {
    return true;
  }
  RunState& state = State();
  std::unique_lock<std::mutex> lock(state.mutex);
  return state.condition.wait_for(
      lock, std::chrono::milliseconds(config.frame_timeout_ms),
      [&state, timestamp_us]
      {
        const auto frame = state.frames.find(timestamp_us);
        return frame != state.frames.end() && frame->second.aimer_output;
      });
}

template <typename Update>
inline void UpdateFrame(uint64_t timestamp_us, Update&& update)
{
  if (!Enabled())
  {
    return;
  }
  const int64_t now_ns = NowNs();
  RunState& state = State();
  std::lock_guard<std::mutex> lock(state.mutex);
  if (state.started_ns == 0)
  {
    state.started_ns = now_ns;
  }
  FrameRecord& frame = state.frames[timestamp_us];
  frame.timestamp_us = timestamp_us;
  update(frame, now_ns);
  state.last_event_ns = now_ns;
}

inline void RecordCaptureStart(uint64_t timestamp_us)
{
  UpdateFrame(timestamp_us, [](FrameRecord& frame, int64_t now_ns)
              { frame.capture_start_ns = now_ns; });
}

inline void RecordCaptureDecode(uint64_t timestamp_us, double read_ms, double decode_ms)
{
  UpdateFrame(timestamp_us,
              [=](FrameRecord& frame, int64_t)
              {
                frame.capture_read_ms = read_ms;
                frame.capture_decode_ms = decode_ms;
              });
}

inline void RecordCaptureCommit(uint64_t timestamp_us, double commit_ms)
{
  UpdateFrame(timestamp_us,
              [=](FrameRecord& frame, int64_t now_ns)
              {
                frame.capture_commit_ns = now_ns;
                frame.capture_commit_ms = commit_ms;
              });
}

inline void RecordSync(uint64_t timestamp_us)
{
  UpdateFrame(timestamp_us,
              [](FrameRecord& frame, int64_t now_ns) { frame.sync_ns = now_ns; });
}

inline void RecordSyncDrop()
{
  if (!Enabled())
  {
    return;
  }
  RunState& state = State();
  std::lock_guard<std::mutex> lock(state.mutex);
  ++state.sync_drops;
  state.last_event_ns = NowNs();
}

inline void RecordDetectorStart(uint64_t timestamp_us)
{
  UpdateFrame(timestamp_us, [](FrameRecord& frame, int64_t now_ns)
              { frame.detector_start_ns = now_ns; });
}

inline void RecordDetector(uint64_t timestamp_us, double preprocess_ms,
                           double infer_call_ms, double postprocess_ms,
                           double hailo_infer_ms, double hailo_tail_ms,
                           double detector_compute_ms, double result_ms,
                           uint32_t armor_count, uint32_t pnp_count)
{
  UpdateFrame(timestamp_us,
              [=](FrameRecord& frame, int64_t now_ns)
              {
                frame.detector_end_ns = now_ns;
                frame.detector_preprocess_ms = preprocess_ms;
                frame.detector_infer_call_ms = infer_call_ms;
                frame.detector_postprocess_ms = postprocess_ms;
                frame.hailo_infer_ms = hailo_infer_ms;
                frame.hailo_tail_ms = hailo_tail_ms;
                frame.detector_compute_ms = detector_compute_ms;
                frame.detector_result_ms = result_ms;
                frame.armor_count = armor_count;
                frame.pnp_count = pnp_count;
              });
}

inline void RecordPipelineTiming(
    uint64_t timestamp_us, int64_t slot_acquire_ns, int64_t infer_enqueue_ns,
    int64_t infer_start_ns, int64_t infer_end_ns, int64_t infer_worker_period_ns,
    int64_t infer_worker_intercall_gap_ns, int64_t infer_worker_dispatch_gap_ns,
    int64_t output_enqueue_ns, int64_t output_start_ns, int64_t output_worker_period_ns,
    int64_t output_end_ns, int64_t post_enqueue_ns, int64_t post_start_ns,
    int64_t post_worker_period_ns, int64_t post_end_ns, int64_t slot_release_ns,
    uint32_t slots_busy_before_admission, uint32_t slot_id, uint64_t slot_generation,
    bool infer_backlog_after_call, uint64_t no_free_count_at_release)
{
  if (!Enabled())
  {
    return;
  }
  const PipelineTimingRecord record{
      .timestamp_us = timestamp_us,
      .slot_acquire_ns = slot_acquire_ns,
      .infer_enqueue_ns = infer_enqueue_ns,
      .infer_start_ns = infer_start_ns,
      .infer_end_ns = infer_end_ns,
      .infer_worker_period_ns = infer_worker_period_ns,
      .infer_worker_intercall_gap_ns = infer_worker_intercall_gap_ns,
      .infer_worker_dispatch_gap_ns = infer_worker_dispatch_gap_ns,
      .output_enqueue_ns = output_enqueue_ns,
      .output_start_ns = output_start_ns,
      .output_worker_period_ns = output_worker_period_ns,
      .output_end_ns = output_end_ns,
      .post_enqueue_ns = post_enqueue_ns,
      .post_start_ns = post_start_ns,
      .post_worker_period_ns = post_worker_period_ns,
      .post_end_ns = post_end_ns,
      .slot_release_ns = slot_release_ns,
      .slots_busy_before_admission = slots_busy_before_admission,
      .slot_id = slot_id,
      .slot_generation = slot_generation,
      .infer_backlog_after_call = infer_backlog_after_call,
      .no_free_count_at_release = no_free_count_at_release,
  };
  RunState& state = State();
  std::lock_guard<std::mutex> lock(state.pipeline_timing_mutex);
  state.pipeline_timings[timestamp_us] = record;
}

inline void RecordAsyncPipelineTiming(
    uint64_t timestamp_us, uint64_t admission_seq, uint64_t request_id,
    int64_t infer_submit_ns, int64_t infer_complete_ns, uint64_t completion_seq,
    uint32_t inflight_before_submit, uint32_t inflight_after_submit,
    uint32_t inflight_at_complete, uint32_t inflight_high_water_after_submit,
    bool completion_reordered, int64_t post_enqueue_ns, int64_t post_start_ns,
    int64_t output_publish_ns, int64_t post_end_ns, uint64_t output_seq,
    int64_t slot_release_ns)
{
  if (!Enabled())
  {
    return;
  }
  const AsyncPipelineTimingRecord record{
      .timestamp_us = timestamp_us,
      .admission_seq = admission_seq,
      .request_id = request_id,
      .infer_submit_ns = infer_submit_ns,
      .infer_complete_ns = infer_complete_ns,
      .completion_seq = completion_seq,
      .inflight_before_submit = inflight_before_submit,
      .inflight_after_submit = inflight_after_submit,
      .inflight_at_complete = inflight_at_complete,
      .inflight_high_water_after_submit = inflight_high_water_after_submit,
      .completion_reordered = completion_reordered,
      .post_enqueue_ns = post_enqueue_ns,
      .post_start_ns = post_start_ns,
      .output_publish_ns = output_publish_ns,
      .post_end_ns = post_end_ns,
      .output_seq = output_seq,
      .slot_release_ns = slot_release_ns,
  };
  RunState& state = State();
  std::lock_guard<std::mutex> lock(state.pipeline_timing_mutex);
  state.async_pipeline_timings[timestamp_us] = record;
}

inline std::atomic<uint64_t>& PipelineNoFreeCounter()
{
  static std::atomic<uint64_t> count{0U};
  return count;
}

inline void RecordPipelineNoFree()
{
  if (Enabled())
  {
    PipelineNoFreeCounter().fetch_add(1U, std::memory_order_relaxed);
  }
}

inline uint64_t PipelineNoFreeCount()
{
  if (!Enabled())
  {
    return 0U;
  }
  return PipelineNoFreeCounter().load(std::memory_order_relaxed);
}

inline void RecordDetectorPipelineCounters(uint64_t admitted, uint64_t completed,
                                           uint64_t prepare_drop, uint64_t no_free,
                                           uint64_t publish_fail, uint64_t unsynced,
                                           uint64_t claim_miss, uint64_t subscriber_drop,
                                           uint64_t infer_fail, uint64_t post_fail)
{
  if (!Enabled())
  {
    return;
  }
  RunState& state = State();
  std::lock_guard<std::mutex> lock(state.mutex);
  state.detector_pipeline_counters_recorded = true;
  state.detector_pipeline_counters = {
      admitted, completed,  prepare_drop,    no_free,    publish_fail,
      unsynced, claim_miss, subscriber_drop, infer_fail, post_fail};
}

inline void RecordDetection(uint64_t timestamp_us, DetectionRecord detection)
{
  UpdateFrame(timestamp_us,
              [detection = std::move(detection)](FrameRecord& frame, int64_t) mutable
              { frame.detections.push_back(std::move(detection)); });
}

inline void RecordTrackerEnqueue(uint64_t timestamp_us)
{
  UpdateFrame(timestamp_us, [](FrameRecord& frame, int64_t now_ns)
              { frame.tracker_enqueue_ns = now_ns; });
}

inline void RecordTrackerQueued(uint64_t timestamp_us, double copy_ms)
{
  UpdateFrame(timestamp_us,
              [=](FrameRecord& frame, int64_t now_ns)
              {
                frame.tracker_queued_ns = now_ns;
                frame.tracker_copy_ms = copy_ms;
              });
}

inline void RecordTrackerQueueAdmission(uint64_t timestamp_us,
                                        uint64_t admission_sequence,
                                        double producer_wait_ms, bool waited_for_full,
                                        uint32_t ready_after_commit,
                                        uint32_t occupied_after_commit,
                                        uint32_t high_water_after_commit)
{
  if (!Enabled())
  {
    return;
  }
  RunState& state = State();
  std::lock_guard<std::mutex> lock(state.pipeline_timing_mutex);
  TrackerPipelineTimingRecord& timing = state.tracker_pipeline_timings[timestamp_us];
  timing.timestamp_us = timestamp_us;
  timing.admission_sequence = admission_sequence;
  timing.producer_wait_ms = producer_wait_ms;
  timing.waited_for_full = waited_for_full;
  timing.ready_after_commit = ready_after_commit;
  timing.occupied_after_commit = occupied_after_commit;
  timing.high_water_after_commit = high_water_after_commit;
}

inline void RecordTrackerWorkerService(uint64_t timestamp_us, uint64_t admission_sequence,
                                       uint64_t worker_sequence, double worker_service_ms)
{
  if (!Enabled())
  {
    return;
  }
  RunState& state = State();
  std::lock_guard<std::mutex> lock(state.pipeline_timing_mutex);
  TrackerPipelineTimingRecord& timing = state.tracker_pipeline_timings[timestamp_us];
  timing.timestamp_us = timestamp_us;
  timing.admission_sequence = admission_sequence;
  timing.worker_sequence = worker_sequence;
  timing.worker_service_ms = worker_service_ms;
}

inline void RecordTrackerOverwrite()
{
  if (!Enabled())
  {
    return;
  }
  RunState& state = State();
  std::lock_guard<std::mutex> lock(state.mutex);
  ++state.tracker_overwrites;
  state.last_event_ns = NowNs();
}

inline void RecordTrackerStart(uint64_t timestamp_us)
{
  UpdateFrame(timestamp_us, [](FrameRecord& frame, int64_t now_ns)
              { frame.tracker_start_ns = now_ns; });
}

inline void RecordTracker(uint64_t timestamp_us, double compute_ms, bool tracking,
                          int target_id, const std::array<double, 3>& position,
                          const std::array<double, 3>& velocity, double yaw,
                          double yaw_velocity, double radius_1, double radius_2,
                          double dz)
{
  UpdateFrame(timestamp_us,
              [&](FrameRecord& frame, int64_t now_ns)
              {
                frame.tracker_end_ns = now_ns;
                frame.tracker_compute_ms = compute_ms;
                frame.tracker_output = true;
                frame.tracking = tracking;
                frame.target_id = target_id;
                frame.target_position = position;
                frame.target_velocity = velocity;
                frame.target_yaw = yaw;
                frame.target_yaw_velocity = yaw_velocity;
                frame.target_radius_1 = radius_1;
                frame.target_radius_2 = radius_2;
                frame.target_dz = dz;
              });
}

inline void RecordAimerStart(uint64_t timestamp_us)
{
  UpdateFrame(timestamp_us,
              [](FrameRecord& frame, int64_t now_ns) { frame.aimer_start_ns = now_ns; });
}

inline void RecordMpc(uint64_t timestamp_us, bool solver_ran, bool output_finite,
                      bool plan_accepted, int yaw_rc, int yaw_solved, int yaw_status,
                      int yaw_iterations, int roll_rc, int roll_solved, int roll_status,
                      int roll_iterations, double reference_ms, double yaw_ms,
                      double roll_ms, double total_ms)
{
  UpdateFrame(timestamp_us,
              [=](FrameRecord& frame, int64_t)
              {
                frame.mpc_attempted = true;
                frame.mpc_solver_ran = solver_ran;
                frame.mpc_output_finite = output_finite;
                frame.mpc_plan_accepted = plan_accepted;
                frame.yaw_rc = yaw_rc;
                frame.yaw_solved = yaw_solved;
                frame.yaw_status = yaw_status;
                frame.yaw_iterations = yaw_iterations;
                frame.roll_rc = roll_rc;
                frame.roll_solved = roll_solved;
                frame.roll_status = roll_status;
                frame.roll_iterations = roll_iterations;
                frame.mpc_reference_ms = reference_ms;
                frame.mpc_yaw_ms = yaw_ms;
                frame.mpc_roll_ms = roll_ms;
                frame.mpc_total_ms = total_ms;
              });
}

inline void RecordAimer(uint64_t timestamp_us, double callback_ms, bool control,
                        bool fire, bool used_mpc, bool output_finite,
                        const std::array<double, 8>& gimbal_plan)
{
  UpdateFrame(timestamp_us,
              [&](FrameRecord& frame, int64_t now_ns)
              {
                frame.aimer_end_ns = now_ns;
                frame.aimer_callback_ms = callback_ms;
                frame.aimer_output = true;
                frame.aimer_control = control;
                frame.aimer_fire = fire;
                frame.aimer_used_mpc = used_mpc;
                frame.aimer_output_finite = output_finite;
                frame.gimbal_plan = gimbal_plan;
              });
  if (Enabled())
  {
    State().condition.notify_all();
  }
}

inline void MarkSourceComplete(uint64_t source_frames, bool source_ok)
{
  if (!Enabled())
  {
    return;
  }
  const int64_t now_ns = NowNs();
  RunState& state = State();
  std::lock_guard<std::mutex> lock(state.mutex);
  state.source_complete = true;
  state.source_ok = source_ok;
  state.source_frames = source_frames;
  state.source_complete_ns = now_ns;
  state.last_event_ns = now_ns;
}

inline bool ShouldStop()
{
  if (!Enabled())
  {
    return false;
  }
  const int64_t now_ns = NowNs();
  const Config& config = GetConfig();
  RunState& state = State();
  std::lock_guard<std::mutex> lock(state.mutex);
  if (state.started_ns != 0 &&
      now_ns - state.started_ns >= static_cast<int64_t>(config.timeout_ms) * 1000000)
  {
    return true;
  }
  return state.source_complete && state.last_event_ns != 0 &&
         now_ns - state.last_event_ns >= static_cast<int64_t>(config.quiet_ms) * 1000000;
}

inline void WriteNumber(std::ostream& output, double value)
{
  if (std::isfinite(value))
  {
    output << value;
  }
  else
  {
    output << "nan";
  }
}

inline int WriteArtifacts()
{
  if (!Enabled())
  {
    return 0;
  }

  std::map<uint64_t, FrameRecord> frames;
  std::map<uint64_t, PipelineTimingRecord> pipeline_timings;
  std::map<uint64_t, AsyncPipelineTimingRecord> async_pipeline_timings;
  std::map<uint64_t, TrackerPipelineTimingRecord> tracker_pipeline_timings;
  bool source_complete = false;
  bool source_ok = false;
  uint64_t source_frames = 0;
  uint64_t sync_drops = 0;
  uint64_t tracker_overwrites = 0;
  bool detector_pipeline_counters_recorded = false;
  std::array<uint64_t, 10> detector_pipeline_counters{};
  int64_t started_ns = 0;
  int64_t source_complete_ns = 0;
  int64_t last_event_ns = 0;
  {
    RunState& state = State();
    std::lock_guard<std::mutex> lock(state.mutex);
    frames = state.frames;
    source_complete = state.source_complete;
    source_ok = state.source_ok;
    source_frames = state.source_frames;
    sync_drops = state.sync_drops;
    tracker_overwrites = state.tracker_overwrites;
    detector_pipeline_counters_recorded = state.detector_pipeline_counters_recorded;
    detector_pipeline_counters = state.detector_pipeline_counters;
    started_ns = state.started_ns;
    source_complete_ns = state.source_complete_ns;
    last_event_ns = state.last_event_ns;
  }
  {
    RunState& state = State();
    std::lock_guard<std::mutex> lock(state.pipeline_timing_mutex);
    pipeline_timings = state.pipeline_timings;
    async_pipeline_timings = state.async_pipeline_timings;
    tracker_pipeline_timings = state.tracker_pipeline_timings;
  }

  const Config& config = GetConfig();
  std::error_code filesystem_error;
  std::filesystem::create_directories(config.output_dir, filesystem_error);
  if (filesystem_error)
  {
    std::fprintf(stderr, "ReplayBenchmark create directory failed: %s\n",
                 filesystem_error.message().c_str());
    return 3;
  }

  std::ofstream frame_output(config.output_dir / "frames.tsv");
  std::ofstream pipeline_timing_output(config.output_dir / "pipeline_timing.tsv");
  std::ofstream async_pipeline_timing_output(config.output_dir /
                                             "async_pipeline_timing.tsv");
  std::ofstream tracker_pipeline_timing_output(config.output_dir /
                                               "tracker_pipeline_timing.tsv");
  std::ofstream detection_output(config.output_dir / "detections.tsv");
  std::ofstream summary_output(config.output_dir / "run_summary.tsv");
  if (!frame_output || !pipeline_timing_output || !async_pipeline_timing_output ||
      !tracker_pipeline_timing_output || !detection_output || !summary_output)
  {
    std::fprintf(stderr, "ReplayBenchmark failed to open output files\n");
    return 3;
  }

  frame_output << std::setprecision(12);
  pipeline_timing_output << std::setprecision(12);
  async_pipeline_timing_output << std::setprecision(12);
  tracker_pipeline_timing_output << std::setprecision(12);
  detection_output << std::setprecision(12);
  summary_output << std::setprecision(12);
  frame_output << "timestamp_us\tcapture_read_ms\tcapture_decode_ms\tcapture_commit_ms"
                  "\tsync_from_capture_ms\tdetector_queue_ms\tdetector_preprocess_ms"
                  "\tdetector_infer_call_ms\tdetector_postprocess_ms\thailo_infer_ms"
                  "\thailo_tail_ms\tdetector_compute_ms\tdetector_result_ms"
                  "\ttracker_copy_ms\ttracker_queue_ms\ttracker_compute_ms"
                  "\taimer_callback_ms\tmpc_reference_ms\tmpc_yaw_ms\tmpc_roll_ms"
                  "\tmpc_total_ms\tend_to_end_ms\tsource_to_end_ms\tarmor_count"
                  "\tpnp_count\ttracking"
                  "\ttarget_id\ttarget_x\ttarget_y\ttarget_z\tvelocity_x\tvelocity_y"
                  "\tvelocity_z\ttarget_yaw\ttarget_yaw_velocity\tradius_1\tradius_2"
                  "\tdz\tmpc_attempted\tmpc_solver_ran\tmpc_output_finite"
                  "\tmpc_plan_accepted\tyaw_rc\tyaw_solved\tyaw_status\tyaw_iterations"
                  "\troll_rc\troll_solved\troll_status\troll_iterations\taimer_control"
                  "\taimer_fire\taimer_used_mpc\taimer_output_finite\tplan_target_yaw"
                  "\tplan_target_roll\tplan_yaw\tplan_yaw_velocity\tplan_yaw_acceleration"
                  "\tplan_roll\tplan_roll_velocity\tplan_roll_acceleration\n";
  detection_output << "timestamp_us\tindex\tcolor\ttype\tnumber\tconfidence\tpnp_valid"
                      "\tpnp_error_px\tx0\ty0\tx1\ty1\tx2\ty2\tx3\ty3\ttx\tty\ttz\n";
  pipeline_timing_output
      << "timestamp_us\tinfer_queue_wait_ms\tinfer_worker_service_ms"
         "\tinfer_worker_period_ms\tinfer_worker_intercall_gap_ms"
         "\tinfer_worker_dispatch_gap_ms\toutput_queue_wait_ms"
         "\toutput_worker_period_ms\toutput_core_service_ms"
         "\toutput_worker_service_ms"
         "\tpost_queue_wait_ms"
         "\tpost_worker_period_ms\tpost_core_service_ms\tpost_worker_service_ms"
         "\tslot_release_wait_ms\tslot_pre_infer_hold_ms\tslot_lifetime_ms"
         "\tslots_busy_before_admission\tinfer_backlog_after_call"
         "\tslot_id\tslot_generation\tno_free_count_at_release\n";
  async_pipeline_timing_output
      << "timestamp_us\tadmission_seq\trequest_id\tinfer_submit_ns"
         "\tinfer_complete_ns\tcompletion_seq\tinflight_before_submit"
         "\tinflight_after_submit\tinflight_at_complete"
         "\tinflight_high_water_after_submit\tcompletion_reordered"
         "\tpost_enqueue_ns\tpost_start_ns\toutput_publish_ns\tpost_end_ns"
         "\toutput_seq\tslot_release_ns\n";
  tracker_pipeline_timing_output
      << "timestamp_us\tadmission_seq\tworker_seq"
         "\tproducer_wait_ms\twaited_for_full"
         "\tready_after_commit\toccupied_after_commit\thigh_water_after_commit"
         "\tworker_service_ms\n";

  for (const auto& [timestamp_us, timing] : pipeline_timings)
  {
    pipeline_timing_output << timestamp_us;
    const std::array<double, 16> values{
        DurationMs(timing.infer_enqueue_ns, timing.infer_start_ns),
        DurationMs(timing.infer_start_ns, timing.infer_end_ns),
        timing.infer_worker_period_ns > 0
            ? static_cast<double>(timing.infer_worker_period_ns) / 1000000.0
            : kNaN,
        timing.infer_worker_intercall_gap_ns > 0
            ? static_cast<double>(timing.infer_worker_intercall_gap_ns) / 1000000.0
            : kNaN,
        timing.infer_worker_dispatch_gap_ns > 0
            ? static_cast<double>(timing.infer_worker_dispatch_gap_ns) / 1000000.0
            : kNaN,
        DurationMs(timing.output_enqueue_ns, timing.output_start_ns),
        timing.output_worker_period_ns > 0
            ? static_cast<double>(timing.output_worker_period_ns) / 1000000.0
            : kNaN,
        DurationMs(timing.output_start_ns, timing.output_end_ns),
        DurationMs(timing.output_start_ns, timing.post_enqueue_ns),
        DurationMs(timing.post_enqueue_ns, timing.post_start_ns),
        timing.post_worker_period_ns > 0
            ? static_cast<double>(timing.post_worker_period_ns) / 1000000.0
            : kNaN,
        DurationMs(timing.post_start_ns, timing.post_end_ns),
        DurationMs(timing.post_start_ns, timing.slot_release_ns),
        DurationMs(timing.post_end_ns, timing.slot_release_ns),
        DurationMs(timing.slot_acquire_ns, timing.infer_start_ns),
        DurationMs(timing.slot_acquire_ns, timing.slot_release_ns),
    };
    for (double value : values)
    {
      pipeline_timing_output << '\t';
      WriteNumber(pipeline_timing_output, value);
    }
    pipeline_timing_output << '\t' << timing.slots_busy_before_admission << '\t'
                           << (timing.infer_backlog_after_call ? 1 : 0) << '\t'
                           << timing.slot_id << '\t' << timing.slot_generation << '\t'
                           << timing.no_free_count_at_release << '\n';
  }

  for (const auto& [timestamp_us, timing] : async_pipeline_timings)
  {
    async_pipeline_timing_output
        << timestamp_us << '\t' << timing.admission_seq << '\t' << timing.request_id
        << '\t' << timing.infer_submit_ns << '\t' << timing.infer_complete_ns << '\t'
        << timing.completion_seq << '\t' << timing.inflight_before_submit << '\t'
        << timing.inflight_after_submit << '\t' << timing.inflight_at_complete << '\t'
        << timing.inflight_high_water_after_submit << '\t'
        << (timing.completion_reordered ? 1 : 0) << '\t' << timing.post_enqueue_ns << '\t'
        << timing.post_start_ns << '\t' << timing.output_publish_ns << '\t'
        << timing.post_end_ns << '\t' << timing.output_seq << '\t'
        << timing.slot_release_ns << '\n';
  }

  for (const auto& [timestamp_us, timing] : tracker_pipeline_timings)
  {
    tracker_pipeline_timing_output << timestamp_us << '\t' << timing.admission_sequence
                                   << '\t' << timing.worker_sequence << '\t';
    WriteNumber(tracker_pipeline_timing_output, timing.producer_wait_ms);
    tracker_pipeline_timing_output
        << '\t' << (timing.waited_for_full ? 1 : 0) << '\t' << timing.ready_after_commit
        << '\t' << timing.occupied_after_commit << '\t' << timing.high_water_after_commit
        << '\t';
    WriteNumber(tracker_pipeline_timing_output, timing.worker_service_ms);
    tracker_pipeline_timing_output << '\n';
  }

  uint64_t capture_count = 0;
  uint64_t sync_count = 0;
  uint64_t detector_count = 0;
  uint64_t armor_total = 0;
  uint64_t pnp_total = 0;
  uint64_t tracker_count = 0;
  uint64_t tracking_count = 0;
  uint64_t aimer_count = 0;
  uint64_t mpc_attempt_count = 0;
  uint64_t mpc_solver_count = 0;
  uint64_t mpc_finite_count = 0;
  uint64_t mpc_accepted_count = 0;
  uint64_t mpc_converged_count = 0;
  uint64_t nonfinite_output_count = 0;
  uint64_t fire_true_count = 0;
  int64_t last_capture_ns = 0;
  int64_t first_detector_ns = 0;
  int64_t last_detector_ns = 0;
  int64_t first_tracker_ns = 0;
  int64_t last_tracker_ns = 0;
  int64_t first_aimer_ns = 0;
  int64_t last_aimer_ns = 0;

  for (const auto& [timestamp_us, frame] : frames)
  {
    capture_count += frame.capture_commit_ns != 0 ? 1U : 0U;
    sync_count += frame.sync_ns != 0 ? 1U : 0U;
    detector_count += frame.detector_end_ns != 0 ? 1U : 0U;
    armor_total += frame.armor_count;
    pnp_total += frame.pnp_count;
    tracker_count += frame.tracker_output ? 1U : 0U;
    tracking_count += frame.tracking ? 1U : 0U;
    aimer_count += frame.aimer_output ? 1U : 0U;
    mpc_attempt_count += frame.mpc_attempted ? 1U : 0U;
    mpc_solver_count += frame.mpc_solver_ran ? 1U : 0U;
    mpc_finite_count += frame.mpc_output_finite ? 1U : 0U;
    mpc_accepted_count += frame.mpc_plan_accepted ? 1U : 0U;
    mpc_converged_count += frame.yaw_rc == 0 && frame.yaw_solved == 1 &&
                                   frame.yaw_status == 1 && frame.roll_rc == 0 &&
                                   frame.roll_solved == 1 && frame.roll_status == 1
                               ? 1U
                               : 0U;
    nonfinite_output_count += frame.aimer_output && !frame.aimer_output_finite ? 1U : 0U;
    fire_true_count += frame.aimer_fire ? 1U : 0U;
    last_capture_ns = std::max(last_capture_ns, frame.capture_commit_ns);
    if (frame.detector_start_ns != 0)
    {
      first_detector_ns = first_detector_ns == 0
                              ? frame.detector_start_ns
                              : std::min(first_detector_ns, frame.detector_start_ns);
    }
    last_detector_ns = std::max(last_detector_ns, frame.detector_end_ns);
    if (frame.tracker_start_ns != 0)
    {
      first_tracker_ns = first_tracker_ns == 0
                             ? frame.tracker_start_ns
                             : std::min(first_tracker_ns, frame.tracker_start_ns);
    }
    last_tracker_ns = std::max(last_tracker_ns, frame.tracker_end_ns);
    if (frame.aimer_start_ns != 0)
    {
      first_aimer_ns = first_aimer_ns == 0
                           ? frame.aimer_start_ns
                           : std::min(first_aimer_ns, frame.aimer_start_ns);
    }
    last_aimer_ns = std::max(last_aimer_ns, frame.aimer_end_ns);

    frame_output << timestamp_us << '\t';
    const std::array<double, 22> timing_values{
        frame.capture_read_ms,
        frame.capture_decode_ms,
        frame.capture_commit_ms,
        DurationMs(frame.capture_start_ns, frame.sync_ns),
        DurationMs(frame.sync_ns, frame.detector_start_ns),
        frame.detector_preprocess_ms,
        frame.detector_infer_call_ms,
        frame.detector_postprocess_ms,
        frame.hailo_infer_ms,
        frame.hailo_tail_ms,
        frame.detector_compute_ms,
        frame.detector_result_ms,
        frame.tracker_copy_ms,
        DurationMs(frame.tracker_queued_ns, frame.tracker_start_ns),
        frame.tracker_compute_ms,
        frame.aimer_callback_ms,
        frame.mpc_reference_ms,
        frame.mpc_yaw_ms,
        frame.mpc_roll_ms,
        frame.mpc_total_ms,
        DurationMs(frame.capture_commit_ns, frame.aimer_end_ns),
        DurationMs(frame.capture_start_ns, frame.aimer_end_ns),
    };
    for (double value : timing_values)
    {
      WriteNumber(frame_output, value);
      frame_output << '\t';
    }
    frame_output << frame.armor_count << '\t' << frame.pnp_count << '\t'
                 << (frame.tracking ? 1 : 0) << '\t' << frame.target_id << '\t';
    for (double value : frame.target_position)
    {
      WriteNumber(frame_output, value);
      frame_output << '\t';
    }
    for (double value : frame.target_velocity)
    {
      WriteNumber(frame_output, value);
      frame_output << '\t';
    }
    WriteNumber(frame_output, frame.target_yaw);
    frame_output << '\t';
    WriteNumber(frame_output, frame.target_yaw_velocity);
    frame_output << '\t';
    WriteNumber(frame_output, frame.target_radius_1);
    frame_output << '\t';
    WriteNumber(frame_output, frame.target_radius_2);
    frame_output << '\t';
    WriteNumber(frame_output, frame.target_dz);
    frame_output << '\t' << (frame.mpc_attempted ? 1 : 0) << '\t'
                 << (frame.mpc_solver_ran ? 1 : 0) << '\t'
                 << (frame.mpc_output_finite ? 1 : 0) << '\t'
                 << (frame.mpc_plan_accepted ? 1 : 0) << '\t' << frame.yaw_rc << '\t'
                 << frame.yaw_solved << '\t' << frame.yaw_status << '\t'
                 << frame.yaw_iterations << '\t' << frame.roll_rc << '\t'
                 << frame.roll_solved << '\t' << frame.roll_status << '\t'
                 << frame.roll_iterations << '\t' << (frame.aimer_control ? 1 : 0) << '\t'
                 << (frame.aimer_fire ? 1 : 0) << '\t' << (frame.aimer_used_mpc ? 1 : 0)
                 << '\t' << (frame.aimer_output_finite ? 1 : 0);
    for (double value : frame.gimbal_plan)
    {
      frame_output << '\t';
      WriteNumber(frame_output, value);
    }
    frame_output << '\n';

    for (std::size_t index = 0; index < frame.detections.size(); ++index)
    {
      const DetectionRecord& detection = frame.detections[index];
      detection_output << timestamp_us << '\t' << index << '\t' << detection.color << '\t'
                       << detection.type << '\t' << detection.number << '\t';
      WriteNumber(detection_output, detection.confidence);
      detection_output << '\t' << (detection.pnp_valid ? 1 : 0) << '\t';
      WriteNumber(detection_output, detection.pnp_error_px);
      for (double value : detection.corners)
      {
        detection_output << '\t';
        WriteNumber(detection_output, value);
      }
      for (double value : detection.translation)
      {
        detection_output << '\t';
        WriteNumber(detection_output, value);
      }
      detection_output << '\n';
    }
  }

  const bool expected_frames_ok =
      config.expected_frames == 0 || source_frames == config.expected_frames;
  const bool complete_pipeline_ok =
      config.expected_frames != 0 && capture_count == config.expected_frames &&
      sync_count == config.expected_frames && detector_count == config.expected_frames &&
      tracker_count == config.expected_frames && aimer_count == config.expected_frames &&
      sync_drops == 0 && tracker_overwrites == 0;
  const bool basic_functional_ok =
      source_complete && source_ok && expected_frames_ok && capture_count > 0 &&
      sync_count > 0 && detector_count > 0 && armor_total > 0 && tracker_count > 0 &&
      tracking_count > 0 && aimer_count > 0 && mpc_finite_count > 0 &&
      mpc_accepted_count > 0 && nonfinite_output_count == 0 && fire_true_count == 0;
  const bool functional_ok =
      basic_functional_ok && (!config.require_complete_pipeline || complete_pipeline_ok);
  const int exit_code = functional_ok ? 0 : 2;
  const int64_t pipeline_end_ns =
      std::max({source_complete_ns, last_capture_ns, last_detector_ns, last_tracker_ns,
                last_aimer_ns});
  const double source_active_ms = DurationMs(started_ns, source_complete_ns);
  const double detector_active_ms = DurationMs(first_detector_ns, last_detector_ns);
  const double tracker_active_ms = DurationMs(first_tracker_ns, last_tracker_ns);
  const double aimer_active_ms = DurationMs(first_aimer_ns, last_aimer_ns);
  const double pipeline_active_ms = DurationMs(started_ns, pipeline_end_ns);
  const double event_active_ms = DurationMs(started_ns, last_event_ns);
  const double pipeline_drain_ms =
      source_complete_ns == 0 || pipeline_end_ns < source_complete_ns
          ? kNaN
          : static_cast<double>(pipeline_end_ns - source_complete_ns) / 1000000.0;

  summary_output << "variant\trun_id\tstatus\texit_code\trequire_complete_pipeline"
                    "\tcomplete_pipeline_ok\tsource_complete\tsource_ok"
                    "\texpected_frames\tsource_frames\tcapture_frames\tsynced_frames"
                    "\tsync_drops\tdetector_frames\tarmor_total\tpnp_total"
                    "\ttracker_frames\ttracker_overwrites\ttracking_frames\taimer_frames"
                    "\tmpc_attempted\tmpc_solver_ran\tmpc_solver_converged"
                    "\tmpc_output_finite\tmpc_plan_accepted\tnonfinite_outputs"
                    "\tfire_true\tsource_active_ms\tdetector_active_ms"
                    "\ttracker_active_ms\taimer_active_ms\tpipeline_drain_ms"
                    "\tpipeline_active_ms\tevent_active_ms"
                    "\tpipeline_timing_frames\tpipeline_no_free_count"
                    "\tdetector_pipeline_counters_recorded\tdetector_admitted"
                    "\tdetector_completed\tdetector_prepare_drop\tdetector_no_free"
                    "\tdetector_publish_fail\tdetector_unsynced"
                    "\tdetector_claim_miss\tdetector_subscriber_drop"
                    "\tdetector_infer_fail\tdetector_post_fail\n";
  summary_output << config.variant << '\t' << config.run_id << '\t'
                 << (functional_ok ? "PASS" : "FAIL") << '\t' << exit_code << '\t'
                 << (config.require_complete_pipeline ? 1 : 0) << '\t'
                 << (complete_pipeline_ok ? 1 : 0) << '\t' << (source_complete ? 1 : 0)
                 << '\t' << (source_ok ? 1 : 0) << '\t' << config.expected_frames << '\t'
                 << source_frames << '\t' << capture_count << '\t' << sync_count << '\t'
                 << sync_drops << '\t' << detector_count << '\t' << armor_total << '\t'
                 << pnp_total << '\t' << tracker_count << '\t' << tracker_overwrites
                 << '\t' << tracking_count << '\t' << aimer_count << '\t'
                 << mpc_attempt_count << '\t' << mpc_solver_count << '\t'
                 << mpc_converged_count << '\t' << mpc_finite_count << '\t'
                 << mpc_accepted_count << '\t' << nonfinite_output_count << '\t'
                 << fire_true_count << '\t';
  const std::array<double, 7> active_time_values{
      source_active_ms,  detector_active_ms, tracker_active_ms, aimer_active_ms,
      pipeline_drain_ms, pipeline_active_ms, event_active_ms,
  };
  for (std::size_t index = 0; index < active_time_values.size(); ++index)
  {
    WriteNumber(summary_output, active_time_values[index]);
    if (index + 1U != active_time_values.size())
    {
      summary_output << '\t';
    }
  }
  summary_output << '\t' << pipeline_timings.size() << '\t' << PipelineNoFreeCount()
                 << '\t' << (detector_pipeline_counters_recorded ? 1 : 0);
  for (uint64_t value : detector_pipeline_counters)
  {
    summary_output << '\t' << value;
  }
  summary_output << '\n';

  frame_output.close();
  pipeline_timing_output.close();
  async_pipeline_timing_output.close();
  tracker_pipeline_timing_output.close();
  detection_output.close();
  summary_output.close();
  if (frame_output.fail() || pipeline_timing_output.fail() ||
      async_pipeline_timing_output.fail() || tracker_pipeline_timing_output.fail() ||
      detection_output.fail() || summary_output.fail())
  {
    std::fprintf(stderr, "ReplayBenchmark failed to write output files\n");
    return 3;
  }

  std::ofstream sentinel_output(config.output_dir / "complete.sentinel");
  if (!sentinel_output)
  {
    std::fprintf(stderr, "ReplayBenchmark failed to open completion sentinel\n");
    return 3;
  }
  sentinel_output << (functional_ok ? "PASS" : "FAIL") << '\t' << exit_code << '\n';
  sentinel_output.close();
  if (sentinel_output.fail())
  {
    std::fprintf(stderr, "ReplayBenchmark failed to write completion sentinel\n");
    return 3;
  }

  RunState& state = State();
  std::lock_guard<std::mutex> lock(state.mutex);
  state.artifacts_written = true;
  state.exit_code = exit_code;
  return exit_code;
}
}  // namespace AutoAimReplayBenchmark
