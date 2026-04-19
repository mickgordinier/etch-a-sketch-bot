// Stepper motors have 2 independent coils. Need to identify pairs
// Use of continuity test using Voltmeter.  Low resistance --> Beep --> Pair.
// Gray-Green (2A,B) / Red-Yellow (1A,B) Pairing for Horizontal (Left) NEMA-17 stepper motor
// Gray-Green (2A,B) / Red-Yellow (1A,B) Pairing for Vertical  (Right) NEMA-17 stepper motor

// For A4988 reference voltage calculations
// NEMA-17 Stepper Motors are 350 mA, 
// A4988 Stepper Motor Driver contains R10 = 0.1 Ohms sense resistors
// Reference Voltage V_ref = I_max * (8 * R_sense) = (.35 A) * (8 * (0.1 Ohms)) = 0.28 V
// 85% is safe target --> Should aim for 0.25 V

// Etch-A-Sketch Dimensions
// Horizontal Rotations = 4.25 rotations
// Vertical Rotations = 3 rotations
// --> w/ 200 step NEMA-17 --> (850 horizontal, 600 vertical)

// Setting Arduino Pins
const int HORIZONTAL_DIR_PIN  = 2;
const int HORIZONTAL_STEP_PIN = 3;
const int VERTICAL_DIR_PIN    = 4;
const int VERTICAL_STEP_PIN   = 5;

// Defining motor turns
const int LEFT_DIR  = HIGH;
const int RIGHT_DIR = LOW;
const int UP_DIR    = LOW;
const int DOWN_DIR  = HIGH;

// Seperate Backslash for horizontal/vertical pulleys
const int HORIZ_BACKSLASH = 10;
const int VERT_BACKSLASH  = 7;

// Variables to track last direction (initialized to -1 for "none")
static int last_h_dir = -1;
static int last_v_dir = -1;

const char* const DIRECTIONS[] = {
    "LEFT",
    "RIGHT",
    "DOWN",
    "UP"
};

void move_steps(int step_pin, int steps) {
  for (int i = 0; i < steps; ++i) {
    digitalWrite(step_pin, HIGH);
    delayMicroseconds(20);
    digitalWrite(step_pin, LOW);
    delayMicroseconds(2000);
  }
}

void move_left(uint8_t steps) {
  int total = steps;
  if (last_h_dir == RIGHT_DIR) total += HORIZ_BACKSLASH;
  digitalWrite(HORIZONTAL_DIR_PIN, LEFT_DIR);
  last_h_dir = LEFT_DIR;
  move_steps(HORIZONTAL_STEP_PIN, total);
}

void move_right(uint8_t steps) {
  int total = steps;
  if (last_h_dir == LEFT_DIR) total += HORIZ_BACKSLASH;
  digitalWrite(HORIZONTAL_DIR_PIN, RIGHT_DIR);
  last_h_dir = RIGHT_DIR;
  move_steps(HORIZONTAL_STEP_PIN, total);
}

void move_up(uint8_t steps) {
  int total = steps;
  if (last_v_dir == DOWN_DIR) total += VERT_BACKSLASH;
  digitalWrite(VERTICAL_DIR_PIN, UP_DIR);
  last_v_dir = UP_DIR;
  move_steps(VERTICAL_STEP_PIN, total);
}

void move_down(uint8_t steps) {
  int total = steps;
  if (last_v_dir == UP_DIR) total += VERT_BACKSLASH;
  digitalWrite(VERTICAL_DIR_PIN, DOWN_DIR);
  last_v_dir = DOWN_DIR;
  move_steps(VERTICAL_STEP_PIN, total);
}

void draw_test_box() {
  move_right(100);
  move_down(100);
  move_left(100);
  move_up(100);
}

void draw_full_border() {
  move_right(850);
  move_down(600);
  move_left(850);
  move_up(600);
}

void setup() {
  Serial.begin(115200); 
  pinMode(HORIZONTAL_DIR_PIN,  OUTPUT);
  pinMode(HORIZONTAL_STEP_PIN, OUTPUT);
  pinMode(VERTICAL_DIR_PIN,    OUTPUT);
  pinMode(VERTICAL_STEP_PIN,   OUTPUT);
}

void loop() {
  if (Serial.available() > 0) {
    const uint8_t key = Serial.read();

    const uint8_t direction = (key >> 6) & 0x3;
    const uint8_t steps = key & 0x3F;

    Serial.print("Direction: ");
    Serial.print(DIRECTIONS[direction]);
    Serial.print(", Step Size: ");
    Serial.println(steps);

    if      (direction == 0) move_left(steps);
    else if (direction == 1) move_right(steps);
    else if (direction == 2) move_down(steps);
    else if (direction == 3) move_up(steps);
  }
}