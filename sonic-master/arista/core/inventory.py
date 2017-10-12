from collections import defaultdict

class Xcvr(object):

   SFP = 0
   QSFP = 1

   def __init__(self, portNum, xcvrType, bus):
      self.portNum = portNum
      self.xcvrType = xcvrType
      self.bus = bus

   def getPresence(self):
      raise NotImplementedError()

   def getLowPowerMode(self):
      raise NotImplementedError()

   def setLowPowerMode(self, value):
      raise NotImplementedError()

   def reset(self, value):
      raise NotImplementedError()

class Inventory(object):
   def __init__(self):
      self.sfpRange = []
      self.qsfpRange = []
      self.allXcvrsRange = []

      self.portStart = None
      self.portEnd = None

      self.xcvrs = {}

      self.xcvrLeds = defaultdict(list)
      self.statusLeds = []

   def freeze(self):
      # XXX: compute the range and some basic information from the various
      #      collections present in the inventory
      # XXX: try to avoid that actually
      pass

   def addPorts(self, sfps=None, qsfps=None):
      if sfps:
         self.sfpRange = sfps
      if qsfps:
         self.qsfpRange = qsfps

      self.allXcvrsRange = sorted(self.sfpRange + self.qsfpRange)
      self.portStart = self.allXcvrsRange[0]
      self.portEnd = self.allXcvrsRange[-1]

   def addXcvr(self, xcvr):
      self.xcvrs[xcvr.portNum] = xcvr

   def getXcvrs(self):
      return self.xcvrs

   def getXcvr(self, xcvrId):
      return self.xcvrs[xcvrId]

   def getPortToEepromMapping(self):
      eepromPath = '/sys/class/i2c-adapter/i2c-{0}/{0}-0050/eeprom'
      return { xcvrId : eepromPath.format(xcvr.bus)
               for xcvrId, xcvr in self.xcvrs.items() }

   def getPortToI2cAdapterMapping(self):
      return { xcvrId : xcvr.bus for xcvrId, xcvr in self.xcvrs.items() }

   def addXcvrLed(self, xcvrId, name):
      self.xcvrLeds[xcvrId].append(name)

   def addStatusLed(self, name):
      self.statusLeds.append(name)

   def addStatusLeds(self, names):
      self.statusLeds.extend(names)



