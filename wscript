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
    ctx.load('compiler_cxx cfitsio CCfits boost armadillo hires tinyhtm_cxx wcs')
    ctx.add_option('--debug', help='Include debug symbols and turn ' +
                                   'compiler optimizations off',
                   action='store_true', default=False, dest='debug')

def configure(ctx):
    ctx.env.append_value('CXXFLAGS', '-Wall')
    ctx.env.append_value('CXXFLAGS', '-Wextra')
    ctx.env.append_value('CXXFLAGS', '-std=c++11')
    ctx.env.append_value('CXXFLAGS', '-D__STDC_CONSTANT_MACROS')
    ctx.load('compiler_cxx cfitsio CCfits boost armadillo hires tinyhtm_cxx wcs')

    if ctx.options.debug:
        ctx.env.append_value('CXXFLAGS', '-g')
    else:
        ctx.env.append_value('CXXFLAGS', '-Ofast')
        ctx.env.append_value('CXXFLAGS', '-mtune=native')
        ctx.env.append_value('CXXFLAGS', '-march=native')
        ctx.env.append_value('CXXFLAGS', '-DNDEBUG')

def build(ctx):
    ctx.program(
        source=['src/compare/compare.cxx',
                'src/compare/read_raw_hdf5.cxx',
                'src/compare/read_indexed_hdf5.cxx',
                'src/lzf/lzf_filter.c',
                'src/lzf/lzf/lzf_c.c', 'src/lzf/lzf/lzf_d.c'],
        target='compare',
        name='compare',
        install_path=os.path.join(ctx.env.PREFIX, 'bin'),
        use='hdf5_cxx boost'
    )

    ctx.program(
        source=['src/planck_to_htm.cxx', 'src/lzf/lzf_filter.c',
                'src/lzf/lzf/lzf_c.c', 'src/lzf/lzf/lzf_d.c'],
        target='planck_to_htm',
        name='planck_to_htm',
        install_path=os.path.join(ctx.env.PREFIX, 'bin'),
        use='tinyhtm_cxx cfitsio CCfits hdf5_cxx boost wcs'
    )

    ctx.program(
        source=[
            'src/planck_toi_search/main.cxx',
            'src/planck_toi_search/Results.cxx'],
        target='planck_toi_search',
        name='planck_toi_search',
        includes=['..'],
        install_path=os.path.join(ctx.env.PREFIX, 'bin'),
        use=['cfitsio','CCfits','hdf5','hdf5_cxx','tinyhtm','tinyhtm_cxx','wcs']
    )

    ctx.program(
        source=[
            'src/planck_toi_hires/main.cxx',
            'src/planck_toi_hires/fill_samples.cxx'],
        target='planck_toi_hires',
        name='planck_toi_hires',
        install_path=os.path.join(ctx.env.PREFIX, 'bin'),
        use=['ccfits','boost','hdf5','hdf5_cxx','armadillo','hires','tinyhtm','tinyhtm_cxx','wcs']
    )
