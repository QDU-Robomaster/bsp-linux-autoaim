#pragma once

#include <algorithm>
#include <array>
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
  std::condition_variable condition{};
  std::map<uint64_t, FrameRecord> frames{};
  bool pipeline_ready{false};
  bool source_complete{false};
  bool source_ok{false};
  uint64_t source_frames{0};
  uint64_t sync_drops{0};
  uint64_t tracker_overwrites{0};
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
  return state.condition.wait_for(
      lock, std::chrono::milliseconds(GetConfig().timeout_ms),
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
      lock, std::chrono::milliseconds(config.frame_timeout_ms), [&state, timestamp_us]
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
  bool source_complete = false;
  bool source_ok = false;
  uint64_t source_frames = 0;
  uint64_t sync_drops = 0;
  uint64_t tracker_overwrites = 0;
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
    started_ns = state.started_ns;
    source_complete_ns = state.source_complete_ns;
    last_event_ns = state.last_event_ns;
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
  std::ofstream detection_output(config.output_dir / "detections.tsv");
  std::ofstream summary_output(config.output_dir / "run_summary.tsv");
  if (!frame_output || !detection_output || !summary_output)
  {
    std::fprintf(stderr, "ReplayBenchmark failed to open output files\n");
    return 3;
  }

  frame_output << std::setprecision(12);
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
                    "\tpipeline_active_ms\tevent_active_ms\n";
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
  summary_output << '\n';

  frame_output.close();
  detection_output.close();
  summary_output.close();

  std::ofstream sentinel_output(config.output_dir / "complete.sentinel");
  sentinel_output << (functional_ok ? "PASS" : "FAIL") << '\t' << exit_code << '\n';
  sentinel_output.close();

  RunState& state = State();
  std::lock_guard<std::mutex> lock(state.mutex);
  state.artifacts_written = true;
  state.exit_code = exit_code;
  return exit_code;
}
}  // namespace AutoAimReplayBenchmark
