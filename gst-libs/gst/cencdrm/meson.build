cencdrm_sources = [
  'gstaesctr.c',
  'gstcencdrm.c',
]

cencdrm_headers = [
  'gstaesctr.h',
  'gstcencdrm.h',
]

install_headers(cencdrm_headers, subdir : 'gstreamer-1.0/gst/cencdrm')

gstcencdrm = library('gstcencdrm-' + api_version,
  cencdrm_sources,
  c_args : gst_cencdec_args + ['-DGST_USE_UNSTABLE_API', '-DBUILDING_GST_CENCDRM'],
  include_directories : [configinc, libsinc],
  version : libversion,
  soversion : soversion,
  darwin_versions : osxversion,
  install : true,
  dependencies : [gstbase_dep, xml2_dep, openssl_dep],
)

gstcencdrm_dep = declare_dependency(link_with : gstcencdrm,
  include_directories : [libsinc],
  dependencies : [gstbase_dep, xml2_dep, openssl_dep],
)