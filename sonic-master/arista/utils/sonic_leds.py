import os
import subprocess
import re
from collections import namedtuple, defaultdict

from ..core import platform
import arista.platforms

try:
   from sonic_led import led_control_base
except ImportError, e:
   raise ImportError (str(e) + " - required module not found")

Port = namedtuple('Port', ['minLane', 'maxLane', 'portNum'])

def parsePortConfig(portConfigPath):
   portMapping = {}
   portLanes = defaultdict(list)
   minLanes = {}

   with open(portConfigPath) as fp:
      for line in fp:
         line = line.strip()
         if not line or line[0] == '#':
            continue

         fields = line.split()
         name = fields[0]
         lanes = map(int, fields[1].split(','))
         try:
            portNum = fields[3]
         except IndexError:
            alias = fields[2]
            portNum = re.findall(r'\d+', alias)[0]

         portNum = int(portNum)
         portMapping[name] = Port(min(lanes), max(lanes), portNum)
         portLanes[portNum] += lanes

   for port, lanes in portLanes.items():
      minLanes[port] = min(lanes)

   return portMapping, minLanes

class LedControl(led_control_base.LedControlBase):
   PORT_CONFIG_PATH = "/usr/share/sonic/hwsku/port_config.ini"
   LED_SYSFS_PATH = "/sys/class/leds/{0}/brightness"

   LED_COLOR_OFF = 0
   LED_COLOR_GREEN = 1
   LED_COLOR_YELLOW = 2

   def __init__(self):
      self.portMapping, self.minLanes = parsePortConfig(self.PORT_CONFIG_PATH)
      self.portSysfsMapping = defaultdict(list)

      inventory = platform.getPlatform().getInventory()
      for port, names in inventory.xcvrLeds.items():
         for name in names:
            self.portSysfsMapping[port].append(
                     self.LED_SYSFS_PATH.format(name))

      # set status leds to green initially (Rook led driver does this
      # automatically)
      for statusLed in inventory.statusLeds:
         with open(self.LED_SYSFS_PATH.format(statusLed), "w") as fp:
            fp.write("%d" % self.LED_COLOR_GREEN)

   def port_link_state_change(self, port, state):
      portNum = self.portMapping[port].portNum
      minLane = self.minLanes[portNum] # min lane for that port
      minLed = self.portMapping[port].minLane - minLane
      maxLed = self.portMapping[port].maxLane - minLane + 1

      for path in self.portSysfsMapping[portNum][minLed:maxLed]:
         with open(path, "w") as fp:
            if state == "up":
               fp.write("%d" % self.LED_COLOR_GREEN)
            elif state == "down":
               fp.write("%d" % self.LED_COLOR_OFF)

def getLedControl():
   return LedControl
