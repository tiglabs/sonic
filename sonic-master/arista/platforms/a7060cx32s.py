from ..core.platform import registerPlatform, Platform
from ..core.driver import KernelDriver
from ..core.utils import incrange
from ..core.types import PciAddr, I2cAddr, Gpio, NamedGpio, ResetGpio
from ..core.component import Priority

from ..components.common import I2cKernelComponent
from ..components.scd import Scd

@registerPlatform(['DCS-7060CX-32S', 'DCS-7060CX-32S-ES'])
class Upperlake(Platform):
   def __init__(self):
      super(Upperlake, self).__init__()

      self.sfpRange = incrange(33, 34)
      self.qsfp100gRange = incrange(1, 32)

      self.inventory.addPorts(sfps=self.sfpRange, qsfps=self.qsfp100gRange)

      self.addDriver(KernelDriver, 'crow-fan-driver')

      scd = Scd(PciAddr(bus=0x02), newDriver=True)
      self.addComponent(scd)

      scd.addComponents([
         I2cKernelComponent(I2cAddr(2, 0x1a), 'max6697',
                            priority=Priority.BACKGROUND),
         I2cKernelComponent(I2cAddr(3, 0x4c), 'max6658',
                            priority=Priority.BACKGROUND),
         I2cKernelComponent(I2cAddr(3, 0x60), 'crow_cpld'),
         I2cKernelComponent(I2cAddr(3, 0x4e), 'pmbus',
                            priority=Priority.BACKGROUND), # ucd90120A
         I2cKernelComponent(I2cAddr(5, 0x58), 'pmbus',
                            priority=Priority.BACKGROUND),
         I2cKernelComponent(I2cAddr(6, 0x58), 'pmbus',
                            priority=Priority.BACKGROUND),
         I2cKernelComponent(I2cAddr(7, 0x4e), 'pmbus',
                            priority=Priority.BACKGROUND), # ucd90120A
      ])

      scd.addSmbusMasterRange(0x8000, 5, 0x80)

      scd.addLeds([
         (0x6050, 'status'),
         (0x6060, 'fan_status'),
         (0x6070, 'psu1'),
         (0x6080, 'psu2'),
         (0x6090, 'beacon'),
      ])
      self.inventory.addStatusLeds(['status', 'fan_status', 'psu1', 'psu2'])

      scd.addResets([
         ResetGpio(0x4000, 1, False, 'switch_chip_reset'),
         ResetGpio(0x4000, 2, False, 'switch_chip_pcie_reset'),
      ])

      scd.addGpios([
         NamedGpio(0x5000, 0, True, False, "psu1"),
         NamedGpio(0x5000, 1, True, False, "psu2"),
      ])

      addr = 0x6100
      for xcvrId in self.sfpRange:
         name = "sfp%d" % xcvrId
         scd.addLed(addr, name)
         self.inventory.addXcvrLed(xcvrId, name)
         addr += 0x10

      addr = 0x6140
      for xcvrId in self.qsfp100gRange:
         for laneId in incrange(1, 4):
            name = "qsfp%d_%d" % (xcvrId, laneId)
            scd.addLed(addr, name)
            self.inventory.addXcvrLed(xcvrId, name)
            addr += 0x10

      addr = 0x5010
      bus = 10
      for xcvrId in self.sfpRange:
         xcvr = scd.addSfp(addr, xcvrId, bus)
         self.inventory.addXcvr(xcvr)
         scd.addComponent(I2cKernelComponent(I2cAddr(bus, 0x50), 'sff8436'))
         addr += 0x10
         bus += 1

      addr = 0x5050
      bus = 18
      for xcvrId in self.qsfp100gRange:
         xcvr = scd.addQsfp(addr, xcvrId, bus)
         self.inventory.addXcvr(xcvr)
         scd.addComponent(I2cKernelComponent(I2cAddr(bus, 0x50), 'sff8436'))
         addr += 0x10
         bus += 1

