#! /usr/bin/env python
# encoding: utf-8

def configure(conf):
    def get_param(varname,default):
        return getattr(Options.options,varname,'')or default

    conf.load('boost')
    conf.check_boost(lib='filesystem system')
    # Find Hires
    if conf.options.hires_dir:
        if not conf.options.hires_incdir:
            conf.options.hires_incdir=conf.options.hires_dir + "/include"
        if not conf.options.hires_libdir:
            conf.options.hires_libdir=conf.options.hires_dir + "/lib"
    frag="#include <Sample.hxx>\n" + 'int main()\n' \
        + "{hires::Sample sample;}\n"
    if conf.options.hires_incdir:
        hires_inc=[conf.options.hires_incdir]
    else:
        hires_inc=[]
    if conf.options.hires_libdir:
        hires_libdir=[conf.options.hires_libdir]
    else:
        hires_libdir=[]
    if conf.options.hires_libs:
        hires_libs=conf.options.hires_libs.split()
    else:
        hires_libs=["hires"]

    conf.check_cxx(msg="Checking for Hires",
                   fragment=frag,
                   includes=hires_inc, uselib_store='hires',
                   libpath=hires_libdir,
                   rpath=hires_libdir,
                   lib=hires_libs,
                   use=['BOOST']
    )

def options(opt):
    hires=opt.add_option_group('hires Options')
    hires.add_option('--hires-dir',
                   help='Base directory where hires is installed')
    hires.add_option('--hires-incdir',
                   help='Directory where hires include files are installed')
    hires.add_option('--hires-libdir',
                   help='Directory where hires library files are installed')
    hires.add_option('--hires-libs',
                   help='Names of the hires libraries without prefix or suffix\n'
                   '(e.g. "hires"')

