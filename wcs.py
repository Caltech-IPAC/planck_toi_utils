#! /usr/bin/env python
# encoding: utf-8

def configure(conf):
    def get_param(varname,default):
        return getattr(Options.options,varname,'')or default

    import os
    wcs_incdir=[]
    wcs_libdir=[]
    for d in ['CM_ENV_DIR','CM_TPS_DIR']:
        env_dir=os.getenv(d)
        if not env_dir:
            break
        if d=='CM_ENV_DIR':
            env_dir=env_dir+'/core'
        if env_dir!=None:
            wcs_incdir.append(env_dir+'/include')
            wcs_libdir.append(env_dir+'/lib')

    # Find wcs
    if conf.options.wcs_dir:
        if not conf.options.wcs_incdir:
            conf.options.wcs_incdir=conf.options.wcs_dir + "/include"
        if not conf.options.wcs_libdir:
            conf.options.wcs_libdir=conf.options.wcs_dir + "/lib"

    if not conf.options.wcs_incdir and not conf.options.wcs_libdir \
       and not conf.options.wcs_libs:
        wcs_config=[[[],[],['wcs']],[[],[],['wcstools']],
                    [['/usr/include/wcstools'],['/usr/lib/x86_64-linux-gnu/'],
                     ['wcstools']]]
    else:
        if conf.options.wcs_incdir:
            wcs_incdir.append(conf.options.wcs_incdir)
        if conf.options.wcs_libdir:
            wcs_libdir.append(conf.options.wcs_libdir)
        
        if conf.options.wcs_libs:
            wcs_libs=[conf.options.wcs_libs.split()]
        else:
            wcs_libs=[['wcs'],['wcstools']]
        
        wcs_config=[]
        for l in wcs_libs:
            wcs_config.append([wcs_incdir,wcs_libdir,l])
    
    wcs_fragment='extern "C" {\n' + \
        '#include "wcs.h"\n' + \
        '}\n' + \
        'int main()\n{\n ' + \
        'wcscon(WCS_J2000,WCS_GALACTIC,0,0,0,0,1950);\n' + '}\n'

    import copy
    found_wcs=False
    for config in wcs_config:
        try:
            conf.check_cc(features="cxx cxxprogram",
                          msg="Checking for WCS library using:\n\t" + str(config),
                          fragment=wcs_fragment,
                          includes=config[0],
                          uselib_store='wcs',
                          libpath=config[1],
                          rpath=config[1],
                          lib=config[2],
                          execute=False)
        except conf.errors.ConfigurationError:
            continue
        else:
            found_wcs=True
            break
    if not found_wcs:
        conf.fatal("Could not find WCS libraries")


def options(opt):
    wcs=opt.add_option_group('WCSTOOLS Options')
    wcs.add_option('--wcs-dir',
                   help='Base directory where wcstools is installed')
    wcs.add_option('--wcs-incdir',
                   help='Directory where wcstools include files are installed')
    wcs.add_option('--wcs-libdir',
                   help='Directory where wcstools library files are installed')
    wcs.add_option('--wcs-libs',
                   help='Names of the wcstools libraries without prefix or suffix\n'
                   '(e.g. "wcs")')
