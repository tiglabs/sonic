#!/usr/bin/env python

from setuptools import setup

setup(
   name='arista',
   version='1.0',
   description='Module to initialize arista platforms',
   packages=[
      'arista',
      'arista.core',
      'arista.components',
      'arista.platforms',
      'arista.utils',
   ],
)

