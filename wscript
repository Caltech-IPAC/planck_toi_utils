#! /usr/bin/env python
# encoding: utf-8
#
# Copyright (C) 2011, 2012 California Institute of Technology
# Authors:
#  Serge Monkewitz (IPAC)
#  Walter Landry (IPAC)

from __future__ import with_statement
import os
import sys
import traceback

from waflib import Build, Logs, Utils

def options(ctx):
    ctx.load('compiler_cxx CCfits boost hires tinyhtm_cxx')
    ctx.add_option('--debug', help='Include debug symbols and turn ' +
                                   'compiler optimizations off',
                   action='store_true', default=False, dest='debug')

def configure(ctx):
    ctx.env.append_value('CXXFLAGS', '-Wall')
    ctx.env.append_value('CXXFLAGS', '-Wextra')
    ctx.env.append_value('CXXFLAGS', '-std=c++11')
    ctx.env.append_value('CXXFLAGS', '-D__STDC_CONSTANT_MACROS')
    ctx.load('compiler_cxx CCfits boost hires tinyhtm_cxx')

    if ctx.options.debug:
        ctx.env.append_value('CXXFLAGS', '-g')
    else:
        ctx.env.append_value('CXXFLAGS', '-Ofast')
        ctx.env.append_value('CXXFLAGS', '-mtune=native')
        ctx.env.append_value('CXXFLAGS', '-march=native')
        ctx.env.append_value('CXXFLAGS', '-DNDEBUG')

def build(ctx):
    ctx.program(
        source=[
            'src/planck_toi_time_ranges/main.cxx',
            'src/planck_toi_time_ranges/Ranges.cxx'],
        target='planck_toi_time_ranges',
        name='planck_toi_time_ranges',
        includes=['..'],
        install_path=os.path.join(ctx.env.PREFIX, 'bin'),
        use=['hdf5','hdf5_cxx','tinyhtm','tinyhtm_cxx']
    )

    ctx.program(
        source=[
            'src/planck_toi_search/main.cxx',
            'src/planck_toi_search/Results.cxx'],
        target='planck_toi_search',
        name='planck_toi_search',
        includes=['..'],
        install_path=os.path.join(ctx.env.PREFIX, 'bin'),
        use=['hdf5','hdf5_cxx','tinyhtm','tinyhtm_cxx']
    )

    ctx.program(
        source=[
            'src/planck_toi_hires/main.cxx',
            'src/planck_toi_hires/fill_samples.cxx'],
        target='planck_toi_hires',
        name='planck_toi_hires',
        install_path=os.path.join(ctx.env.PREFIX, 'bin'),
        use=['ccfits','boost','hdf5','hdf5_cxx','hires','tinyhtm','tinyhtm_cxx']
    )
