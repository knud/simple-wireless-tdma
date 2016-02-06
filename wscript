## -*- Mode: python; py-indent-offset: 4; indent-tabs-mode: nil; coding: utf-8; -*-

# def options(opt):
#     pass

# def configure(conf):
#     conf.check_nonfatal(header_name='stdint.h', define_name='HAVE_STDINT_H')`

def build(bld):
    module = bld.create_ns3_module('simple-wireless-tdma', ['network', 'propagation', 'core', 'internet', 'wifi', 'mesh', 'applications'])
    module.includes = '.'
    module.source = [
        'model/simple-wireless-channel.cc',
        'model/tdma-mac.cc',
        'model/tdma-central-mac.cc',
        'model/tdma-mac-net-device.cc',
        'model/tdma-mac-low.cc',
        'model/tdma-controller.cc',
        'model/tdma-mac-queue.cc',
        'helper/tdma-slot-assignment-parser.cc',
        'helper/tdma-controller-helper.cc',
        'helper/tdma-helper.cc',
        ]
        
    module_test = bld.create_ns3_module_test_library('simple-wireless-tdma')
    module_test.source = [
        'test/tdma-test.cc',
        ]
        
    headers = bld(features=['ns3header'])
    headers.module = 'simple-wireless-tdma'
    headers.source = [
        'model/simple-wireless-channel.h',
        'model/tdma-mac.h',
        'model/tdma-central-mac.h',
        'model/tdma-mac-net-device.h',
        'model/tdma-mac-low.h',
        'model/tdma-controller.h',
        'model/tdma-mac-queue.h',
        'helper/tdma-slot-assignment-parser.h',
        'helper/tdma-controller-helper.h',
        'helper/tdma-helper.h',        
        ]

    if (bld.env['ENABLE_EXAMPLES']):
      bld.recurse('examples')
      
#    bld.ns3_python_bindings()
