Arduino code

const int AUDIO_PIN   = 9;   
const int TRIGGER_PIN = 2;   

bool lastTrigger = HIGH;
bool pendingDance = false;
bool dancePlayed = false;
unsigned long danceAtMs = 0;

void playSong(const int *notes, const int *durations, int count, int gapMs = 18) {
  for (int i = 0; i < count; i++) {
    if (notes[i] > 0) {
      tone(AUDIO_PIN, notes[i]);
    } else {
      noTone(AUDIO_PIN);
    }
    delay(durations[i]);
    noTone(AUDIO_PIN);
    delay(gapMs);
  }
}

// ---------- Startup tune ----------

 void playStartupTune() {
  const int notes[] = {
    1175, 1175, 932, 1047, 1175, 1175,
    1175, 1175, 1245, 1397, 1245, 1175, 1047,
    1047, 1175, 1245, 1397, 1245, 1175, 1047
  };

  const int durations[] = {
    240, 240, 280, 220, 220, 420,
    220, 220, 220, 320, 220, 220, 420,
    260, 220, 220, 320, 220, 220, 500
  };

  playSong(notes, durations, sizeof(notes) / sizeof(notes[0]), 22);
}




// ---------- Metal sounds ----------
void playMetalSurprised() {
  const int notes[] = {
    900, 1450, 2400, 2850
  };

  const int durations[] = {
    45, 55, 85, 160
  };

  playSong(notes, durations, 4, 10);
}

void playMetalDance() {
  const int notes[] = {
    1760, 2093, 2349, 2093, 0,
    1760, 2093, 2620, 2349, 0,
    2093, 1760, 2349, 2620, 2093
  };

  const int durations[] = {
    75,   75,   110,  75,   40,
    80,   100,  140,  100,  45,
    90,   90,   110,  150,  220
  };

  playSong(notes, durations, 15, 14);
}

void setup() {
  pinMode(TRIGGER_PIN, INPUT_PULLUP);   
  noTone(AUDIO_PIN);
  playStartupTune();
}

void loop() {
  bool triggerNow = digitalRead(TRIGGER_PIN);

  // when metal gets detected
  if (triggerNow == LOW && lastTrigger == HIGH) {
    playMetalSurprised();          
    pendingDance = true;
    dancePlayed = false;
    danceAtMs = millis() + 1000;   // waits 1 second can be changed to lower or higher
  }

  // plays a lil tune if on metal for a second
  if (triggerNow == LOW && pendingDance && !dancePlayed) {
    if (millis() >= danceAtMs) {
      playMetalDance();
      dancePlayed = true;
      pendingDance = false;
    }
  }

  // Resets
  if (triggerNow == HIGH && lastTrigger == LOW) {
    pendingDance = false;
    dancePlayed = false;
  }

  lastTrigger = triggerNow;
}