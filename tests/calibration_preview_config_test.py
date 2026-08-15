import hashlib
import re
from pathlib import Path

import yaml


ROOT = Path(__file__).resolve().parents[1]
CONFIG_DIR = ROOT / "User" / "RunConfig"
MODULES_MANIFEST = ROOT / "Modules" / "modules.yaml"
WORKFLOW_PATH = ROOT / ".github" / "workflows" / "build-test.yml"

EXPECTED_K = [
    2328.6857198980888,
    0.0,
    733.35646250924742,
    0.0,
    2328.6701077899961,
    540.61872869227727,
    0.0,
    0.0,
    1.0,
]
EXPECTED_D = [
    -0.091821039187099038,
    0.46399073468302049,
    0.0026098786426372819,
    0.0009819586010405485,
    -0.47512788503104569,
]
EXPECTED_MODULES = [
    ("camera", "HikCamera"),
    ("camera_frame_sync", "CameraFrameSync"),
    ("shared_topic_rx", "SharedTopic"),
    ("shared_topic_tx", "SharedTopicClient"),
    ("vision_capture", "VisionCapture"),
]
EXPECTED_HIK_RUNTIME_KEYS = [
    "camera_name",
    "image_topic_name",
    "imu_topic_name",
    "gain",
    "exposure_time",
    "external_trigger",
    "acquisition_frame_rate",
    "grab_timeout_ms",
    "image_node_num",
    "rotate_180",
    "wide_decimation_x",
    "wide_decimation_y",
    "wide_trigger_period_us",
    "narrow_trigger_period_us",
]
EXPECTED_CFS_RUNTIME_KEYS = [
    "mode",
    "offset_us",
    "host_topic_domain_name",
    "sync_command_topic_name",
    "sync_result_topic_name",
    "sync_active_level",
    "camera_settle_us",
    "raw_imu_frame",
]
EXPECTED_CAPTURE_KEYS = [
    "mode",
    "output_dir",
    "session_name",
    "record",
    "preview",
    "board",
    "camera_calibration",
    "calibration_sampling",
    "control",
    "filter",
]
EXPECTED_MODULE_PINS = [
    "qdu-future/CameraBase@ff44801075f829e8d1b713b6d4649c24f536649c",
    "qdu-future/HikCamera@8ff874e47776efdd0d52bd86cf9092792aa90497",
    "qdu-future/CaptureFileCamera@44f4820483c885e9b583cc30c62d56bee687b7f8",
    "qdu-future/CameraSync@933cb82ace248aa4e005444fdb231ec20e2af02e",
    "qdu-future/CameraFrameSync@57e416365c1e337e39b3865e26d4d98c8b4b9737",
    "xrobot-org/SharedTopic@489d05938f4b8b24ab950789decb4b43393e8d46",
    "xrobot-org/SharedTopicClient@43365ed2718573f35fe3fdbaadfe4a7565453233",
    "qdu-future/VisionPreview@e2ed25fe76227e0d5ab34705ea013c5f3a6f4c8b",
    "qdu-future/ArmorDetector@efde0d1dbf29a7442fa105ecafe245f2b33a47ea",
    "qdu-future/ArmorTracker@9101af40a0d90d18e4c2a4995205105b7cbe712e",
    "qdu-future/Aimer@14c0dcff11977edeac1420bf549855af43e73f39",
    "qdu-future/VisionCapture@5362fc939991477067146f79dc877ecfcf9ab554",
]
EXPECTED_PRODUCT_CONFIG_SHA256 = {
    "User/xrobot.yaml": "7649f68837d6f983fb2e9a7ef37acc009c2bffa2f991637337a183e0c1b0f0d7",
    "User/RunConfig/hik.yaml": "4c65ab62f4813f53cdf35b7bcc4f0fdc76cd83ae307f0b4a0942da7ab8bca3ea",
    "User/RunConfig/vision_capture.yaml": "12ed6603b664171970e2e9722cd2d53bd200156fe7f9921ed3d48d7e3e8015fa",
    "User/RunConfig/capturefile.yaml": "c184d750a5df8b1a3e197a2e94766b942d540b25931dd2686b89168d311ef420",
    "User/RunConfig/sentry.yaml": "15afac18493dcaab40f99e7966d7b2d6ca9b54d36d0bd9f29d9a4e95490b9d00",
    "User/RunConfig/replay_perf.yaml": "b9283ec3521f15b67cfa638f417c6e984b7d48a3a982238535e0d0520db8674a",
}


def require(condition, message):
    if not condition:
        raise AssertionError(message)


def load_config(name):
    path = CONFIG_DIR / name
    with path.open("r", encoding="utf-8") as stream:
        return yaml.safe_load(stream)


def load_yaml(path):
    with path.open("r", encoding="utf-8") as stream:
        return yaml.safe_load(stream)


def check_integration_contracts():
    for relative_path, expected_sha256 in EXPECTED_PRODUCT_CONFIG_SHA256.items():
        actual_sha256 = hashlib.sha256((ROOT / relative_path).read_bytes()).hexdigest()
        require(actual_sha256 == expected_sha256, f"product config changed: {relative_path}")

    manifest = load_yaml(MODULES_MANIFEST)
    require(manifest == {"modules": EXPECTED_MODULE_PINS}, "exact module pins")
    for entry in manifest["modules"]:
        require(
            re.fullmatch(r"[^/]+/[^@]+@[0-9a-f]{40}", entry) is not None,
            f"module must use an exact 40-hex pin: {entry}",
        )

    workflow = load_yaml(WORKFLOW_PATH)
    steps = workflow["jobs"]["build"]["steps"]
    step_by_name = {step.get("name"): (index, str(step.get("run", ""))) for index, step in enumerate(steps)}
    contracts = [
        (
            "generate intrinsic calibration preset",
            [
                "--config User/RunConfig/camera_intrinsic_calibration.yaml",
                "--output User/xrobot_main.hpp",
            ],
        ),
        (
            "config cmake intrinsic calibration",
            ["-Bbuild_calibration_intrinsic", "-DBUILD_TESTING=OFF"],
        ),
        (
            "build intrinsic calibration",
            ["cmake --build build_calibration_intrinsic --target rm_auto_aim"],
        ),
        (
            "generate handeye calibration preset",
            [
                "--config User/RunConfig/camera_extrinsic_calibration.yaml",
                "--output User/xrobot_main.hpp",
            ],
        ),
        (
            "config cmake handeye calibration",
            ["-Bbuild_calibration_handeye", "-DBUILD_TESTING=OFF"],
        ),
        (
            "build handeye calibration",
            ["cmake --build build_calibration_handeye --target rm_auto_aim"],
        ),
    ]
    indices = []
    for name, fragments in contracts:
        require(name in step_by_name, f"missing workflow step: {name}")
        index, command = step_by_name[name]
        indices.append(index)
        for fragment in fragments:
            require(fragment in command, f"workflow step {name}: {fragment}")
    require(indices == sorted(indices), "calibration generate/configure/build order")

    workflow_text = WORKFLOW_PATH.read_text(encoding="utf-8")
    require("/tmp/calibration-presets" not in workflow_text, "generate-only presets forbidden")


def module_by_id(config, module_id):
    return next(module for module in config["modules"] if module["id"] == module_id)


def check_common(config):
    require(
        [(module["id"], module["name"]) for module in config["modules"]]
        == EXPECTED_MODULES,
        "calibration graph must contain only camera, sync, shared transport, and capture",
    )

    constexprs = config["constexprs"]
    layout = constexprs["MainFrameLayout"]["value"]
    require(list(layout) == ["width", "height", "step", "encoding"], "layout field order")
    require(
        (layout["width"], layout["height"], layout["step"])
        == (1440, 1080, 4320),
        "native BGR8 layout",
    )
    require(layout["encoding"]["expr"] == "CameraTypes::Encoding::BGR8", "BGR8")

    calibration = constexprs["MainCameraCalibration"]["value"]
    require(calibration["native_width"] == 1440, "native calibration width")
    require(calibration["native_height"] == 1080, "native calibration height")
    require(calibration["camera_matrix"] == EXPECTED_K, "frozen camera matrix")
    require(calibration["distortion_coefficients"] == EXPECTED_D, "frozen distortion")

    camera = module_by_id(config, "camera")
    runtime = camera["constructor_args"]["runtime"]
    require(list(runtime) == EXPECTED_HIK_RUNTIME_KEYS, "Hik RuntimeParam field order")
    require(runtime["wide_decimation_x"] == 1, "native horizontal decimation")
    require(runtime["wide_decimation_y"] == 1, "native vertical decimation")
    require(
        layout["width"] * runtime["wide_decimation_x"] == calibration["native_width"],
        "full native width implies ROI x=0",
    )
    require(
        layout["height"] * runtime["wide_decimation_y"] == calibration["native_height"],
        "full native height implies ROI y=0",
    )
    require(runtime["rotate_180"] is False, "native ROI orientation")
    require(runtime["wide_trigger_period_us"] == 50000, "20 Hz wide trigger")
    require(runtime["narrow_trigger_period_us"] == 50000, "20 Hz narrow trigger")

    sync = module_by_id(config, "camera_frame_sync")
    sync_runtime = sync["constructor_args"]["runtime"]
    require(list(sync_runtime) == EXPECTED_CFS_RUNTIME_KEYS, "CFS RuntimeParam field order")
    require(sync_runtime["mode"]["expr"].endswith("SyncMode::TRIGGER"), "trigger mode")
    require(sync_runtime["sync_active_level"] == 1, "active-high trigger")
    require(sync_runtime["camera_settle_us"] == 10000, "camera settle interval")
    require("sync_probe_div" not in sync_runtime, "legacy sync probe field")
    require("target_trigger_hz" not in sync_runtime, "camera owns trigger period")

    rx = module_by_id(config, "shared_topic_rx")["constructor_args"]
    require(
        rx["topic_configs"]
        == [
            ["gimbal_gyro", "host"],
            ["gimbal_accl", "host"],
            ["gimbal_quat", "host"],
            ["camera_sync_result", "host"],
        ],
        "shared receive topics",
    )
    tx = module_by_id(config, "shared_topic_tx")["constructor_args"]
    require(tx["topic_configs"] == [["camera_sync_command", "host"]], "sync command")

    capture_args = module_by_id(config, "vision_capture")["constructor_args"]
    require(list(capture_args) == ["cfg", "sync"], "VisionCapture constructor field order")
    require(capture_args["sync"] == "@camera_frame_sync", "VisionCapture sync input")
    capture = capture_args["cfg"]
    require(list(capture) == EXPECTED_CAPTURE_KEYS, "VisionCapture Config field order")
    require(capture["session_name"] == "", "timestamped session directory")
    require(capture["record"]["save_raw_imu"] is True, "raw IMU recording")
    require(capture["preview"]["enabled"] is True, "preview enabled")
    require(capture["preview"]["output_mode"] == "web", "web preview")
    require(capture["preview"]["queue_capacity"] == 1, "bounded preview queue")
    require(capture["board"]["dictionary"] == "DICT_ARUCO_ORIGINAL", "board dictionary")
    require(capture["board"]["marker_length_m"] == 0.025, "25 mm marker")
    require(capture["camera_calibration"]["cols"] == 8, "GShang columns")
    require(capture["camera_calibration"]["rows"] == 6, "GShang rows")
    require(capture["control"]["stdin_enabled"] is False, "no interactive solver command")
    return capture


def main():
    check_integration_contracts()
    intrinsic = check_common(load_config("camera_intrinsic_calibration.yaml"))
    require(intrinsic["mode"] == "calibrate_camera", "intrinsic mode")
    require(intrinsic["output_dir"] == "runs/calibration/intrinsic", "intrinsic output")
    require(intrinsic["preview"]["web_port"] == 8084, "intrinsic preview port")
    require(
        intrinsic["preview"]["web_stream_name"] == "intrinsic_calibration",
        "intrinsic preview stream",
    )
    require(intrinsic["camera_calibration"]["enabled"] is True, "intrinsic solver")
    intrinsic_sampling = intrinsic["calibration_sampling"]
    require(
        list(intrinsic_sampling)
        == ["enabled", "auto_start", "window_size", "min_accept_interval_us"],
        "intrinsic visual sampling field order",
    )
    require(intrinsic_sampling["enabled"] is True, "intrinsic visual sampling")
    require(intrinsic_sampling["auto_start"] is True, "intrinsic sampling auto start")
    require(intrinsic_sampling["window_size"] == 8, "intrinsic visual window")
    require(
        intrinsic_sampling["min_accept_interval_us"] == 500000,
        "intrinsic accept interval",
    )
    require("max_imu_rotation_jitter_deg" not in intrinsic_sampling, "no intrinsic IMU gate")
    require("max_gyro_norm_dps" not in intrinsic_sampling, "no intrinsic gyro gate")
    require("max_acc_norm_error_mps2" not in intrinsic_sampling, "no intrinsic acc gate")
    require(intrinsic["filter"]["require_synced_imu"] is False, "no intrinsic IMU gate")

    handeye = check_common(load_config("camera_extrinsic_calibration.yaml"))
    require(handeye["mode"] == "calibrate_handeye", "hand-eye dataset mode")
    require(handeye["output_dir"] == "runs/calibration/handeye", "hand-eye output")
    require(handeye["preview"]["web_port"] == 8085, "hand-eye preview port")
    require(
        handeye["preview"]["web_stream_name"] == "handeye_dataset",
        "hand-eye preview stream",
    )
    require(handeye["camera_calibration"]["enabled"] is False, "no hand-eye solver")
    require(handeye["camera_calibration"]["auto_save_views"] == 0, "solver disabled")
    sampling = handeye["calibration_sampling"]
    require(sampling["enabled"] is True, "hand-eye stability sampling")
    require(
        list(sampling)[-1] == "min_sample_rotation_delta_deg",
        "sampling field order",
    )
    require("acceleration_unit" not in sampling, "CameraBase IMU is fixed to m/s^2")
    require(handeye["filter"]["require_synced_imu"] is True, "explicit hand-eye IMU gate")

    print("calibration preview config contracts: PASS (2/2, 12 exact pins, 2 CI builds)")


if __name__ == "__main__":
    main()
