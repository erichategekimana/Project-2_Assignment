#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include <math.h>

#define STUDENT_ID_HEADER "001"
#define INITIAL_DEVICE_CAPACITY 6

/* Configurable Operating Limit Thresholds */
#define TEMP_MIN 10.0f
#define TEMP_MAX 40.0f
#define PRESSURE_MIN 950
#define PRESSURE_MAX 1050
#define VOLTAGE_MIN 3.30f
#define VOLTAGE_MAX 4.20f

/* Custom Callback Thresholds: Maximum allowable delta before triggering drift warning */
#define DRIFT_TEMP_DELTA 15.0f
#define DRIFT_PRESSURE_DELTA 60.0f
#define DRIFT_VOLTAGE_DELTA 0.50f

/* Device Types */
typedef enum {
    DEVICE_TEMP = 1,
    DEVICE_PRESSURE = 2,
    DEVICE_VOLTAGE = 3
} DeviceType;

/* Operational Status Flags */
typedef enum {
    STATUS_NORMAL = 0,
    STATUS_ALERT = 1
} DeviceStatus;

/* 1. Device Data Structure with Reading Union */
struct Device {
    int id;
    char name[40];
    int type; /* 1: Temperature, 2: Pressure, 3: Battery Voltage */
    union {
        float temperature; /* Celsius */
        int pressure;      /* hPa */
        float voltage;     /* Volts */
    } readings;
    int status; /* 0: Normal, 1: Alert */

    /* Metadata for alert handling and custom drift callback */
    float prev_numeric_reading;
    bool has_prev_reading;
    char alert_reason[96];
    char active_callback_name[48];
    bool custom_drift_flagged;
};

/* Dynamic Engine Container */
typedef struct {
    struct Device *devices;
    size_t count;
    size_t capacity;

    /* Metrics for summary reporting */
    size_t total_readings_processed;
    size_t normal_readings_count;
    size_t abnormal_readings_count;
    size_t temp_alerts;
    size_t pressure_alerts;
    size_t voltage_alerts;
    size_t custom_drift_alerts;
} IoTEngine;

/* Callback function pointer types */
typedef void (*DeviceCallback)(struct Device *);
typedef void (*MenuHandler)(IoTEngine *);

/* --- Core Function Prototypes --- */
void display_header(void);
void clear_input_buffer(void);
int get_valid_int(const char *prompt, int min, int max);
const char* get_type_name(int type);

/* Required Callback Engine Interface */
void process_device(struct Device *device, void (*callback)(struct Device *));

/* Standard Device Callbacks */
void temperature_monitor(struct Device *device);
void pressure_monitor(struct Device *device);
void battery_monitor(struct Device *device);

/* Custom Callback Function */
void sensor_drift_monitor(struct Device *device);

/* Callback Dispatch Resolver */
DeviceCallback resolve_device_callback(int type);

/* Menu Actions */
void action_register_device(IoTEngine *engine);
void action_display_devices(IoTEngine *engine);
void action_remove_device(IoTEngine *engine);
void action_simulate_readings(IoTEngine *engine);
void action_process_all_callbacks(IoTEngine *engine);
void action_display_alerts(IoTEngine *engine);
void action_monitoring_summary(IoTEngine *engine);
void action_exit_engine(IoTEngine *engine);

/* Seed Dataset */
static const struct Device INITIAL_SEEDS[] = {
    {101, "Server_Rack_Temp",  DEVICE_TEMP,     {.temperature = 24.5f}, STATUS_NORMAL, 24.5f, true,  "None", "None", false},
    {102, "Hydraulic_Line_A",  DEVICE_PRESSURE, {.pressure = 1013},     STATUS_NORMAL, 1013.0f, true, "None", "None", false},
    {103, "Backup_Pack_B",     DEVICE_VOLTAGE,  {.voltage = 3.85f},     STATUS_NORMAL, 3.85f, true,  "None", "None", false},
    {104, "Exhaust_Chamber",   DEVICE_TEMP,     {.temperature = 28.0f}, STATUS_NORMAL, 28.0f, true,  "None", "None", false},
    {105, "Boiler_Intake",     DEVICE_PRESSURE, {.pressure = 998},      STATUS_NORMAL, 998.0f, true,  "None", "None", false},
    {106, "Solar_LiPo_Node",   DEVICE_VOLTAGE,  {.voltage = 4.10f},     STATUS_NORMAL, 4.10f, true,  "None", "None", false}
};

/* --- Main Entry Point --- */
int main(void) {
    srand((unsigned int)time(NULL));

    IoTEngine engine = {
        .devices = NULL,
        .count = 0,
        .capacity = INITIAL_DEVICE_CAPACITY,
        .total_readings_processed = 0,
        .normal_readings_count = 0,
        .abnormal_readings_count = 0,
        .temp_alerts = 0,
        .pressure_alerts = 0,
        .voltage_alerts = 0,
        .custom_drift_alerts = 0
    };

    /* Allocate dynamic memory for the initial device array */
    engine.devices = (struct Device *)malloc(engine.capacity * sizeof(struct Device));
    if (engine.devices == NULL) {
        fprintf(stderr, "FATAL ERROR: Failed to allocate dynamic memory for IoT engine.\n");
        return EXIT_FAILURE;
    }

    /* Populate initial baseline devices */
    size_t seed_count = sizeof(INITIAL_SEEDS) / sizeof(INITIAL_SEEDS[0]);
    memcpy(engine.devices, INITIAL_SEEDS, sizeof(INITIAL_SEEDS));
    engine.count = seed_count;

    /* Menu Dispatch Table using Function Pointers */
    MenuHandler menu_dispatch[] = {
        action_register_device,        /* 1 */
        action_display_devices,         /* 2 */
        action_remove_device,          /* 3 */
        action_simulate_readings,      /* 4 */
        action_process_all_callbacks,  /* 5 */
        action_display_alerts,         /* 6 */
        action_monitoring_summary,     /* 7 */
        action_exit_engine             /* 8 */
    };
    const int total_menu_actions = (int)(sizeof(menu_dispatch) / sizeof(menu_dispatch[0]));

    int choice = 0;
    while (choice != 8) {
        display_header();
        printf("1. Register a new device\n");
        printf("2. Display all registered devices\n");
        printf("3. Remove a device using its ID\n");
        printf("4. Generate simulated readings for registered devices\n");
        printf("5. Process device readings through callback functions\n");
        printf("6. Display detected alerts\n");
        printf("7. Generate a monitoring summary\n");
        printf("8. Exit the program\n");
        printf("----------------------------------------------------------------------\n");

        choice = get_valid_int("Select operation (1-8): ", 1, total_menu_actions);
        printf("\n");
        menu_dispatch[choice - 1](&engine);
        printf("\n");
    }

    return EXIT_SUCCESS;
}

/* --- Header & Formatted Input Utilities --- */
void display_header(void) {
    printf("======================================================================\n");
    printf("Student ID: %s\n", STUDENT_ID_HEADER);
    printf("Project: Dynamic IoT Event Monitoring and Callback Engine\n");
    printf("Custom Callback: Sensor Drift & Sudden Delta Detector\n");
    printf("======================================================================\n");
}

int get_valid_int(const char *prompt, int min, int max) {
    int val;
    char buffer[64];
    while (1) {
        printf("%s", prompt);
        if (fgets(buffer, sizeof(buffer), stdin) != NULL) {
            if (sscanf(buffer, "%d", &val) == 1 && val >= min && val <= max) {
                return val;
            }
        }
        printf("  [!] Invalid input. Enter an integer between %d and %d.\n", min, max);
    }
}

const char* get_type_name(int type) {
    switch (type) {
        case DEVICE_TEMP:     return "Temperature Sensor";
        case DEVICE_PRESSURE: return "Pressure Sensor";
        case DEVICE_VOLTAGE:  return "Battery Monitor";
        default:              return "Unknown Sensor";
    }
}

/* --- Callback Implementation & Dispatch Interface --- */

/* Image requirement: Exact callback engine invocation signature */
void process_device(struct Device *device, void (*callback)(struct Device *)) {
    if (device != NULL && callback != NULL) {
        callback(device);
    }
}

/* 1. Temperature Monitor Callback */
void temperature_monitor(struct Device *device) {
    /* Safe union access for active type */
    float temp = device->readings.temperature;
    strncpy(device->active_callback_name, "temperature_monitor()", sizeof(device->active_callback_name) - 1);

    if (temp < TEMP_MIN) {
        device->status = STATUS_ALERT;
        snprintf(device->alert_reason, sizeof(device->alert_reason),
                 "Critical Undetemperature (%.2f C < Limit %.2f C)", temp, TEMP_MIN);
    } else if (temp > TEMP_MAX) {
        device->status = STATUS_ALERT;
        snprintf(device->alert_reason, sizeof(device->alert_reason),
                 "Critical Overheating (%.2f C > Limit %.2f C)", temp, TEMP_MAX);
    } else {
        device->status = STATUS_NORMAL;
        strcpy(device->alert_reason, "Nominal Operating Temperature");
    }
}

/* 2. Pressure Monitor Callback */
void pressure_monitor(struct Device *device) {
    /* Safe union access for active type */
    int press = device->readings.pressure;
    strncpy(device->active_callback_name, "pressure_monitor()", sizeof(device->active_callback_name) - 1);

    if (press < PRESSURE_MIN) {
        device->status = STATUS_ALERT;
        snprintf(device->alert_reason, sizeof(device->alert_reason),
                 "Loss of Pressure (Underpressure: %d hPa < Limit %d hPa)", press, PRESSURE_MIN);
    } else if (press > PRESSURE_MAX) {
        device->status = STATUS_ALERT;
        snprintf(device->alert_reason, sizeof(device->alert_reason),
                 "Chamber Overpressurization (%d hPa > Limit %d hPa)", press, PRESSURE_MAX);
    } else {
        device->status = STATUS_NORMAL;
        strcpy(device->alert_reason, "Nominal Operating Pressure");
    }
}

/* 3. Battery Monitor Callback */
void battery_monitor(struct Device *device) {
    /* Safe union access for active type */
    float volt = device->readings.voltage;
    strncpy(device->active_callback_name, "battery_monitor()", sizeof(device->active_callback_name) - 1);

    if (volt < VOLTAGE_MIN) {
        device->status = STATUS_ALERT;
        snprintf(device->alert_reason, sizeof(device->alert_reason),
                 "Critical Undervoltage / Depletion (%.2f V < Limit %.2f V)", volt, VOLTAGE_MIN);
    } else if (volt > VOLTAGE_MAX) {
        device->status = STATUS_ALERT;
        snprintf(device->alert_reason, sizeof(device->alert_reason),
                 "Battery Overcharging Spike (%.2f V > Limit %.2f V)", volt, VOLTAGE_MAX);
    } else {
        device->status = STATUS_NORMAL;
        strcpy(device->alert_reason, "Nominal Bus Voltage");
    }
}

/* 6. Custom Callback Function: Sensor Drift Detector */
void sensor_drift_monitor(struct Device *device) {
    if (!device->has_prev_reading) {
        device->custom_drift_flagged = false;
        return;
    }

    float current_val = 0.0f;
    float max_delta = 0.0f;

    switch (device->type) {
        case DEVICE_TEMP:
            current_val = device->readings.temperature;
            max_delta = DRIFT_TEMP_DELTA;
            break;
        case DEVICE_PRESSURE:
            current_val = (float)device->readings.pressure;
            max_delta = DRIFT_PRESSURE_DELTA;
            break;
        case DEVICE_VOLTAGE:
            current_val = device->readings.voltage;
            max_delta = DRIFT_VOLTAGE_DELTA;
            break;
        default:
            return;
    }

    float delta = fabsf(current_val - device->prev_numeric_reading);
    if (delta > max_delta) {
        device->custom_drift_flagged = true;
        device->status = STATUS_ALERT;
        snprintf(device->alert_reason, sizeof(device->alert_reason),
                 "Sensor Drift Alert: Sudden swing of %.2f units detected", delta);
        strncpy(device->active_callback_name, "sensor_drift_monitor() [Custom]",
                sizeof(device->active_callback_name) - 1);
    } else {
        device->custom_drift_flagged = false;
    }
}

/* Resolver returning the corresponding callback function pointer */
DeviceCallback resolve_device_callback(int type) {
    switch (type) {
        case DEVICE_TEMP:     return temperature_monitor;
        case DEVICE_PRESSURE: return pressure_monitor;
        case DEVICE_VOLTAGE:  return battery_monitor;
        default:              return NULL;
    }
}

/* --- Dynamic Memory Operations & Menu Actions --- */

/* 1. Register a new device (Dynamic Memory Growth via realloc) */
void action_register_device(IoTEngine *engine) {
    printf("--- Register New IoT Device ---\n");
    int new_id = get_valid_int("Enter Unique Device ID (100-999): ", 100, 999);

    /* Pointer traversal to prevent duplicate IDs */
    struct Device *ptr = engine->devices;
    struct Device *end = engine->devices + engine->count;
    for (; ptr < end; ++ptr) {
        if (ptr->id == new_id) {
            printf("[Rejected] Device with ID %d is already registered.\n", new_id);
            return;
        }
    }

    char name_buffer[40];
    printf("Enter Device Name (max 39 chars): ");
    if (fgets(name_buffer, sizeof(name_buffer), stdin) == NULL) return;
    name_buffer[strcspn(name_buffer, "\r\n")] = '\0';
    if (strlen(name_buffer) == 0) {
        strcpy(name_buffer, "Unnamed_Sensor");
    }

    printf("Select Device Type:\n");
    printf("  1. Temperature Sensor\n");
    printf("  2. Pressure Sensor\n");
    printf("  3. Battery Voltage Monitor\n");
    int type = get_valid_int("Choice (1-3): ", 1, 3);

    /* Expand dynamic memory pool */
    size_t new_cap = engine->count + 1;
    struct Device *temp = (struct Device *)realloc(engine->devices, new_cap * sizeof(struct Device));
    if (temp == NULL) {
        printf("[Error] Memory reallocation failed. Unable to register device.\n");
        return;
    }

    engine->devices = temp;
    engine->capacity = new_cap;

    /* Write directly to new slot using pointer arithmetic */
    struct Device *target = engine->devices + engine->count;
    target->id = new_id;
    strncpy(target->name, name_buffer, sizeof(target->name) - 1);
    target->name[sizeof(target->name) - 1] = '\0';
    target->type = type;
    target->status = STATUS_NORMAL;
    target->has_prev_reading = false;
    target->custom_drift_flagged = false;
    strcpy(target->alert_reason, "None");
    strcpy(target->active_callback_name, "None");

    /* Initialize type-specific union reading to nominal defaults */
    if (type == DEVICE_TEMP) {
        target->readings.temperature = 22.0f;
        target->prev_numeric_reading = 22.0f;
    } else if (type == DEVICE_PRESSURE) {
        target->readings.pressure = 1000;
        target->prev_numeric_reading = 1000.0f;
    } else {
        target->readings.voltage = 3.70f;
        target->prev_numeric_reading = 3.70f;
    }

    engine->count++;
    printf("[Success] Device '%s' (ID: %d) registered. Allocated capacity is now %zu.\n",
           target->name, target->id, engine->capacity);
}

/* 2. Display all registered devices (Pointer Traversal) */
void action_display_devices(IoTEngine *engine) {
    if (engine->count == 0) {
        printf("[Info] No devices currently registered.\n");
        return;
    }

    printf("-----------------------------------------------------------------------------------------\n");
    printf("%-6s | %-20s | %-18s | %-12s | %-8s\n", "ID", "Device Name", "Type", "Active Value", "Status");
    printf("-----------------------------------------------------------------------------------------\n");

    struct Device *ptr = engine->devices;
    struct Device *end = engine->devices + engine->count;

    for (; ptr < end; ++ptr) {
        char reading_str[32];
        /* Respect active union type */
        if (ptr->type == DEVICE_TEMP) {
            snprintf(reading_str, sizeof(reading_str), "%.2f C", ptr->readings.temperature);
        } else if (ptr->type == DEVICE_PRESSURE) {
            snprintf(reading_str, sizeof(reading_str), "%d hPa", ptr->readings.pressure);
        } else if (ptr->type == DEVICE_VOLTAGE) {
            snprintf(reading_str, sizeof(reading_str), "%.2f V", ptr->readings.voltage);
        } else {
            snprintf(reading_str, sizeof(reading_str), "N/A");
        }

        const char *stat_str = (ptr->status == STATUS_NORMAL) ? "NORMAL" : "ALERT";
        printf("%-6d | %-20s | %-18s | %-12s | %-8s\n",
               ptr->id, ptr->name, get_type_name(ptr->type), reading_str, stat_str);
    }
    printf("-----------------------------------------------------------------------------------------\n");
    printf("Total Active Devices: %zu | Memory Capacity: %zu slots\n", engine->count, engine->capacity);
}

/* 3. Remove a device (Dynamic Memory Shrinking via memmove and realloc) */
void action_remove_device(IoTEngine *engine) {
    if (engine->count == 0) {
        printf("[Info] No devices registered to remove.\n");
        return;
    }

    int target_id = get_valid_int("Enter Device ID to unregister: ", 100, 999);
    size_t found_idx = engine->count;

    for (size_t i = 0; i < engine->count; ++i) {
        if ((engine->devices + i)->id == target_id) {
            found_idx = i;
            break;
        }
    }

    if (found_idx == engine->count) {
        printf("[Error] Device with ID %d was not found.\n", target_id);
        return;
    }

    /* Shift subsequent records backward */
    size_t items_to_move = engine->count - found_idx - 1;
    if (items_to_move > 0) {
        memmove(engine->devices + found_idx,
                engine->devices + found_idx + 1,
                items_to_move * sizeof(struct Device));
    }
    engine->count--;

    /* Shrink the dynamic array */
    if (engine->count > 0) {
        struct Device *temp = (struct Device *)realloc(engine->devices, engine->count * sizeof(struct Device));
        if (temp != NULL) {
            engine->devices = temp;
            engine->capacity = engine->count;
        }
    } else {
        free(engine->devices);
        engine->devices = NULL;
        engine->capacity = 0;
    }

    printf("[Success] Device %d removed. Heap pool shrunk to %zu elements.\n", target_id, engine->capacity);
}

/* 4. Generate at least 10 Simulated Readings and Process via Callbacks */
void action_simulate_readings(IoTEngine *engine) {
    if (engine->count == 0) {
        printf("[Info] Register at least one device before generating readings.\n");
        return;
    }

    const int target_readings = 12; /* Guarantees >= 10 readings per execution */
    printf("================ Simulating %d Live Telemetry Readings ================\n", target_readings);

    for (int i = 0; i < target_readings; ++i) {
        /* Select device via round-robin / randomized device indexing */
        size_t dev_idx = (size_t)i % engine->count;
        struct Device *dev = engine->devices + dev_idx;

        /* Save previous reading for drift calculation */
        if (dev->type == DEVICE_TEMP) {
            dev->prev_numeric_reading = dev->readings.temperature;
        } else if (dev->type == DEVICE_PRESSURE) {
            dev->prev_numeric_reading = (float)dev->readings.pressure;
        } else {
            dev->prev_numeric_reading = dev->readings.voltage;
        }
        dev->has_prev_reading = true;

        /* Generate new random value across realistic bounds (including nominal & abnormal) */
        char val_str[32];
        if (dev->type == DEVICE_TEMP) {
            /* Range: -5.0 C to 55.0 C */
            float rand_temp = -5.0f + ((float)rand() / (float)RAND_MAX) * 60.0f;
            dev->readings.temperature = rand_temp;
            snprintf(val_str, sizeof(val_str), "%6.2f C", rand_temp);
        } else if (dev->type == DEVICE_PRESSURE) {
            /* Range: 880 to 1120 hPa */
            int rand_press = 880 + (rand() % 241);
            dev->readings.pressure = rand_press;
            snprintf(val_str, sizeof(val_str), "%d hPa", rand_press);
        } else if (dev->type == DEVICE_VOLTAGE) {
            /* Range: 2.90V to 4.45V */
            float rand_volt = 2.90f + ((float)rand() / (float)RAND_MAX) * 1.55f;
            dev->readings.voltage = rand_volt;
            snprintf(val_str, sizeof(val_str), "%6.2f V", rand_volt);
        }

        /* Dispatch reading to appropriate callback via function pointer */
        DeviceCallback target_cb = resolve_device_callback(dev->type);
        process_device(dev, target_cb);

        /* Evaluate with custom callback via function pointer */
        process_device(dev, sensor_drift_monitor);

        /* Update Engine Aggregates */
        engine->total_readings_processed++;
        if (dev->status == STATUS_NORMAL) {
            engine->normal_readings_count++;
        } else {
            engine->abnormal_readings_count++;
            if (dev->type == DEVICE_TEMP) engine->temp_alerts++;
            else if (dev->type == DEVICE_PRESSURE) engine->pressure_alerts++;
            else if (dev->type == DEVICE_VOLTAGE) engine->voltage_alerts++;

            if (dev->custom_drift_flagged) engine->custom_drift_alerts++;
        }

        /* Display individual simulated result per requirement */
        printf("[%02d] Device [%d - %-18s] | Type: %-18s | Value: %-10s | Result: %s\n",
               i + 1, dev->id, dev->name, get_type_name(dev->type), val_str,
               (dev->status == STATUS_NORMAL) ? "OK [NOMINAL]" : dev->alert_reason);
    }
    printf("========================================================================\n");
    printf("[Complete] %d readings dispatched through type-specific callbacks.\n", target_readings);
}

/* 5. Process current readings for all devices using callback dispatch */
void action_process_all_callbacks(IoTEngine *engine) {
    if (engine->count == 0) {
        printf("[Info] No devices registered.\n");
        return;
    }

    printf("--- Executing Callback Pipeline Across All Devices ---\n");
    struct Device *ptr = engine->devices;
    struct Device *end = engine->devices + engine->count;

    for (; ptr < end; ++ptr) {
        DeviceCallback cb = resolve_device_callback(ptr->type);
        process_device(ptr, cb);
        process_device(ptr, sensor_drift_monitor);

        printf("Dispatched Callback -> Device %d [%s] handled by '%s' => Status: %s\n",
               ptr->id, ptr->name, ptr->active_callback_name,
               (ptr->status == STATUS_NORMAL) ? "NORMAL" : "ALERT");
    }
}

/* 6. Display Detected Alerts */
void action_display_alerts(IoTEngine *engine) {
    if (engine->count == 0) {
        printf("[Info] No devices available.\n");
        return;
    }

    printf("======================== Active Alert Audit Log ========================\n");
    size_t active_alerts = 0;
    struct Device *ptr = engine->devices;
    struct Device *end = engine->devices + engine->count;

    for (; ptr < end; ++ptr) {
        if (ptr->status == STATUS_ALERT) {
            active_alerts++;
            char val_buf[32];
            char range_buf[48];

            if (ptr->type == DEVICE_TEMP) {
                snprintf(val_buf, sizeof(val_buf), "%.2f C", ptr->readings.temperature);
                snprintf(range_buf, sizeof(range_buf), "%.1f C - %.1f C", TEMP_MIN, TEMP_MAX);
            } else if (ptr->type == DEVICE_PRESSURE) {
                snprintf(val_buf, sizeof(val_buf), "%d hPa", ptr->readings.pressure);
                snprintf(range_buf, sizeof(range_buf), "%d hPa - %d hPa", PRESSURE_MIN, PRESSURE_MAX);
            } else {
                snprintf(val_buf, sizeof(val_buf), "%.2f V", ptr->readings.voltage);
                snprintf(range_buf, sizeof(range_buf), "%.2f V - %.2f V", VOLTAGE_MIN, VOLTAGE_MAX);
            }

            printf("ALERT #%zu\n", active_alerts);
            printf("  Device:                 %s (ID: %d)\n", ptr->name, ptr->id);
            printf("  Type:                   %s\n", get_type_name(ptr->type));
            printf("  Current Reading:        %s\n", val_buf);
            printf("  Operating Range:        %s\n", range_buf);
            printf("  Reason for Alert:       %s\n", ptr->alert_reason);
            printf("  Processing Callback:    %s\n", ptr->active_callback_name);
            printf("--------------------------------------------------------------------\n");
        }
    }

    if (active_alerts == 0) {
        printf("[Info] All devices operating within nominal parameters. Zero active alerts.\n");
    }
    printf("========================================================================\n");
}

/* 7. Generate a Monitoring Summary */
void action_monitoring_summary(IoTEngine *engine) {
    printf("======================= Engine Monitoring Summary =======================\n");
    printf("Total Registered Devices:             %zu\n", engine->count);
    printf("Total Simulated Readings Processed:   %zu\n", engine->total_readings_processed);
    printf("Normal Readings:                      %zu\n", engine->normal_readings_count);
    printf("Abnormal Readings (Alerts Triggered): %zu\n", engine->abnormal_readings_count);
    printf("-------------------------------------------------------------------------\n");
    printf("Alert Breakdown By Device Type:\n");
    printf("  - Temperature Over/Under Alerts:    %zu\n", engine->temp_alerts);
    printf("  - Pressure Discrepancy Alerts:      %zu\n", engine->pressure_alerts);
    printf("  - Battery Voltage Out-of-Spec:      %zu\n", engine->voltage_alerts);
    printf("-------------------------------------------------------------------------\n");
    printf("Custom Callback Performance:\n");
    printf("  - Sudden Sensor Drift Flags:        %zu events detected\n", engine->custom_drift_alerts);
    printf("=========================================================================\n");
}

/* 8. Exit and Deallocate */
void action_exit_engine(IoTEngine *engine) {
    printf("[Shutdown] Releasing dynamically allocated device array...\n");
    if (engine->devices != NULL) {
        free(engine->devices);
        engine->devices = NULL;
    }
    engine->count = 0;
    engine->capacity = 0;
    printf("[Shutdown] Clean exit complete. Memory zeroed.\n");
}