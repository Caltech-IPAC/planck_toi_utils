#! /usr/bin/env python
# encoding: utf-8
#
# Copyright (C) 2011-2015 California Institute of Technology
# Authors:
#  Serge Monkewitz (IPAC)
#  Walter Landry (IPAC)

import os
import sys
import traceback

from waflib import Build, Logs, Utils

def options(ctx):
    ctx.load('compiler_cxx cxx11 cfitsio CCfits boost eigen tinyhtm_cxx hires wcs json5_parser tablator')
    ctx.add_option('--debug', help='Include debug symbols and turn ' +
                                   'compiler optimizations off',
                   action='store_true', default=False, dest='debug')

def configure(ctx):
    ctx.env.append_value('CXXFLAGS', '-Wall')
    ctx.env.append_value('CXXFLAGS', '-Wextra')
    ctx.env.append_value('CXXFLAGS', '-D__STDC_CONSTANT_MACROS')
    ctx.load('compiler_cxx cxx11 cfitsio CCfits boost eigen wcs tinyhtm_cxx hires json5_parser tablator')
    ctx.check_boost(lib='filesystem system regex')
        
    if ctx.options.debug:
        ctx.env.append_value('CXXFLAGS', '-g')
    else:
        ctx.env.append_value('CXXFLAGS', '-Ofast')
        ctx.env.append_value('CXXFLAGS', '-DNDEBUG')

def build(ctx):
    ctx.program(
        source=['src/compare/compare.cxx',
                'src/compare/read_raw_hdf5.cxx',
                'src/compare/read_indexed_hdf5.cxx',
                'src/lzf/lzf_filter.cxx',
                'src/lzf/lzf/lzf_c.cxx', 'src/lzf/lzf/lzf_d.cxx'],
        target='compare',
        name='compare',
        install_path=os.path.join(ctx.env.PREFIX, 'bin'),
        use=['cxx11','hdf5_cxx','BOOST']
    )

    if ctx.env.LIB_wcs:
        ctx.program(
            source=['src/planck_to_htm.cxx', 'src/lzf/lzf_filter.cxx',
                    'src/lzf/lzf/lzf_c.cxx', 'src/lzf/lzf/lzf_d.cxx'],
            target='planck_to_htm',
            name='planck_to_htm',
            install_path=os.path.join(ctx.env.PREFIX, 'bin'),
            use=['cxx11','tinyhtm_cxx','cfitsio','CCfits','hdf5_cxx','BOOST','wcs']
        )

    ctx.program(
        source=['src/add_string_attribute.cxx'],
        target='add_string_attribute',
        name='add_string_attribute',
        install_path=os.path.join(ctx.env.PREFIX, 'bin'),
        use=['cxx11','hdf5_cxx','BOOST']
    )

    ctx.program(
        source=['src/remove_attribute.cxx'],
        target='remove_attribute',
        name='remove_attribute',
        install_path=os.path.join(ctx.env.PREFIX, 'bin'),
        use=['cxx11','hdf5_cxx','BOOST']
    )

    ctx.program(
        source=[
            'src/planck_toi_search/main.cxx',
            'src/planck_toi_search/Results.cxx'],
        target='planck_toi_search',
        name='planck_toi_search',
        includes=['..'],
        install_path=os.path.join(ctx.env.PREFIX, 'bin'),
        use=['cxx11','cfitsio','CCfits','hdf5','hdf5_cxx','tinyhtm','tinyhtm_cxx']
    )

    ctx.program(
        source=[
            'src/planck_toi_hires/main.cxx',
            'src/planck_toi_hires/read_input.cxx',
            'src/planck_toi_hires/get_sample_from_query.cxx',
            'src/planck_toi_hires/get_sample_from_table.cxx'],
        target='planck_toi_hires',
        name='planck_toi_hires',
        install_path=os.path.join(ctx.env.PREFIX, 'bin'),
        use=['cxx11','cfitsio','CCfits','BOOST','hdf5','hdf5_cxx','eigen',
             'tinyhtm','tinyhtm_cxx','hires', 'json5_parser', 'tablator']
    )
