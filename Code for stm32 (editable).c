/* USER CODE BEGIN PV */
uint8_t metalLatched = 0;
uint8_t pendingDance = 0;
uint8_t dancePlayed = 0;
uint32_t danceAtMs = 0;


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

void playMetalBeat(void)
{
    static const note_t notes[] = {
        1760, 2093, 2349, 2093, 0,
        1760, 2093, 2620, 2349, 0,
        2093, 1760, 2349, 2620, 2093
    };

    static const uint16_t durations[] = {
        75,  75,  110, 75,  40,
        80,  100, 140, 100, 45,
        90,  90,  110, 150, 220
    };

    playSong(notes, durations, 15, 14);
}



/* USER CODE BEGIN 2 */
HAL_UART_Receive_IT(&huart3, &rx_byte, 1);
stopTone();
playStartupTune();

/YOUR METAL DETECTION BLOCK IN while(1)
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
        playMetalBeat();
        dancePlayed = 1;
        pendingDance = 0;
    }
}

if (!metalNow && metalLatched) {
    pendingDance = 0;
    dancePlayed = 0;
    stopTone();
}

metalLatched = metalNow;
\



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
                    playMetalBeat();
                    dancePlayed = 1;
                    pendingDance = 0;
                }
            }

            if (!metalNow && metalLatched) {
                pendingDance = 0;
                dancePlayed = 0;
                stopTone();
            }

            metalLatched = metalNow;
        }
    }

    HAL_Delay(100);
