#! /usr/bin/env python
# encoding: utf-8

def configure(conf):
    def get_param(varname,default):
        return getattr(Options.options,varname,'')or default

    conf.load('hdf5_cxx');
    # Find TINYHTM C
    if conf.options.tinyhtm_dir:
        if not conf.options.tinyhtm_incdir:
            conf.options.tinyhtm_incdir=conf.options.tinyhtm_dir + "/include"
        if not conf.options.tinyhtm_libdir:
            conf.options.tinyhtm_libdir=conf.options.tinyhtm_dir + "/lib"
    frag="#include <tinyhtm.h>\n" + 'int main()\n' \
        + "{struct htm_tree tree;}\n"
    if conf.options.tinyhtm_incdir:
        tinyhtm_inc=[conf.options.tinyhtm_incdir]
    else:
        tinyhtm_inc=[]

    if conf.options.tinyhtm_c_libs:
        tinyhtm_c_libs=conf.options.tinyhtm_c_libs.split()
    else:
        tinyhtm_c_libs=["tinyhtm"]

    conf.check_cxx(msg="Checking for TINYHTM C bindings",
                   fragment=frag,
                   includes=tinyhtm_inc, uselib_store='tinyhtm',
                   libpath=[conf.options.tinyhtm_libdir],
                   rpath=[conf.options.tinyhtm_libdir],
                   lib=tinyhtm_c_libs,
                   use=['hdf5'])

    # Find TINYHTM C++
    if conf.options.tinyhtm_dir:
        if not conf.options.tinyhtm_incdir:
            conf.options.tinyhtm_incdir=conf.options.tinyhtm_dir + "/include"
        if not conf.options.tinyhtm_libdir:
            conf.options.tinyhtm_libdir=conf.options.tinyhtm_dir + "/lib"
    frag="#include <tinyhtm/Spherical.hxx>\n" + 'int main()\n' \
        + "{tinyhtm::Spherical s(0,0);}\n"
    if conf.options.tinyhtm_incdir:
        tinyhtm_inc=[conf.options.tinyhtm_incdir]
    else:
        tinyhtm_inc=[]

    if conf.options.tinyhtm_cxx_libs:
        tinyhtm_cxx_libs=conf.options.tinyhtm_cxx_libs.split()
    else:
        tinyhtm_cxx_libs=["tinyhtmcxx"]

    conf.check_cxx(msg="Checking for TINYHTM C++ bindings",
                   fragment=frag,
                   includes=tinyhtm_inc, uselib_store='tinyhtm_cxx',
                   libpath=[conf.options.tinyhtm_libdir],
                   rpath=[conf.options.tinyhtm_libdir],
                   lib=tinyhtm_cxx_libs+tinyhtm_c_libs,
                   use=['hdf5','hdf5_cxx'])

def options(opt):
    opt.load('hdf5_cxx');
    tinyhtm=opt.add_option_group('TINYHTM C/C++ Options')
    tinyhtm.add_option('--tinyhtm-dir',
                   help='Base directory where TINYHTM C/C++ is installed')
    tinyhtm.add_option('--tinyhtm-incdir',
                   help='Directory where TINYHTM C/C++ include files are installed')
    tinyhtm.add_option('--tinyhtm-libdir',
                   help='Directory where TINYHTM C/C++ library files are installed')
    tinyhtm.add_option('--tinyhtm-c-libs',
                   help='Names of the TINYHTM C libraries without prefix or suffix\n'
                   '(e.g. "tinyhtm")')
    tinyhtm.add_option('--tinyhtm-cxx-libs',
                   help='Names of the TINYHTM C++ libraries without prefix or suffix\n'
                   '(e.g. "tinyhtmcxx")')
