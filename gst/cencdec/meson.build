# add plugin specific dependencies here, example:
# liquid_dep = cc.find_library('liquid', required : true)

gstcenc_sources = [
  'gstplugin.c',
  'gstcencdec.c',
  'gstdrm_stub.c',
]

gstcencdec = library('gstcencdec',
  gstcenc_sources,
  c_args : gst_cencdec_args + ['-DGST_USE_UNSTABLE_API', '-DBUILDING_GST_CENCDEC'],
  link_args : noseh_link_args,
  include_directories : [configinc, libsinc],
  dependencies : [gstcencdrm_dep, gst_dep, gstbase_dep, json_dep, openssl_dep, xml2_dep, curl_dep],
  install : true,
  install_dir : plugins_install_dir,
)

pkgconfig.generate(gstcencdec, install_dir : plugins_pkgconfig_install_dir)

plugins += [gstcencdec]
