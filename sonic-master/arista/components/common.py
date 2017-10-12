
import logging
import os

from ..core.component import Component
from ..core.driver import KernelDriver
from ..core.utils import inSimulation
from ..core.types import PciAddr, I2cAddr

class PciComponent(Component):
   def __init__(self, addr, **kwargs):
      assert isinstance(addr, PciAddr)
      super(PciComponent, self).__init__(addr=addr, **kwargs)

class I2cComponent(Component):
   def __init__(self, addr, **kwargs):
      assert isinstance(addr, I2cAddr)
      super(I2cComponent, self).__init__(addr=addr, **kwargs)

class I2cKernelComponent(I2cComponent):
   def __init__(self, addr, name, **kwargs):
      super(I2cKernelComponent, self).__init__(addr, **kwargs)
      self.addDriver(I2cKernelDriver, name)

class PciKernelDriver(KernelDriver):
   def __init__(self, component, name, args=None):
      assert isinstance(component, PciComponent)
      super(PciKernelDriver, self).__init__(component, name, args)

   def getSysfsPath(self):
      return os.path.join('/sys/bus/pci/devices', str(self.component.addr))

class I2cKernelDriver(KernelDriver):
   def __init__(self, component, name):
      assert isinstance(component, I2cComponent)
      super(I2cKernelDriver, self).__init__(component, None)
      self.name = name

   def getSysfsPath(self):
      return os.path.join('/sys/bus/i2c/devices', str(self.component.addr))

   def getSysfsBusPath(self):
      return '/sys/bus/i2c/devices/i2c-%d' % self.component.addr.bus

   def setup(self):
      addr = self.component.addr
      devicePath = self.getSysfsPath()
      path = os.path.join(self.getSysfsBusPath(), 'new_device')
      logging.debug('creating i2c device %s on bus %d at 0x%02x',
                    self.name, addr.bus, addr.address)
      if inSimulation():
         return
      if os.path.exists(devicePath):
         logging.debug('i2c device %s already exists', devicePath)
      else:
         with open(path, 'w') as f:
            f.write('%s 0x%02x' % (self.name, self.component.addr.address))

   def clean(self):
      # i2c kernel devices are automatically cleaned when the module is removed
      if inSimulation():
         return
      path = os.path.join(self.getSysfsBusPath(), 'delete_device')
      addr = self.component.addr
      if os.path.exists(self.getSysfsPath()):
         logging.debug('removing i2c device %s from bus %d' % (self.name, addr.bus))
         with open(path, 'w') as f:
            f.write('0x%02x' % addr.address)

   def __str__(self):
      return '%s(%s)' % (self.__class__.__name__, self.name)
