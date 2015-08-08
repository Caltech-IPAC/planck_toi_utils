#! /usr/bin/env python
# encoding: utf-8

def configure(conf):
    def get_param(varname,default):
        return getattr(Options.options,varname,'')or default

    conf.load('boost cxx11 cfitsio CCfits hdf5_cxx')
    # Find Tablator
    if conf.options.tablator_dir:
        if not conf.options.tablator_incdir:
            conf.options.tablator_incdir=conf.options.tablator_dir + "/include"
        if not conf.options.tablator_libdir:
            conf.options.tablator_libdir=conf.options.tablator_dir + "/lib"
    frag="#include <tablator/Format.hxx>\n" + 'int main()\n' \
        + "{tablator::Format();}\n"
    if conf.options.tablator_incdir:
        tablator_inc=[conf.options.tablator_incdir]
    else:
        tablator_inc=[]
    if conf.options.tablator_libdir:
        tablator_libdir=[conf.options.tablator_libdir]
    else:
        tablator_libdir=[]
    if conf.options.tablator_libs:
        tablator_libs=conf.options.tablator_libs.split()
    else:
        tablator_libs=["tablator"]

    conf.check_cxx(msg="Checking for Tablator",
                  fragment=frag,
                  includes=tablator_inc, uselib_store='tablator',
                  libpath=tablator_libdir,
                  rpath=tablator_libdir,
                  lib=tablator_libs,
                  use=['BOOST','cxx11','cfitsio','CCfits','hdf5_cxx'])

def options(opt):
    tablator=opt.add_option_group('tablator Options')
    tablator.add_option('--tablator-dir',
                   help='Base directory where tablator is installed')
    tablator.add_option('--tablator-incdir',
                   help='Directory where tablator include files are installed')
    tablator.add_option('--tablator-libdir',
                   help='Directory where tablator library files are installed')
    tablator.add_option('--tablator-libs',
                   help='Names of the tablator libraries without prefix or suffix\n'
                   '(e.g. "tablator"')

    opt.load('boost cxx11 cfitsio CCfits hdf5_cxx')
