  // 모터 제어 핀 (MD10C 방식)
const int MOTOR_DIR_PIN = 12;    // 방향 제어
const int MOTOR_PWM_PIN = 11;     // PWM 출력

// 인터럽트 핀 (A상, B상 엔코더용)
const int encoderPinA = 2;       // 인터럽트 가능
const int encoderPinB = 7;       // 일반 핀

// 엔코더 관련 변수
volatile long encoderCount = 0;
int lastEncoded = 0;

// 아날로그 각도 센서 변수
float ADCvalue = 0;
float currentAngle = 0;
const float ADCmin = 104.0;
const float ADCmax = 919.0;
const float ANGLE_OFFSET = 18.0 + 7.00 -0.77 -1.04 - 2.5 + 0.3;  // -가 뜬 만큼 더해주기

// PID 제어 변수
double targetAngle = 0.0;
double angleError = 0.0;
double controlSignal = 0.0;
double previousError = 0.0;
double integralTerm = 0.0;
double Kp = 150.0, Ki = 40.0, Kd = 80.0; // 400에 45

// 타이머
unsigned long lastControlTime = 0;
const unsigned long controlInterval = 50; // ms

// 실행 상태 제어 변수
bool isRunning = true;


// ADC 평균 필터 함수
float readFilteredADC(int pin, int sampleCount = 100) {
  long total = 0;
  for (int i = 0; i < sampleCount; i++) {
    total += analogRead(pin);
    delayMicroseconds(5);  // 샘플 간 약간의 간격
  }
  return total / (float)sampleCount;
}


void setup() {
  Serial.begin(115200);

  // 모터 핀 설정
  pinMode(MOTOR_DIR_PIN, OUTPUT);
  pinMode(MOTOR_PWM_PIN, OUTPUT);

  // 엔코더 핀 설정
  pinMode(encoderPinA, INPUT_PULLUP);
  pinMode(encoderPinB, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(encoderPinA), updateEncoder, CHANGE);

  Serial.println("Press 'r' to start/stop the control loop.");
}

void loop() {
  // 시리얼 입력 처리
  if (Serial.available() > 0) {
    char input = Serial.read();
    if (input == 'r' || input == 'R') {
      isRunning = !isRunning;
      if (!isRunning) {
        stopMotor();  // 제어 정지 시 모터도 정지
        Serial.println("System STOPPED.");
      } else {
        Serial.println("System RUNNING.");
      }
    }
  }

  // 여기서 50ms 맞추고 있음
  // 제어 루프 실행
  if (isRunning && millis() - lastControlTime >= controlInterval) {
    lastControlTime += controlInterval;

    // 1. ADC → 각도 (0~360) [필터 적용]
    ADCvalue = readFilteredADC(A0, 100);  // 10회 평균 필터링
    currentAngle = (ADCvalue - ADCmin) * 360.0 / (ADCmax - ADCmin);
    currentAngle = constrain(currentAngle, 0.0, 360.0);

    // 2. 오프셋 보정 (실제 0도에 맞게 수정)
    currentAngle += ANGLE_OFFSET;
    if (currentAngle < 0) currentAngle += 360.0;
    if (currentAngle >= 360.0) currentAngle -= 360.0;

    // 3. -180 ~ +180 범위로 변환
    float relativeAngle = currentAngle;
    if (relativeAngle > 180.0) relativeAngle -= 360.0;

    // 4. 목표 각도 설정
    targetAngle = 0.0;  // 목표 각도를 0도로 설정
    angleError = relativeAngle - targetAngle;

    // 5. PID 제어
    if (abs(angleError) > 30.0 || abs(angleError) < 0.000001) {  // 오차가 너무 크거나 작으면 정지
      stopMotor();
      integralTerm = 0;
      previousError = 0;
    } else {
      integralTerm += angleError;
      integralTerm = constrain(integralTerm, -70, 70);
      double derivativeTerm = (angleError - previousError);
      previousError = angleError;

      controlSignal = (Kp * angleError) + (Ki * integralTerm) + (Kd * derivativeTerm);
      int pwmValue = constrain(abs(controlSignal), 0, 255);

      // ★ 방향 반전된 부분 ★
      moveMotor(pwmValue, controlSignal < 0);
    }

    // 6. 디버깅 출력 (200ms 간격)
    static unsigned long lastDebugTime = 0;
    if (millis() - lastDebugTime >= 200) {
      lastDebugTime = millis();
      Serial.print("ADC: "); Serial.print(ADCvalue, 2);
      Serial.print(" | Angle: "); Serial.print(currentAngle, 4); //숫자가 소수
      Serial.print(" | Error: "); Serial.print(angleError, 4);
      Serial.print(" | PWM: "); Serial.print(abs(controlSignal));
      Serial.print(" | Encoder Count: "); Serial.println(encoderCount);
    }
  }

  delay(10); // 시스템 안정화용 소량 딜레이
}

// 인터럽트 핸들러
void updateEncoder() {
  int MSB = digitalRead(encoderPinA);
  int LSB = digitalRead(encoderPinB);
  int encoded = (MSB << 1) | LSB;
  int sum = (lastEncoded << 2) | encoded;

  if (sum == 0b1101 || sum == 0b0100 || sum == 0b0010 || sum == 0b1011)
    encoderCount++;
  if (sum == 0b1110 || sum == 0b0111 || sum == 0b0001 || sum == 0b1000)
    encoderCount--;

  lastEncoded = encoded;
}

// 모터 제어 함수
void moveMotor(int pwm, bool forward) {
  digitalWrite(MOTOR_DIR_PIN, forward ? HIGH : LOW);
  analogWrite(MOTOR_PWM_PIN, pwm);
}

// 모터 정지 함수
void stopMotor() {
  analogWrite(MOTOR_PWM_PIN, 0);
}
