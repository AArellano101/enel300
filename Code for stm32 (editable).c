/* USER CODE BEGIN PV */
uint8_t metalLatched = 0;
uint8_t pendingDance = 0;
uint8_t dancePlayed = 0;
uint32_t danceAtMs = 0;

uint8_t metalbeatPlaying = 0;
uint8_t metalbeatnoteon = 0;
uint8_t metalbeatindex = 0;
uint32_t metalbeatnextms = 0;
/* USER CODE END PV */

/* USER CODE BEGIN 0 */
// Place below playTone() / stopTone()

typedef uint16_t note_t;

void playSong(const note_t *notes, const uint16_t *durations, int count, uint32_t gapMs)
{
    for (int i = 0; i < count; i++) {
        if (notes[i] > 0) {
            playTone(notes[i]);
        } else {
            stopTone();
        }

        HAL_Delay(durations[i]);
        stopTone();
        HAL_Delay(gapMs);
    }
}

void playStartupTune(void)
{
    static const note_t notes[] = {
        1175, 1175, 932, 1047, 1175, 1175,
        1175, 1175, 1245, 1397, 1245, 1175, 1047,
        1047, 1175, 1245, 1397, 1245, 1175, 1047
    };

    static const uint16_t durations[] = {
        240, 240, 280, 220, 220, 420,
        220, 220, 220, 320, 220, 220, 420,
        260, 220, 220, 320, 220, 220, 500
    };

    playSong(notes, durations, sizeof(notes) / sizeof(notes[0]), 22);
}

void playMetalDetect(void)
{
    static const note_t notes[] = {
        900, 1450, 2400, 2850
    };

    static const uint16_t durations[] = {
        45, 55, 85, 160
    };

    playSong(notes, durations, 4, 10);
}

/* metal beat data */
static const note_t metalbeatnotes[] = {
    1760, 2093, 2349, 2093, 0,
    1760, 2093, 2620, 2349, 0,
    2093, 1760, 2349, 2620, 2093
};

static const uint16_t metalbeatdurations[] = {
    75,  75,  110, 75,  40,
    80,  100, 140, 100, 45,
    90,  90,  110, 150, 220
};

#define METALBEATCOUNT (sizeof(metalbeatnotes) / sizeof(metalbeatnotes[0]))

/* starts looping beat */
void playMetalBeat(void)
{
    metalbeatPlaying = 1;
    metalbeatnoteon = 0;
    metalbeatindex = 0;
    metalbeatnextms = HAL_GetTick();
}

/* call this every loop */
void updateMetalBeat(void)
{
    uint32_t now = HAL_GetTick();

    if (!metalbeatPlaying) return;
    if (now < metalbeatnextms) return;

    if (!metalbeatnoteon)
    {
        if (metalbeatnotes[metalbeatindex] > 0) {
            playTone(metalbeatnotes[metalbeatindex]);
        } else {
            stopTone();
        }

        metalbeatnoteon = 1;
        metalbeatnextms = now + metalbeatdurations[metalbeatindex];
    }
    else
    {
        stopTone();
        metalbeatnoteon = 0;
        metalbeatindex++;

        if (metalbeatindex >= METALBEATCOUNT) {
            metalbeatindex = 0;   // loop forever while on metal
        }

        metalbeatnextms = now + 14;   // small gap between notes
    }
}

/* stops beat immediately */
void stopMetalBeat(void)
{
    metalbeatPlaying = 0;
    metalbeatnoteon = 0;
    metalbeatindex = 0;
    stopTone();
}
/* USER CODE END 0 */


/* USER CODE BEGIN 2 */
HAL_UART_Receive_IT(&huart3, &rx_byte, 1);
stopTone();
playStartupTune();
/* USER CODE END 2 */

/* USER CODE BEGIN WHILE */
while (1)
{
    updateMetalBeat();

    printf("\n\n--------------------------------------\r\n");

    if (ENABLE_METAL_DETECTION == 1) {
        int32_t diff = (int32_t)((int32_t)(storedTimeDelta - signalTimeDelta) * SENSITIVITY);

        uint32_t now = HAL_GetTick();
        if (now - lastPrintTime >= SERIAL_INTERVAL_MS)
        {
            lastPrintTime = now;

            if (storedTimeDelta == 0)
            {
                printf("Calibrating... ");
            }
            else
            {
                if (diff > LED_THRESHOLD) {
                    metalDetectionCount++;
                } else {
                    metalDetectionCount = 0;
                }

                uint8_t metalNow = (metalDetectionCount >= metalDetectionThreshold);

                printf("Metal: %s (count: %lu)\r\n",
                       metalNow ? "YES" : "no",
                       metalDetectionCount);

                if (metalNow && !metalLatched) {
                    playMetalDetect();
                    pendingDance = 1;
                    dancePlayed = 0;
                    danceAtMs = HAL_GetTick() + 1000;
                }

                if (metalNow && pendingDance && !dancePlayed) {
                    if (HAL_GetTick() >= danceAtMs) {
                        playMetalBeat();   // starts looping
                        dancePlayed = 1;
                        pendingDance = 0;
                    }
                }

                if (!metalNow && metalLatched) {
                    pendingDance = 0;
                    dancePlayed = 0;
                    stopMetalBeat();      // stops immediately
                }

                metalLatched = metalNow;
            }
        }

        HAL_Delay(1);
    }

    if (ENABLE_OBJ_DETECTION == 1) {
        HAL_GPIO_WritePin(TRIG_PORT, TRIG_PIN, GPIO_PIN_RESET);
        micros();
        micros();
        HAL_GPIO_WritePin(TRIG_PORT, TRIG_PIN, GPIO_PIN_SET);

        for (int i = 0 ; i < 10; i++) { micros();}

        HAL_GPIO_WritePin(TRIG_PORT, TRIG_PIN, GPIO_PIN_RESET);

        uint32_t t0 = HAL_GetTick();
        while (!echo_done && (HAL_GetTick() - t0 < 100));

        if (echo_pulse_us < 40000) {
            HAL_Delay(100);
            char lcd_buf[17];
            float distance = echo_pulse_us * 11.5 / 610.0;

            int rounded = distance * 100 / 1;

            printf("Distance = %.2f cm, Rounded = %d cm\r\n", distance, rounded);
            if (ENABLE_BT_OBJ == 1) {
                char buffer_o[25];
                printf("Sending signal...\r\n");

                int len = snprintf(buffer_o, sizeof(buffer_o), "%d\n", rounded);
                HAL_UART_Transmit(&huart3, (uint8_t *)buffer_o, len, HAL_MAX_DELAY);
            }
        } else {
            printf("\n");
        }
    }

    if (TOGGLE_LED == 1) {
        HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);
        HAL_Delay(500);
    }

    if (ENABLE_MOTOR_SIGNAL == 1) {
        printf("Running tim3 and tim4 pwm gen.\r\n");
        motor1(0, 1.0, 0);
        motor2(0, 1.0, 0);
        HAL_Delay(100);
        stopMotor1();
        stopMotor2();
    }

    if (ENABLE_BT_MOTOR == 1) {
        if (line_ready)
        {
            char local[24];
            int l, r, q;

            __disable_irq();
            strncpy(local, rx_line, sizeof(local));
            local[sizeof(local) - 1] = '\0';
            line_ready = 0;
            __enable_irq();

            if (sscanf(local, "L:%d,R:%d,HL:%d\n", &l, &r, &q) == 3)
            {
                bt_motorL = (int16_t)l;
                bt_motorR = (int16_t)r;
                bt_light = (int16_t)q;

                if (bt_light == 0) {
                    printf("Light: on\r\n");
                    HAL_GPIO_WritePin(LIGHT_PORT, LIGHT_PIN, GPIO_PIN_SET);
                } else if (bt_light == 1) {
                    printf("Light: off\r\n");
                    HAL_GPIO_WritePin(LIGHT_PORT, LIGHT_PIN, GPIO_PIN_RESET);
                }

                printf("Parsed -> L:%d R:%d\r\n", bt_motorL, bt_motorR);
                printf("%f, %f\r\n", bt_motorL/100.0, bt_motorR/100.0);

                if (bt_motorL > 0) {
                    motor1(0, bt_motorL/100.0, 0);
                } else if (bt_motorL < 0) {
                    motor1(0, bt_motorL/100.0, 1);
                } else {
                    stopMotor1();
                }

                if (bt_motorR > 0) {
                    motor2(0, bt_motorR/100.0, 0);
                } else if (bt_motorR < 0) {
                    motor2(0, bt_motorR/100.0, 1);
                } else {
                    stopMotor2();
                }
            }
            else
            {
                printf("Bad packet: %s\r\n", local);
            }
        }
        HAL_Delay(100);
    }
}
/* USER CODE END WHILE */


