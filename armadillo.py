#! /usr/bin/env python
# encoding: utf-8

def configure(conf):
    def get_param(varname,default):
        return getattr(Options.options,varname,'')or default

    # Find Armadillo
    if conf.options.armadillo_dir:
        if not conf.options.armadillo_incdir:
            conf.options.armadillo_incdir=conf.options.armadillo_dir + "/include"
        if not conf.options.armadillo_libdir:
            conf.options.armadillo_libdir=conf.options.armadillo_dir + "/lib"
    frag="#include <armadillo>\n" + 'int main()\n' \
        + "{arma::mat M;}\n"
    if conf.options.armadillo_incdir:
        armadillo_inc=[conf.options.armadillo_incdir]
    else:
        armadillo_inc=[]
    if conf.options.armadillo_libdir:
        armadillo_libdir=[conf.options.armadillo_libdir]
    else:
        armadillo_libdir=[]
    if conf.options.armadillo_libs:
        armadillo_libs=conf.options.armadillo_libs.split()
    else:
        armadillo_libs=["armadillo"]

    conf.check_cxx(msg="Checking for Armadillo",
                  fragment=frag,
                  includes=armadillo_inc, uselib_store='armadillo',
                  libpath=armadillo_libdir,
                  rpath=armadillo_libdir,
                  lib=armadillo_libs)

def options(opt):
    armadillo=opt.add_option_group('armadillo Options')
    armadillo.add_option('--armadillo-dir',
                   help='Base directory where armadillo is installed')
    armadillo.add_option('--armadillo-incdir',
                   help='Directory where armadillo include files are installed')
    armadillo.add_option('--armadillo-libdir',
                   help='Directory where armadillo library files are installed')
    armadillo.add_option('--armadillo-libs',
                   help='Names of the armadillo libraries without prefix or suffix\n'
                   '(e.g. "armadillo")')

