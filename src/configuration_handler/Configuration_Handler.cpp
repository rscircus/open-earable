#include "Configuration_Handler.h"


const int max_config = 15;
const int normal_count = 6;
const configuration_bundle CONFS[max_config]= {
        {30, 30, 62500, 0},     // 1
        {30, 30, 41667, 0},     // 2
        {30, 30, 0, 0},         // 3
        {20, 20, 62500, 0},     // 4
        {20, 20, 41667, 0},     // 5
        {20, 20, 0, 0},         // 6

        {30, 30, 41667, 1},     // 7
        {30, 30, 0, 1},         // 8
        {20, 20, 41667, 1},     // 9
        {20, 20, 0, 1},         // 10
        {0, 0, 62500, 1},      // 11
        {0, 0, 41667, 1},      // 12
        {0, 0, 0, 1},          // 13

        {30, 30, 62500, 1},     // 14  UNSTABLE WITH FILE!
        {20, 20, 62500, 1}     // 15  UNSTABLE WITH FILE!
};

// NOTE! IMU_rate and BARO_rate must be compatible


bool Configuration_Handler::check_active() {
    return _config_active;
}

void Configuration_Handler::update() {
    update_edge_ml();

    if (_buffer_flag) return;
    _buffer_flag = true;

    if (_cycle++ % 2) {
        if (update_pdm()) return;
        update_play();
    } else {
        if (update_play()) return;
        update_pdm();
    }
    BLE.poll();
}

unsigned long last = 0;

void Configuration_Handler::update_edge_ml() {
    unsigned int now = millis();
    if (now - _edge_ml_last > _edge_ml_delay) {
        _edge_ml_last = now;
        _buffer_flag = false;
        edge_ml_generic.update();
    }
}

bool Configuration_Handler::update_pdm() {
    if (pdm_mic_sensor.ready_blocks() < _pdm_min_blocks) return false;

    int cont = pdm_mic_sensor.update_contiguous(_pdm_update_blocks);
    if (check_overlap()) return true; // Make sure that time limit is not reached
    cont = _pdm_update_blocks - cont; // Compute rest: rest 0 => good; rest == total blocks => bad and return
    if (!cont || _pdm_update_blocks == cont) return false;
    pdm_mic_sensor.update_contiguous(cont);
    return check_overlap();
}

bool Configuration_Handler::update_play() {
    if (audio_player.is_mode_tone()) {
        for (int i = 0; i < _play_update_blocks; ++i) {
            audio_player.update();
        }
    }

    if (audio_player.ready_blocks() < _play_min_blocks) return false;

    int cont = audio_player.update_contiguous(_play_update_blocks);

    if (check_overlap()) return true; // Make sure that time limit is not reached
    cont = _play_update_blocks - cont; // Compute rest: rest 0 => good; rest == total blocks => bad and return
    if (!cont || _play_update_blocks == cont) return false;
    audio_player.update_contiguous(cont);
    return check_overlap();
}

void Configuration_Handler::stop_all() {
    SensorConfigurationPacket config;
    config.sampleRate = 0;

    int other_sensor_count = 4;

    for (int i=0; i<other_sensor_count; i++) {
        config.sensorId = i;
        edge_ml_generic.configure_sensor(config);
    }
}

void Configuration_Handler::configure(int config_num, int config_info) {
    configuration_bundle conf;
    SensorConfigurationPacket config;

    // config_num == 0 --> STOP
    // config_num > 0  --> config_num is configuration
    if (config_num < 0 || config_num > max_config + 1) return;

    stop_all();
    _current_conf_num = config_num;
    _cycle = 0;

    if (config_num == 0) {
        _config_active = false;
        return;
    }

    _config_active = true;
    config_num -= 1;

    if (config_num < normal_count) {
        conf = CONFS[config_num];
    } else {
        conf = CONFS[config_num];
        if (!config_info) {
            config_info = 1;
        }
        conf.Play = config_info;
    }

    config.sensorId = ACC_GYRO_MAG;
    config.sampleRate = conf.IMU_rate * _rate_factor;
    edge_ml_generic.configure_sensor(config);
    edge_ml_generic.update();

    config.sensorId = BARO_TEMP;
    config.sampleRate = conf.BARO_rate * _rate_factor;
    edge_ml_generic.configure_sensor(config);
    edge_ml_generic.update();

    config.sensorId = PLAYER;
    config.sampleRate = float(conf.Play);
    Audio_Player::config_callback(&config);

    config.sensorId = PDM_MIC;
    config.sampleRate = float(conf.PDM_rate);
    PDM_MIC_Sensor::config_callback(&config);

    float edge_rate = max(conf.BARO_rate, conf.IMU_rate);

    if (edge_rate == 0) {
        edge_rate = _alternate_loop_rate;
    }

    _edge_ml_delay = (int)(1000.0/edge_rate);
    _edge_ml_last = millis();

    _buffer_interval_time = _edge_ml_delay - _overlap;
    last = millis();
}

bool Configuration_Handler::check_overlap() {
    return (millis() - _edge_ml_last > _buffer_interval_time);
}

void Configuration_Handler::config_callback(SensorConfigurationPacket *config) {
    // Check for Configuration ID
    if (config->sensorId != CONFIGURATION) return;

    int config_num = int(config->sampleRate);
    int config_info = int(config->latency); // Additional extra info

    conf_handler.configure(config_num, config_info);
}

Configuration_Handler conf_handler;

