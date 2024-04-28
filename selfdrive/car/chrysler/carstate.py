from cereal import car
from openpilot.common.conversions import Conversions as CV
from opendbc.can.parser import CANParser
from opendbc.can.can_define import CANDefine
from openpilot.selfdrive.car.interfaces import CarStateBase
from openpilot.selfdrive.car.chrysler.values import DBC, STEER_THRESHOLD, RAM_CARS, CUSW_CARS

import numpy as np

ButtonType = car.CarState.ButtonEvent.Type

CHECK_BUTTONS = {ButtonType.cancel: ["CRUISE_BUTTONS", 'ACC_Cancel'],
                 ButtonType.resumeCruise: ["CRUISE_BUTTONS", 'ACC_Resume'],
                 ButtonType.accelCruise: ["CRUISE_BUTTONS", 'ACC_Accel'],
                 ButtonType.decelCruise: ["CRUISE_BUTTONS", 'ACC_Decel'],
                 ButtonType.followInc: ["CRUISE_BUTTONS", 'ACC_Distance_Inc'],
                 ButtonType.followDec: ["CRUISE_BUTTONS", 'ACC_Distance_Dec'],
                 ButtonType.lkasToggle: ["TRACTION_BUTTON", 'TOGGLE_LKAS']}

PEDAL_GAS_PRESSED_XP = [0, 32, 255]
PEDAL_BRAKE_PRESSED_XP = [0, 24, 255]
PEDAL_PRESSED_YP = [0, 128, 255]

class CarState(CarStateBase):
  def __init__(self, CP):
    super().__init__(CP)
    self.CP = CP
    can_define = CANDefine(DBC[CP.carFingerprint]["pt"])

    self.auto_high_beam = 0
    self.button_counter = 0
    self.lkas_car_model = -1

    if CP.carFingerprint in RAM_CARS:
      self.shifter_values = can_define.dv["Transmission_Status"]["Gear_State"]
    else:
      self.shifter_values = can_define.dv["GEAR"]["PRNDL"]

    self.lkasHeartbit = None

  def update(self, cp, cp_cam):

    if self.CP.carFingerprint in CUSW_CARS:
      return self.update_cusw(cp, cp_cam) 
      
    ret = car.CarState.new_message()

    # lock info
    ret.doorOpen = any([cp.vl["BCM_1"]["DOOR_OPEN_FL"],
                        cp.vl["BCM_1"]["DOOR_OPEN_FR"],
                        cp.vl["BCM_1"]["DOOR_OPEN_RL"],
                        cp.vl["BCM_1"]["DOOR_OPEN_RR"]])
    ret.seatbeltUnlatched = cp.vl["ORC_1"]["SEATBELT_DRIVER_UNLATCHED"] == 1

    # brake pedal
    ret.brake = 0
    ret.brakePressed = cp.vl["ESP_1"]["Brake_Pedal_State"] == 1  # Physical brake pedal switch

    # gas pedal
    ret.gas = cp.vl["ECM_5"]["Accelerator_Position"]
    ret.gasPressed = ret.gas > 1e-5

    # car speed
    if self.CP.carFingerprint in RAM_CARS:
      ret.gearShifter = self.parse_gear_shifter(self.shifter_values.get(cp.vl["Transmission_Status"]["Gear_State"], None))
    else:
      ret.gearShifter = self.parse_gear_shifter(self.shifter_values.get(cp.vl["GEAR"]["PRNDL"], None))
    ret.vEgoRaw = cp.vl["ESP_8"]["Vehicle_Speed"] * CV.KPH_TO_MS
    ret.vEgo, ret.aEgo = self.update_speed_kf(ret.vEgoRaw)
    ret.standstill = not ret.vEgoRaw > 0.001
    ret.wheelSpeeds = self.get_wheel_speeds(
      cp.vl["ESP_6"]["WHEEL_SPEED_FL"],
      cp.vl["ESP_6"]["WHEEL_SPEED_FR"],
      cp.vl["ESP_6"]["WHEEL_SPEED_RL"],
      cp.vl["ESP_6"]["WHEEL_SPEED_RR"],
      unit=1,
    )

    # button presses
    ret.leftBlinker, ret.rightBlinker = self.update_blinker_from_stalk(200, cp.vl["STEERING_LEVERS"]["TURN_SIGNALS"] == 1,
                                                                       cp.vl["STEERING_LEVERS"]["TURN_SIGNALS"] == 2)
    ret.genericToggle = cp.vl["STEERING_LEVERS"]["HIGH_BEAM_PRESSED"] == 1

    # steering wheel
    ret.steeringAngleDeg = cp.vl["STEERING"]["STEERING_ANGLE"] + cp.vl["STEERING"]["STEERING_ANGLE_HP"]
    ret.steeringRateDeg = cp.vl["STEERING"]["STEERING_RATE"]
    ret.steeringTorque = cp.vl["EPS_2"]["COLUMN_TORQUE"]
    ret.steeringTorqueEps = cp.vl["EPS_2"]["EPS_TORQUE_MOTOR"]
    ret.steeringPressed = abs(ret.steeringTorque) > STEER_THRESHOLD

    # cruise state
    cp_cruise = cp_cam if self.CP.carFingerprint in RAM_CARS else cp

    ret.cruiseState.available = cp_cruise.vl["DAS_3"]["ACC_AVAILABLE"] == 1
    ret.cruiseState.enabled = cp_cruise.vl["DAS_3"]["ACC_ACTIVE"] == 1
    ret.cruiseState.speed = cp_cruise.vl["DAS_4"]["ACC_SET_SPEED_KPH"] * CV.KPH_TO_MS
    ret.cruiseState.nonAdaptive = cp_cruise.vl["DAS_4"]["ACC_STATE"] in (1, 2)  # 1 NormalCCOn and 2 NormalCCSet
    ret.cruiseState.standstill = cp_cruise.vl["DAS_3"]["ACC_STANDSTILL"] == 1
    ret.accFaulted = cp_cruise.vl["DAS_3"]["ACC_FAULTED"] != 0
    self.lkasHeartbit = cp_cam.vl["LKAS_HEARTBIT"]

    if self.CP.carFingerprint in RAM_CARS:
      # Auto High Beam isn't Located in this message on chrysler or jeep currently located in 729 message
      self.auto_high_beam = cp_cam.vl["DAS_6"]['AUTO_HIGH_BEAM_ON']
      ret.steerFaultTemporary = cp.vl["EPS_3"]["DASM_FAULT"] == 1
    else:
      ret.steerFaultTemporary = cp.vl["EPS_2"]["LKAS_TEMPORARY_FAULT"] == 1
      ret.steerFaultPermanent = cp.vl["EPS_2"]["LKAS_STATE"] == 4

    # blindspot sensors
    if self.CP.enableBsm:
      ret.leftBlindspot = cp.vl["BSM_1"]["LEFT_STATUS"] == 1
      ret.rightBlindspot = cp.vl["BSM_1"]["RIGHT_STATUS"] == 1

    self.lkas_car_model = cp_cam.vl["DAS_6"]["CAR_MODEL"]
    self.button_counter = cp.vl["CRUISE_BUTTONS"]["COUNTER"]

    brake = cp.vl["ESP_8"]["BRK_PRESSURE"]
    gas = cp.vl["ECM_2"]["ACCEL"]
    if gas > 0:
      ret.jvePilotCarState.pedalPressedAmount = float(np.interp(gas, PEDAL_GAS_PRESSED_XP, PEDAL_PRESSED_YP)) / 256
    elif brake > 0:
      ret.jvePilotCarState.pedalPressedAmount = float(np.interp(brake / 16, PEDAL_BRAKE_PRESSED_XP, PEDAL_PRESSED_YP)) / -256
    else:
      ret.jvePilotCarState.pedalPressedAmount = 0

    ret.jvePilotCarState.accFollowDistance = int(min(3, max(0, cp.vl["DAS_4"]['ACC_DISTANCE_CONFIG_2'])))

    button_events = []
    for buttonType in CHECK_BUTTONS:
      self.check_button(button_events, buttonType, bool(cp.vl[CHECK_BUTTONS[buttonType][0]][CHECK_BUTTONS[buttonType][1]]))
    ret.buttonEvents = button_events

    return ret

  def update_cusw(self, cp, cp_cam):
    ret = car.CarState.new_message()

    # JVE Pilot 
    self.lkasHeartbit = cp_cam.vl["LKAS_HEARTBIT"]
    
    ret.doorOpen = any([cp.vl["DOORS"]["DOOR_OPEN_FL"],
                        cp.vl["DOORS"]["DOOR_OPEN_FR"],
                        cp.vl["DOORS"]["DOOR_OPEN_RL"],
                        cp.vl["DOORS"]["DOOR_OPEN_RR"]])
    ret.seatbeltUnlatched = bool(cp.vl["SEATBELT_STATUS"]["SEATBELT_DRIVER_UNLATCHED"])

    ret.brakePressed = bool(cp.vl["BRAKE_2"]["BRAKE_HUMAN"])
    ret.brake = cp.vl["BRAKE_1"]["BRAKE_PSI"]
    ret.gas = cp.vl["ACCEL_GAS"]["GAS_HUMAN"]
    ret.gasPressed = ret.gas > 0

    ret.espDisabled = bool(cp.vl["TRACTION_BUTTON"]["TRACTION_OFF"])

    ret.vEgoRaw = cp.vl["BRAKE_1"]["VEHICLE_SPEED"]
    ret.vEgo, ret.aEgo = self.update_speed_kf(ret.vEgoRaw)
    ret.standstill = not ret.vEgoRaw > 0.001
    ret.wheelSpeeds = self.get_wheel_speeds(
      cp.vl["WHEEL_SPEEDS_FRONT"]["WHEEL_SPEED_FL"],
      cp.vl["WHEEL_SPEEDS_REAR"]["WHEEL_SPEED_RR"],
      cp.vl["WHEEL_SPEEDS_REAR"]["WHEEL_SPEED_RL"],
      cp.vl["WHEEL_SPEEDS_FRONT"]["WHEEL_SPEED_FR"],
      unit=1,
    )

    ret.leftBlinker = cp.vl["STEERING_LEVERS"]["TURN_SIGNALS"] == 1
    ret.rightBlinker = cp.vl["STEERING_LEVERS"]["TURN_SIGNALS"] == 2
    ret.steeringAngleDeg = cp.vl["STEERING"]["STEER_ANGLE"]
    ret.steeringRateDeg = cp.vl["STEERING"]["STEERING_RATE"]
    ret.gearShifter = self.parse_gear_shifter(self.shifter_values.get(cp.vl["GEAR"]["PRNDL"], None))

    ret.cruiseState.speed = cp.vl["ACC_HUD"]["ACC_SET_SPEED_KMH"] * CV.KPH_TO_MS
    ret.cruiseState.available = bool(cp.vl["ACC_CONTROL"]["ACC_MAIN_ON"])
    ret.cruiseState.enabled = bool(cp.vl["ACC_CONTROL"]["ACC_ACTIVE"])

    ret.steeringTorque = cp.vl["EPS_STATUS"]["TORQUE_DRIVER"]
    ret.steeringTorqueEps = cp.vl["EPS_STATUS"]["TORQUE_MOTOR"]
    ret.steeringPressed = abs(ret.steeringTorque) > STEER_THRESHOLD
    ret.steerFaultPermanent = bool(cp.vl["EPS_STATUS"]["LKAS_FAULT"])

    return ret  
  
  def check_button(self, button_events, button_type, pressed):
    pressed_frames = 0
    pressed_changed = False
    for ob in self.out.buttonEvents:
      if ob.type == button_type:
        pressed_frames = ob.pressedFrames
        pressed_changed = ob.pressed != pressed
        break

    if pressed or pressed_changed:
      be = car.CarState.ButtonEvent.new_message()
      be.type = button_type
      be.pressed = pressed
      be.pressedFrames = pressed_frames

      if not pressed_changed:
        be.pressedFrames += 1

      button_events.append(be)

  def button_pressed(self, button_type, pressed=True):
    for b in self.out.buttonEvents:
      if b.type == button_type:
        if b.pressed == pressed:
          return b
        break

  @staticmethod
  def get_cruise_messages():
    messages = [
      ("DAS_3", 50),
      ("DAS_4", 50),
    ]
    return messages

  @staticmethod
  def get_can_parser(CP):
    
    if CP.carFingerprint in CUSW_CARS:
      return CarState.get_can_parser_cusw(CP)

    messages = [
      # sig_address, frequency
      ("ESP_1", 50),
      ("EPS_2", 100),
      ("ESP_6", 50),
      ("STEERING", 100),
      ("ECM_5", 50),
      ("CRUISE_BUTTONS", 50),
      ("STEERING_LEVERS", 10),
      ("ORC_1", 2),
      ("BCM_1", 1),
      ("ESP_8", 50),
      ("LKAS_HEARTBIT", 50), #JVE Pilot
      ("ECM_2", 50),
      ("TRACTION_BUTTON", 1),
    ]

    if CP.enableBsm:
      messages.append(("BSM_1", 2))

    if CP.carFingerprint in RAM_CARS:
      messages += [
        ("EPS_3", 50),
        ("Transmission_Status", 50),
      ]
    else:
      messages += [
        ("GEAR", 50),
        ("SPEED_1", 100),
      ]
      messages += CarState.get_cruise_messages()

    return CANParser(DBC[CP.carFingerprint]["pt"], messages, 0)

  @staticmethod
  def get_can_parser_cusw(CP):
    messages = [
      # sig_address, frequency
      ("ACC_CONTROL", 50),
      ("ACC_HUD", 17),
      ("ACCEL_GAS", 50),
      ("BRAKE_1", 50),
      ("BRAKE_2", 50),
      ("DOORS", 10),
      ("EPS_STATUS", 100),
      ("GEAR", 10),
      ("SEATBELT_STATUS", 10),
      ("STEERING", 100),
      ("STEERING_LEVERS", 10),
      ("TRACTION_BUTTON", 50),
      ("LKAS_HEARTBIT", 50),  #JVEPilot
      ("WHEEL_SPEEDS_FRONT", 50),
      ("WHEEL_SPEEDS_REAR", 50),
    ]

    return CANParser(DBC[CP.carFingerprint]["pt"], messages, 0)
  
  @staticmethod
  def get_cam_can_parser(CP):
    
    if CP.carFingerprint in CUSW_CARS:
      return CarState.get_cam_can_parser_cusw(CP)

    messages = [
      ("DAS_6", 4),
    ]

    if CP.carFingerprint in RAM_CARS:
      messages += CarState.get_cruise_messages()
    else:
      # LKAS_HEARTBIT data needs to be forwarded!
      forward_lkas_heartbit_messages = [
        ("LKAS_HEARTBIT", 10),
      ]
      messages += forward_lkas_heartbit_messages

    return CANParser(DBC[CP.carFingerprint]["pt"], messages, 2)

  @staticmethod
  def get_cam_can_parser_cusw(CP):
    messages = [
      ("DAS_6", 4),
    ]

    return CANParser(DBC[CP.carFingerprint]["pt"], messages, 2)
